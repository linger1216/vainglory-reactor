//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_EPOLL_DISPATCHER_H
#define VAINGLORY_REACTOR_EPOLL_DISPATCHER_H

#include "no_copyable.h"
class EventLoop;

class EpollDispatcher : public NoCopyable {
public:
  explicit EpollDispatcher(EventLoop* loop);
  ~EpollDispatcher();
  int fdEvent(int events);
private:
private:
  int epollfd_;
};


#endif //VAINGLORY_REACTOR_EPOLL_DISPATCHER_H
