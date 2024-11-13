//
// Created by Jingwei on 2024-11-03.
//

#ifndef REACTOR_CPP_TCP_CONNECTION_H
#define REACTOR_CPP_TCP_CONNECTION_H


#include "no_copyable.h"
#include <string>
#include "time_stamp.h"

class EventLoop;
class Channel;
class Buffer;

class TcpConnection : public NoCopyable{
public:
  ~TcpConnection();
  explicit TcpConnection(int fd, EventLoop* eventLoop);

private:
  int readHandler(void* arg);
  int writeHandler(void* arg);
  int closeHandler(void* arg);
  int destroyHandler(void* arg);

private:
  const int BUFFER_SIZE = 10240;
  std::string _name;
  EventLoop* _eventLoop;
  Channel* _channel;
  Buffer* _readBuf;
  Buffer* _writeBuf;
};


#endif//REACTOR_CPP_TCP_CONNECTION_H
