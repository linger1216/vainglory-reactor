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
Acceptor::Acceptor(EventLoop* loop, const INetAddress* listenAddr, AcceptCallback cb)
    : loop_(loop), acceptChannel_(nullptr), acceptCallback(std::move(cb)) {

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

  INetAddress peerAddr(0, false);
  int clientFd = SocketHelper::Accept(acceptChannel_->Fd(), peerAddr.GetSockAddr());

  auto addr = SocketHelper::GetLocalAddr(acceptChannel_->Fd());
  INetAddress localAddr(&addr);

  Debug("【资源】%s(%d) ---> %s", peerAddr.IpPort().c_str(), clientFd, localAddr.IpPort().c_str());

  // 此处是要求必须有回调函数的, 如果没有就放弃这个连接.
  if (acceptCallback) {
    acceptCallback(clientFd, &peerAddr);
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
