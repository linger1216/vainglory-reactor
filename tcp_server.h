//
// Created by Jingwei on 2024-11-03.
//

#ifndef REACTOR_CPP_TCP_SERVER_H
#define REACTOR_CPP_TCP_SERVER_H

#include "no_copyable.h"
#include "callback.h"
#include <map>
#include <string>

class EventLoop;
class ThreadPool;
class INetAddress;
class Acceptor;

class TcpServer : public NoCopyable{
public:
  ~TcpServer();
  explicit TcpServer(unsigned short port, int threadNum,
                     ConnectionCallback connectionCallback,
                     MessageCallback messageCallback,
                     WriteCompleteCallback writeCompleteCallback);
public:
  int Run();
private:
  void listen();
  int acceptConnection();
  void connectedCallback(int clientFd, const INetAddress& peerAddr);
  void removeConnection(const TcpConnection* conn);
  void defaultConnectionCallback(const TcpConnection* conn);
  void defaultMessageCallback(const TcpConnection* conn, Buffer*, int n);
private:
  int fd_;
  INetAddress* netAddress_;
  EventLoop* mainEventLoop_;
  ThreadPool* threadPool_;
  Acceptor* acceptor_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  std::map<std::string, TcpConnection*> connections_;
};


#endif//REACTOR_CPP_TCP_SERVER_H
