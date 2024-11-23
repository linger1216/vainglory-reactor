//
// Created by Jingwei on 2024-11-22.
//

#ifndef VAINGLORY_REACTOR_ACCEPTOR_H
#define VAINGLORY_REACTOR_ACCEPTOR_H
#include "no_copyable.h"
#include <functional>
#include "callback.h"

class INetAddress;
class EventLoop;
class Channel;

class Acceptor : public NoCopyable {
public:
  explicit Acceptor(EventLoop* loop, const INetAddress* listenAddr, NewConnectionCallback cb);
  ~Acceptor();
  int Listen();
private:
  void handleAccept();
  EventLoop* loop_;
  int acceptSocketFd_;
  Channel* acceptChannel_;
  NewConnectionCallback newConnectionCallback;
};


#endif //VAINGLORY_REACTOR_ACCEPTOR_H
