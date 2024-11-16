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
private:
  EventLoop* mainEventLoop_;
  ThreadPool* threadPool_;
  int fd_;
  INetAddress* netAddress_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  std::map<std::string, TcpConnection*> connections_;
};


#endif//REACTOR_CPP_TCP_SERVER_H
