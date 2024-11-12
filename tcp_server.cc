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
#include "log.h"

TcpServer::TcpServer(unsigned short port, int threadNum)
    : fd_(-1),
      netAddress_(new INetAddress(port, "127.0.0.1")) {
  mainEventLoop_ = new EventLoop();
  threadPool_ = new ThreadPool(mainEventLoop_, threadNum);
  listen();
}


TcpServer::~TcpServer() {
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
                                     nullptr, nullptr);
  mainEventLoop_->AddTask(listenerChannel, EventLoopOperator::Add);
  mainEventLoop_->Run();
  return 0;
}

int TcpServer::acceptConnection() {
  // 客户端进行连接
  auto addr = const_cast<sockaddr_in*>(netAddress_->GetSockAddr());
  unsigned int client_addr_len = sizeof(*addr);
  int clientFd = accept(fd_,
                        (sockaddr*)addr,
                        &client_addr_len);
  if (clientFd == -1) {
    Error("accept connection error");
  }

  Debug("accept client %s:%d\n", inet_ntoa(addr->sin_addr),
         ntohs(addr->sin_port));

  // TODO：
  // 得到了client id后，不能在主线程中去处理通信的相关流程了
  // 需要从线程池得到一个合适的工作线程， 委托他使用Tcp Connection来通信相关处理
  EventLoop* eventLoop = threadPool_->TakeEventLoop();

  // TODO:
  // 新建的tcpConnection资源什么时候释放？
  new TcpConnection(clientFd, eventLoop);

  return 0;
}


void TcpServer::listen() {
  // 1. 创建监听的fd
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == -1) {
    Error("create socket error");
  }

  // 2. 设置端口复用
  int opt = 1;
  int ret = setsockopt(fd_, SOL_SOCKET,
                       SO_REUSEADDR, &opt, sizeof opt);
  if (ret == -1) {
    Error("set socket option SO_REUSEADDR error");
  }

  // 3. 绑定
  auto addr = const_cast<sockaddr_in*>(netAddress_->GetSockAddr());
  ret = bind(fd_, (struct sockaddr*) addr, sizeof(*addr));
  if (ret == -1) {
    Error("bind socket error");
  }

  // 4. 设置监听
  ret = ::listen(fd_, 128);
  if (ret == -1) {
    Error("listen socket error");
  }
}
