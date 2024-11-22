//
// Created by Jingwei on 2024-11-03.
//

#ifndef REACTOR_CPP_TCP_CONNECTION_H
#define REACTOR_CPP_TCP_CONNECTION_H


#include "callback.h"
#include "inet_address.h"
#include "no_copyable.h"
#include "time_stamp.h"
#include <atomic>
#include <string>

class EventLoop;
class Channel;
class Buffer;

class TcpConnection : public NoCopyable {
public:
  enum class Status {
    Disconnected = 0x00,
    Connected = 0x01,
    Disconnecting = 0x01 << 1,
    Connecting = 0x01 << 2,
  };

  ~TcpConnection();
  explicit TcpConnection(const char* name, int fd, EventLoop* eventLoop,
                         const INetAddress& localAddr,
                         const INetAddress& peerAddr,
                         ConnectionCallback connectionCallback,
                         CloseCallback closeCallback,
                         MessageCallback messageCallback,
                         WriteCompleteCallback writeCompleteCallback);

  void Send(const char* data, size_t len);
  EventLoop* Loop();
  const char* Name();
  const char* PeerIpPort();
  int Close();
private:
  int readHandler(void* arg);
  int writeHandler(void* arg);
  int closeHandler(void* arg);
  int errorHandler(void* arg);
private:
  const int BUFFER_SIZE = 10240;
  std::string name_;
  EventLoop* loop_;
  Channel* channel_;
  Buffer* readBuf_;
  Buffer* writeBuf_;
  std::atomic_int  status_;

  INetAddress localAddr_;
  INetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  size_t highWaterMark_;
  HighWaterMarkCallback highWaterMarkCallback_;
};


#endif //REACTOR_CPP_TCP_CONNECTION_H
