//
// Created by Jingwei on 2024-11-09.
//

#include "epoll_dispatcher.h"
#include <sys/epoll.h>
#include "fd_event.h"
#include "log.h"

EpollDispatcher::EpollDispatcher(EventLoop* loop) {
  Debug("fuck");
}

EpollDispatcher::~EpollDispatcher() {
}

int EpollDispatcher::fdEvent(int events) {

  FDEvent revents;

  /*
   * EPOLLHUP 表示对端关闭了连接，但仍然可能有数据可读。
   * EPOLLIN 表示文件描述符可读。
   * 当 EPOLLHUP 和 EPOLLIN 同时出现时，通常意味着对端已经关闭了连接，
   * 但缓冲区中还有未读取的数据。
   * 在这种情况下，通常希望应用程序继续读取这些数据，而不是立即关闭连接
   */
  if ((events & EPOLLHUP) && !(events & EPOLLIN)) {
    revents = FDEvent::CloseEvent;
  }

  if (events & EPOLLERR)  {
    revents = FDEvent::ErrorEvent;
  }

  // 读和紧急读事件
  if (events & (EPOLLIN | EPOLLPRI))  {
    revents = FDEvent::ReadEvent;
  }

  if (events & EPOLLOUT)  {
    revents = FDEvent::WriteEvent;
  }
  return static_cast<int>(revents);
}

