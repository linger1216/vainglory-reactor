//
// Created by Jingwei on 2024-11-03.
//

#include "tcp_connection.h"

#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "log.h"
#include "socket_helper.h"
#include <cassert>
#include <utility>
TcpConnection::TcpConnection(int fd, EventLoop* eventLoop,
                             const INetAddress& localAddr,
                             const INetAddress& peerAddr,
                             ConnectionCallback connectionCallback,
                             CloseCallback closeCallback,
                             MessageCallback messageCallback,
                             WriteCompleteCallback writeCompleteCallback)
    : eventLoop_(eventLoop),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      connectionCallback_(std::move(connectionCallback)),
      closeCallback_(std::move(closeCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  name_ = "TcpConnection-" + std::to_string(fd);
  readBuf_ = new Buffer(BUFFER_SIZE);
  writeBuf_ = new Buffer(BUFFER_SIZE);

  auto readHandler = std::bind(&TcpConnection::readHandler, this, std::placeholders::_1);
  auto writeHandler = std::bind(&TcpConnection::writeHandler, this, std::placeholders::_1);
  auto closeHandler = std::bind(&TcpConnection::closeHandler, this, std::placeholders::_1);
  auto errorHandler = std::bind(&TcpConnection::errorHandler, this, std::placeholders::_1);

  // 设置 keep alive
  SocketHelper::SetKeepAlive(fd, true);

  channel_ = new Channel(fd, FDEvent::ReadEvent,
                         readHandler,
                         writeHandler,
                         closeHandler,
                         errorHandler,
                         this);

  eventLoop_->AddTask(channel_, EventLoopOperator::Add);
}

TcpConnection::~TcpConnection() {
  delete readBuf_;
  delete writeBuf_;
  eventLoop_->FreeChannel(channel_);
  delete channel_;
  Debug("连接断开, 释放资源, connName: %s", name_.c_str());
}

int TcpConnection::readHandler(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  int count = conn->readBuf_->ReadSocket(conn->channel_->Fd());
  if (count > 0) {
    // TODO
    // 这里有点奇怪, 不知道要不要读.
    messageCallback_(conn, conn->readBuf_, count);
  } else if (count == 0) {
    // 没有读到数据，关闭连接
    Debug("read 0 bytes, means connection is down, so we close connection\n");
    conn->eventLoop_->AddTask(conn->channel_, EventLoopOperator::Delete);
  } else {
    Error("read error, error code: %d\n", SocketHelper::GetSocketError(conn->channel_->Fd()));
  }
  return 0;
}

int TcpConnection::writeHandler(void* arg) {
  assert(eventLoop_->IsInLoopThread());
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
        conn->eventLoop_->AddTask(conn->channel_, EventLoopOperator::Update);

        if (writeCompleteCallback_ != nullptr) {
          writeCompleteCallback_(conn);
        }
      }
    } else {
      // 没有写到数据，关闭连接
      Debug("write 0 bytes, means connection is down, so we close connection\n");
      conn->eventLoop_->AddTask(conn->channel_, EventLoopOperator::Delete);
    }
  }
  return 0;
}

int TcpConnection::errorHandler(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  int err = SocketHelper::GetSocketError(conn->channel_->Fd());
  Warn("errorHandler, error code: %d\n", err);
  return 0;
}

int TcpConnection::closeHandler(void* arg) {
  assert(eventLoop_->IsInLoopThread());
  auto conn = static_cast<TcpConnection*>(arg);
  conn->channel_->DisableAll();
  conn->eventLoop_->AddTask(channel_, EventLoopOperator::Update);
  conn->closeCallback_(conn);
  delete conn;
  return 0;
}
