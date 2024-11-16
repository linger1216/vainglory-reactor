//
// Created by Jingwei on 2024-11-14.
//

#ifndef VAINGLORY_REACTOR_TCP_CLIENT_H
#define VAINGLORY_REACTOR_TCP_CLIENT_H


#include "no_copyable.h"
#include <string>
#include "inet_address.h"
#include <functional>

class EventLoop;
class TcpConnection;
class Buffer;

class TcpClient : public NoCopyable {
public:
  TcpClient(EventLoop* loop, const INetAddress& addr, const std::string& name);
  ~TcpClient();
  void Connect();
  void Disconnect();
  void Stop();
  void SetConnectionCallback(const ConnectionCallback& cb);
  void SetMessageCallback(const MessageCallback& cb);
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb);
private:
  INetAddress addr_;
  EventLoop* loop_;
  std::string name_;
  TcpConnection* conn_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
};


#endif //VAINGLORY_REACTOR_TCP_CLIENT_H
