//
// Created by Jingwei on 2024-11-03.
//

#ifndef REACTOR_CPP_TCP_CONNECTION_H
#define REACTOR_CPP_TCP_CONNECTION_H


#include "callback.h"
#include "inet_address.h"
#include "no_copyable.h"
#include "time_stamp.h"
#include <string>

class EventLoop;
class Channel;
class Buffer;

class TcpConnection : public NoCopyable{
public:
  ~TcpConnection();
  explicit TcpConnection(int fd, EventLoop* eventLoop,
                         const INetAddress& localAddr,
                         const INetAddress& peerAddr,
                         ConnectionCallback connectionCallback,
                         CloseCallback closeCallback,
                         MessageCallback messageCallback,
                         WriteCompleteCallback writeCompleteCallback);

private:
  int readHandler(void* arg);
  int writeHandler(void* arg);
  int closeHandler(void* arg);
  int errorHandler(void* arg);

private:
  const int BUFFER_SIZE = 10240;
  std::string name_;
  EventLoop* eventLoop_;
  Channel* channel_;
  Buffer* readBuf_;
  Buffer* writeBuf_;

  INetAddress localAddr_;
  INetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
};


#endif//REACTOR_CPP_TCP_CONNECTION_H
