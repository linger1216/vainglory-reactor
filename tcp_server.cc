//
// Created by Jingwei on 2024-11-03.
//

#include "tcp_server.h"
#include "acceptor.h"
#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "fd_event.h"
#include "inet_address.h"
#include "log.h"
#include "socket_helper.h"
#include "tcp_connection.h"
#include "thread_pool.h"
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <utility>

TcpServer::TcpServer(unsigned short port, int threadNum,
                     ConnectionCallback connectionCallback,
                     ConnectionCallback destroyCallback,
                     MessageCallback messageCallback,
                     ConnectionCallback writeCompleteCallback)
    : fd_(-1),
      acceptor_(nullptr),
      netAddress_(new INetAddress(port)),
      connectionCallback_(std::move(connectionCallback)),
      destroyCallback_(std::move(destroyCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  // 初始化主线程
  mainEventLoop_ = new EventLoop();

  // 创建监听器
  acceptor_ = new Acceptor(mainEventLoop_, netAddress_,
                           std::bind(&TcpServer::newConnectionCallback, this, std::placeholders::_1, std::placeholders::_2));
  // 创建线程池
  threadPool_ = new ThreadPool(mainEventLoop_, threadNum);
}


TcpServer::~TcpServer() {
  Debug("Server %s is down", netAddress_->IpPort().c_str());
  close(fd_);
  delete netAddress_;
  delete threadPool_;
  delete mainEventLoop_;
}


int TcpServer::Run() {

  // 线程池启动
  threadPool_->Run();

  // 开始监听 acceptor listen 内部封装了loop queue,
  // 不用直接用run in loop
  acceptor_->Listen();
  Debug("listen on %s", netAddress_->IpPort().c_str());

  // 主线程循环启动
  Debug("Server %s is running", netAddress_->IpPort().c_str());
  mainEventLoop_->Run();
  Debug("Server %s is down", netAddress_->IpPort().c_str());
  return 0;
}

void TcpServer::destroyConnection(const TcpConnection* conn) {
  mainEventLoop_->AssertInLoop();
  // 1. 删除集合中的元素
  auto c = const_cast<TcpConnection*>(conn);
  Debug("释放新连接 from %s", c->PeerIpPort());
  auto n = connections_.erase(c->Name());
  assert(n == 1);
  Debug("【释放】TcpServer connections 删除了 TcpConnection[%s] = %p", c->Name(), c);

  // 手动删除tcp conn的资源
  // 想依靠delete来触发tcp conn的析构操作, 可以但不安全
  // 因为tcp server是mainloop, tcp conn是io loop
//  auto ioLoop = c->Loop();
//  ioLoop->RunInLoop(std::bind(&TcpConnection::Destroy, c));

  Debug("【释放】删除对象 TcpConnection[%s] = %p", c->Name(), conn);
  delete conn;

  // 回调应用层的函数
  if (destroyCallback_) {
    destroyCallback_(conn);
  }
}

// 代表一个新链接建立完毕, 后面的工作是将这个连接的数据接收
// 放在一个线程中进行处理
// 得到了client id后，不能在主线程中去处理通信的相关流程了
// 需要从线程池得到一个合适的工作线程， 委托他使用Tcp Connection来通信相关处理
void TcpServer::newConnectionCallback(int clientFd, const INetAddress* peerAddr) {
  auto addr = SocketHelper::GetLocalAddr(clientFd);
  INetAddress localAddr(&addr);
  EventLoop* ioLoop = threadPool_->GetNextEventLoop();
  auto name = "TcpConnection-" + std::to_string(clientFd);

  auto conn = new TcpConnection(name.c_str(), clientFd, ioLoop, &localAddr, peerAddr,
                                std::bind(&TcpServer::destroyByCallback, this, std::placeholders::_1),
                                messageCallback_,
                                writeCompleteCallback_);
  Debug("【资源】新增对象 TcpConnection[%s] = %p", name.c_str(), conn);

  connections_[name] = conn;
  Debug("【资源】TcpServer connections 添加了 TcpConnection[%s] = %p", name.c_str(), conn);

  // 回调用户层
  if (connectionCallback_) {
    connectionCallback_(conn);
  }
}

// 被直接回调, 来自io线程所以要转换成本地运行.
// 这里是很有必要的, 包装一个functor, 让其进入队列唤醒执行.
// 不能在io线程的空间执行函数
void TcpServer::destroyByCallback(const TcpConnection* conn) {
  Debug("TcpServer destroyByCallback");
  auto fn = [this, conn](){
    this->destroyConnection(conn);
    return 0; // 返回0，因为EventLoop::Functor期望返回int
  };
  mainEventLoop_->RunInLoop(fn);
}
