//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_EPOLL_DISPATCHER_H
#define VAINGLORY_REACTOR_EPOLL_DISPATCHER_H

#include "dispatcher.h"
#include "fd_event.h"
#include "no_copyable.h"
#include <vector>

class EventLoop;
class Channel;
class epoll_event;

class EpollDispatcher : public NoCopyable, public IDispatcher {
public:
  explicit EpollDispatcher(EventLoop* loop);
  ~EpollDispatcher() override;
public:
  int Add(Channel* channel) override;
  int Delete(Channel* channel) override;
  int Update(Channel* channel) override;
  int Dispatch( int timeoutMs) override;

private:
  const int EPOLL_INIT_EVENTS = 128;
private:
  int epollControl(Channel* channel, int op) const;
  static FDEvent toFDEvent(int events);
private:
  EventLoop* loop_;
  int epollfd_;
  std::vector<struct epoll_event> events_;
};


#endif //VAINGLORY_REACTOR_EPOLL_DISPATCHER_H
