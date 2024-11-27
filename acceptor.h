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
  explicit Acceptor(EventLoop* loop, const INetAddress* listenAddr, AcceptCallback cb);
  ~Acceptor();
  int Listen();
private:
  void handleAccept();
  EventLoop* loop_;
  Channel* acceptChannel_;
  AcceptCallback acceptCallback;
};


#endif //VAINGLORY_REACTOR_ACCEPTOR_H
