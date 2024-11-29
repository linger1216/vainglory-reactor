//
// Created by Jingwei on 2024-11-03.
//

#include "tcp_connection.h"

#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "log.h"
#include "socket_helper.h"
#include <cassert>
#include <unistd.h>
#include <utility>

TcpConnection::TcpConnection(const char* name, int fd, EventLoop* eventLoop,
                             const INetAddress* localAddr,
                             const INetAddress* peerAddr,
                             ConnectionCallback destroyCallback,
                             MessageCallback messageCallback,
                             ConnectionCallback writeCompleteCallback)
    : name_(name),
      status_(Status::Disconnected),
      loop_(eventLoop),
      localAddr_(*localAddr),
      peerAddr_(*peerAddr),
      destroyCallback_(std::move(destroyCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  name_ = "TcpConnection-" + std::to_string(fd);
  Debug("创建TCP连接, connName: %s", name_.c_str());

  readBuf_ = new Buffer(BUFFER_SIZE);
  Debug("【资源】新增对象 ReadBuffer = %p", readBuf_);

  writeBuf_ = new Buffer(BUFFER_SIZE);
  Debug("【资源】新增对象 WriteBuffer = %p", writeBuf_);

  // 设置 keep alive
  SocketHelper::SetKeepAlive(fd, true);

  channel_ = new Channel(fd, FDEvent::ReadEvent,
                         std::bind(&TcpConnection::handlerRead, this, std::placeholders::_1),
                         std::bind(&TcpConnection::handlerWrite, this, std::placeholders::_1),
                         std::bind(&TcpConnection::handlerDestroy, this, std::placeholders::_1),
                         std::bind(&TcpConnection::handlerError, this, std::placeholders::_1),
                         this);
  Debug("【资源】新增对象 Channel fd=%d = %p", fd, channel_);
  // TODO 状态要不要给个cb来回调
  status_ = Status::Connecting;
  loop_->AddChannelEventInLoop(channel_);
  status_ = Status::Connected;
}

TcpConnection::~TcpConnection() {
  // delete conn前一定要调用destroy函数
  // 因为conn是在mainloop中创建的, 按照谁申请谁释放的原则, 应该是mainloop去delete conn;
  // 但mainloop是在主线程内构造, 所以在mainloop中delete conn不安全, 要在io中去操作
  // 所以在main中要调用ioloop->Destroy. 然后在手动执行delete.
  // 所以这里的析构函数为空.
  // 一定要注意 ⚠!!!
}

int TcpConnection::Destroy() {
  loop_->AssertInLoop();

  auto fd = channel_->Fd();
  // 删除对底层的监听
  Debug("删除对底层的监听 from %d", fd);
  loop_->DeleteChannelEventInLoop(channel_);

  // 清理loop中关于这个channel的资源
  Debug("【释放】EventLoop fd2ChannelMap_ 删除了 %d -> %p", fd, channel_);

  loop_->EraseChannelInMap(channel_);

  Debug("【释放】删除对象 Channel fd=%d = %p", fd, channel_);
  delete channel_;
  Debug("【释放】删除对象 ReadBuffer = %p", readBuf_);
  delete readBuf_;
  Debug("【释放】删除对象 WriteBuffer = %p", writeBuf_);
  delete writeBuf_;
  Debug("%s 释放读写缓冲区,channel内存", name_.c_str());

  Debug("【释放】关闭文件描述符 %d", fd);
  ::close(fd);
  return 0;
}


int TcpConnection::handlerRead(void* arg) {
  int errNo = 0;
  int count = readBuf_->ReadSocket(channel_->Fd(), &errNo);
  if (count > 0) {
    messageCallback_(this, readBuf_, count);
  } else if (count == 0) {
    // 没有读到数据，关闭连接
    Debug("TcpConnection::handlerRead read 0 byte, so we close connection %p ", std::this_thread::get_id());
    handlerDestroy(this);
  } else {
    Error("read error, error code: %d\n", SocketHelper::GetSocketError(channel_->Fd()));
  }
  return 0;
}

int TcpConnection::handlerWrite(void* arg) {
  assert(loop_->IsInLoopThread());
  auto conn = static_cast<TcpConnection*>(arg);
  // 判断是否有写事件
  if (channel_->IsWriting()) {
    // 将还没有读完的数据,全部写出去.
    int count = conn->writeBuf_->SendSocket(conn->channel_->Fd());
    if (count > 0) {
      // 数据已经被全部发出去了
      if (conn->writeBuf_->GetReadableSize() == 0) {
        // TODO: 这一块逻辑是为啥
        // 1. 不在检测写事件
        conn->channel_->DisableWriting();
        // 2. 修改dispathcer检测的集合 -- 添加节点
        //        conn->loop_->AddTask(conn->channel_, EventLoopOperator::Update);
        conn->loop_->UpdateChannelEventInLoop(conn->channel_);
        if (writeCompleteCallback_ != nullptr) {
          writeCompleteCallback_(conn);
        }
      }
    } else {
      // 没有写到数据，关闭连接
      Debug("write 0 bytes, means connection is down, so we close connection");
      //      conn->loop_->AddTask(conn->channel_, EventLoopOperator::Delete);
      conn->loop_->UpdateChannelEventInLoop(conn->channel_);
    }
  }
  return 0;
}

int TcpConnection::handlerError(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  int err = SocketHelper::GetSocketError(conn->channel_->Fd());
  Warn("handlerError, error code: %d\n", err);
  return 0;
}


int TcpConnection::handlerDestroy(void* arg) {
  // handlerDestroy 来自两个路径
  // 1. epoll检测到close事件
  // 2. epoll在read事件中发现读到的长度为0
  // 但不管怎么样, 这些都是io线程, io线程处理io线程的资源
  // io调用上层也就是server的callback, 属于跨线程, 需要wake up and loop
  // 这里应该是来自io线程判断出来读事件为0, 然后调用channel的destroy callback
  // 继而调用的是connection 的handler Destroy
  loop_->AssertInLoop();
  if (status_ == Status::Disconnecting || status_ == Status::Disconnected) {
    Debug("TcpConnection::handlerDestroy return %s", statusToString(status_));
    return 0;
  }

  Debug("TcpConnection::handlerDestroy %s", statusToString(status_));

  status_ = Status::Disconnecting;

  // 禁用channel的底层事件监听
//  loop_->DeleteChannelEventInLoop(channel_);
//  Debug("禁用%d的一切事件", channel_->Fd());

  Destroy();

  status_ = Status::Disconnected;

  // TODO 有问题, 这里的loop是io, 怎么调用到server里面的callback, 跨线程通知.
  // 调用close Callback
  if (destroyCallback_) {
    destroyCallback_(this);
  }
  return 0;
}

/*
  在muduo中，当用户调用了TcpConnection::send(buf)函数时：
  如果发送缓冲区没有待发送数据：
    如果TCP发送缓冲区能一次性容纳buf，调用用户自定义的writeCompleteCallback_
    来移除该TcpConnection在事件监听器上的可写事件（因为大多数时候是没有数据需要发送的，
    频繁触发可写事件但又没有数据可写）
    如果TCP发送缓冲区不能一次性容纳buf，判断一下errno是不是SIGPIPE RESET等致命错误。
  如果发送缓冲区没有待发送数据 && 非致命错误：
    判断是否是高水位，执行回调highWaterMarkCallback_ ；
    不直接write，先将数据append到发送缓冲区（如果缓冲区不足会执行makeSpace扩容）；
    注册当前channel_的写事件，通知epoll监听处理。
 */

/**
 * 发送数据 应用写的快 而内核发送数据慢 需要把待发送数据写入缓冲区，而且设置了水位回调
 **/
void TcpConnection::Send(const char* data, size_t len) {
  ssize_t wroteSize = 0;
  size_t remaining = len;
  bool faultError = false;

  // 之前没有设置过写操作,第一次开始写数据
  // 写缓冲区没有可读的数据, 也就是说 写缓冲区没有待发送数据
  if (!channel_->IsWriting() && writeBuf_->GetReadableSize() == 0) {
    wroteSize = ::write(channel_->Fd(), data, len);
    /*
     * wroteSize == 0 的场景
      1. 连接已关闭：如果对端已经关闭了连接，那么尝试写入数据时，::write 调用可能会返回 0，
      表示没有数据被写入。这是因为在 TCP 连接中，如果对端关闭了连接，
      再向该连接写入数据将不再可能，系统会返回一个信号表明写入操作无法进行

      2. 非阻塞模式下的写操作：在非阻塞模式下，如果尝试写入的数据量大于内核立即可接受的数据量，
      ::write 调用可能会返回 0，表示没有数据被写入。这是因为在非阻塞模式下，
      ::write 调用不会等待直到数据被写入，而是立即返回

      3. 内核缓冲区已满：在某些情况下，如果内核的发送缓冲区已满，
      ::write 调用可能会返回 0，表示没有数据被写入。这是因为内核缓冲区有限，
      当它满了之后，新的写入操作将无法进行，直到缓冲区中的空间被释放
     */
    if (wroteSize >= 0) {
      remaining = len - wroteSize;
      // 写完了data[len]到到内核缓冲区, 就回调writeCompleteCallback_
      if (remaining == 0 && writeCompleteCallback_ != nullptr) {
        // 这里要改造loop的task啊....
        // 不仅仅是只有channel的东西, 还有一些业务逻辑
        // 为啥要调用就回调writeCompleteCallback_
        // TODO:
      }
    } else {
      wroteSize = 0;
      // EWOULDBLOCK: 发送缓冲区已满
      // 用来检查在非阻塞模式下进行写操作时，
      // 是否因为内核缓冲区暂时没有空间而无法立即完成写入
      // 等同于EAGAIN
      if (errno == EWOULDBLOCK) {
        Warn("TcpConnection::send");
        // EPIPE 表示尝试向已经关闭的管道写入数据，即“Broken pipe”
        // 而 ECONNRESET 表示连接被对方重置
        // 这两种情况都表明连接已经出现问题，无法继续进行正常的数据传输，
        // 因此设置 faultError 为 true 来标记这个错误状态。
        // 在实际应用中，这可能意味着需要关闭连接并进行相应的错误处理。
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }

  /**
     * 说明当前这一次write并没有把数据全部发送出去 剩余的数据需要保存到缓冲区当中
     * 然后给channel注册EPOLLOUT事件，Poller发现tcp的发送缓冲区有空间后会通知
     * 相应的sock->channel，调用channel对应注册的writeCallback_回调方法，
     * channel的writeCallback_实际上就是TcpConnection设置的handleWrite回调，
     * 把发送缓冲区outputBuffer_的内容全部发送完成
     **/

  // 处理剩余待发送数据
  // 可能就是数据没写完
  // 举例:
  // len = 1000
  // wroteSize = 600
  if (!faultError && remaining > 0) {
    size_t oldLen = writeBuf_->GetReadableSize();
    if (oldLen + remaining >= highWaterMark_ &&
        oldLen < highWaterMark_ &&
        highWaterMarkCallback_) {
      // TODO
    }

    // 如果当前发送缓冲区的数据量已经超过了高水位线，
    // 则执行高水位回调highWaterMarkCallback_
    // 这里一定要注册channel的写事件 否则poller不会给channel通知epollout
    writeBuf_->Append(data + wroteSize, remaining);
    if (!channel_->IsWriting()) {
      channel_->EnableWriting();
    }
  }
}
const char* TcpConnection::Name() {
  return name_.c_str();
}
const char* TcpConnection::PeerIpPort() {
  return peerAddr_.IpPort().c_str();
}
EventLoop* TcpConnection::Loop() {
  return loop_;
}
const char* TcpConnection::statusToString(Status status) const {

  if (status == Status::Disconnecting) {
    return "Disconnecting";
  } else if (status == Status::Connected) {
    return "Connected";
  } else if (status == Status::Disconnected) {
    return "Disconnected";
  } else if (status == Status::Connecting) {
    return "Connecting";
  }
  return "Unknown Status";
}
