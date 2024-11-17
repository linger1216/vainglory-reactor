//
// Created by Jingwei on 2024-11-03.
//

#include "tcp_server.h"
#include "channel.h"
#include "event_loop.h"
#include "fd_event.h"
#include "inet_address.h"
#include "tcp_connection.h"
#include "thread_pool.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <utility>
#include "buffer.h"
#include "log.h"
#include "socket_helper.h"

TcpServer::TcpServer(unsigned short port, int threadNum,
                     ConnectionCallback connectionCallback,
                     MessageCallback messageCallback,
                     WriteCompleteCallback writeCompleteCallback)
    : fd_(-1),
      netAddress_(new INetAddress(port)),
      connectionCallback_(std::move(connectionCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  if (connectionCallback_ == nullptr) {
    connectionCallback_ = std::bind(&TcpServer::defaultConnectionCallback, this, std::placeholders::_1);
  }
  if (messageCallback_ == nullptr) {
    messageCallback_ = std::bind(&TcpServer::defaultMessageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  }

  mainEventLoop_ = new EventLoop();
  threadPool_ = new ThreadPool(mainEventLoop_, threadNum);
  listen();
  Debug("Server %s is created", netAddress_->IpPort().c_str());
}


TcpServer::~TcpServer() {
  Debug("Server %s is down", netAddress_->IpPort().c_str());
  close(fd_);
  delete netAddress_;
  delete threadPool_;
  delete mainEventLoop_;
}


int TcpServer::Run() {
  threadPool_->Run();
  auto listenerChannel = new Channel(fd_,
                                     FDEvent::ReadEvent,
                                     std::bind(&TcpServer::acceptConnection, this),
                                     nullptr,
                                     nullptr,
                                     nullptr,
                                     this);
  mainEventLoop_->AddTask(listenerChannel, EventLoopOperator::Add);
  mainEventLoop_->Run();
  Debug("Server %s is running", netAddress_->IpPort().c_str());
  return 0;
}

int TcpServer::acceptConnection() {

  INetAddress peerAddr(0);
  int clientFd = SocketHelper::Accept(fd_, peerAddr.GetSockAddr());
  auto peer = SocketHelper::to_sockaddr_in(peerAddr.GetSockAddr());
  Debug("accept client %s:%d\n", inet_ntoa(peer->sin_addr),
        ntohs(peer->sin_port));

  INetAddress localAddr(SocketHelper::GetLocalAddr(clientFd));

  // TODO：
  // 得到了client id后，不能在主线程中去处理通信的相关流程了
  // 需要从线程池得到一个合适的工作线程， 委托他使用Tcp Connection来通信相关处理
  EventLoop* ioLoop = threadPool_->GetNextEventLoop();

  // TODO:
  // 新建的tcpConnection资源什么时候释放？
  auto conn = new TcpConnection(clientFd,
                                ioLoop,
                                localAddr,
                                peerAddr,
                                connectionCallback_,
                                std::bind(&TcpServer::removeConnection, this, std::placeholders::_1),
                                messageCallback_,
                                writeCompleteCallback_);
  connections_["TcpConnection-" + std::to_string(clientFd)] = conn;
  return 0;
}


void TcpServer::listen() {
  fd_ = SocketHelper::CreateSocket(AF_INET);
  SocketHelper::SetReuseAddr(fd_, true);
  SocketHelper::SetReusePort(fd_, true);
  SocketHelper::Bind(fd_, netAddress_->GetSockAddr());
  SocketHelper::Listen(fd_);
  Debug("listen on %s", netAddress_->IpPort().c_str());

  // 后面还差一个accept, 其实就是委托channel来实现
}
void TcpServer::defaultConnectionCallback(const TcpConnection* conn) {
}
void TcpServer::defaultMessageCallback(const TcpConnection* conn, Buffer* buffer, int n) {
  char* data = new char[n];
  buffer->ReadAll(data, n);
  Debug("defaultMessage receive data %s", data);
  delete []data;
}
void TcpServer::removeConnection(const TcpConnection* conn) {
}
