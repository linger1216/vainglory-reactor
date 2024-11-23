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
                     MessageCallback messageCallback,
                     WriteCompleteCallback writeCompleteCallback)
    : fd_(-1),
      acceptor_(nullptr),
      netAddress_(new INetAddress(port)),
      connectionCallback_(std::move(connectionCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  // 初始化主线程
  mainEventLoop_ = new EventLoop();

  // 创建监听器
  acceptor_ = new Acceptor(mainEventLoop_, netAddress_,
                           std::bind(&TcpServer::newConnectionCallback, this, std::placeholders::_1, std::placeholders::_2));

  // 创建线程池
  threadPool_ = new ThreadPool(mainEventLoop_, threadNum);

  if (connectionCallback_ == nullptr) {
    connectionCallback_ = std::bind(&TcpServer::defaultConnectionCallback, this, std::placeholders::_1);
  }
  if (messageCallback_ == nullptr) {
    messageCallback_ = std::bind(&TcpServer::defaultMessageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  }
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

void TcpServer::defaultConnectionCallback(const TcpConnection* conn) {
}
void TcpServer::defaultMessageCallback(const TcpConnection* conn, Buffer* buffer, int n) {
}

void TcpServer::removeConnection(const TcpConnection* conn) {
  mainEventLoop_->AssertInLoop();
  // 主线程需要将他自己拥有的资源移除
  // 主要就是链表删除这个连接引用, 其他没什么吊事.
  // 删除资源不在这.
  auto c = const_cast<TcpConnection*>(conn);
  Debug("释放新连接 from %s", c->PeerIpPort());
  auto n = connections_.erase(c->Name());
  assert(n == 1);

  // 关闭资源在io线程中做, 因为这个连接的资源是io线程拥有的
  auto ioLoop = c->Loop();
  ioLoop->RunInLoop(std::bind(&TcpConnection::Close, c));
}

// 代表一个新链接建立完毕, 后面的工作是将这个连接的数据接收
// 放在一个线程中进行处理
// 得到了client id后，不能在主线程中去处理通信的相关流程了
// 需要从线程池得到一个合适的工作线程， 委托他使用Tcp Connection来通信相关处理
void TcpServer::newConnectionCallback(int clientFd, const INetAddress* peerAddr) {

  auto addr = SocketHelper::GetLocalAddr(clientFd);
  INetAddress localAddr(&addr);

  Debug("收到新连接 from %s", peerAddr->IpPort().c_str());
  EventLoop* ioLoop = threadPool_->GetNextEventLoop();
  Debug("从线程池取一个IO事件循环, 因为运行在线程池的, 所以一定是非主线程");
  // TODO:
  // 新建的tcpConnection资源什么时候释放？
  auto name = "TcpConnection-" + std::to_string(clientFd);
  auto conn = new TcpConnection(name.c_str(), clientFd, ioLoop, &localAddr, peerAddr,
                                connectionCallback_,
                                std::bind(&TcpServer::removeConnection, this, std::placeholders::_1),
                                messageCallback_,
                                writeCompleteCallback_);
  connections_[name] = conn;
}
