//
// Created by Jingwei on 2024-11-22.
//

#include "acceptor.h"
#include "channel.h"
#include "inet_address.h"
#include "log.h"
#include "socket_helper.h"
#include "event_loop.h"

/*
 * 调用的线程必须和创建线程是一致的, 也就是"主线程";
 */
Acceptor::Acceptor(EventLoop* loop, const INetAddress* listenAddr, ConnectedCallback cb)
    : loop_(loop), acceptChannel_(nullptr), connectedCallback_(std::move(cb)) {

  int acceptSocketFd = SocketHelper::CreateSocket(AF_INET);
  SocketHelper::SetReuseAddr(acceptSocketFd, true);
  SocketHelper::SetReusePort(acceptSocketFd, true);
  SocketHelper::Bind(acceptSocketFd, listenAddr->GetSockAddr());
  acceptChannel_ = new Channel(acceptSocketFd, FDEvent::ReadEvent,
                               std::bind(&Acceptor::handleAccept, this),
                               nullptr,
                               nullptr,
                               nullptr,
                               this);
}

//
void Acceptor::handleAccept() {
  loop_->AssertInLoop();

  INetAddress peerAddr(0);
  int clientFd = SocketHelper::Accept(acceptSocketFd_, peerAddr.GetSockAddr());
  auto peer = SocketHelper::to_sockaddr_in(peerAddr.GetSockAddr());
  Debug("有个新的客户端 %s:%d", inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));

  // 此处是要求必须有回调函数的, 如果没有就放弃这个连接.
  if (connectedCallback_) {
    connectedCallback_(clientFd, &peerAddr);
  } else {
    SocketHelper::Close(clientFd);
  }
}

Acceptor::~Acceptor() {
  loop_->AssertInLoop();
  acceptChannel_->DisableAll();
  loop_->UpdateChannelEventInLoop(acceptChannel_);
}

int Acceptor::Listen() {
  loop_->AssertInLoop();
  SocketHelper::Listen(acceptChannel_->Fd());
  acceptChannel_->EnableReading();
  loop_->AddChannelEventInLoop(acceptChannel_);
  return 0;
}
