//
// Created by Jingwei on 2024-11-09.
//

#include "epoll_dispatcher.h"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

#include "fd_event.h"
#include "log.h"

#include "channel.h"
#include "event_loop.h"
#include "time_stamp.h"
#include <cstring>

EpollDispatcher::EpollDispatcher(EventLoop* loop)
    : loop_(loop),
      epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      events_(EPOLL_INIT_EVENTS) {
  assert(epollfd_ > 0);
}

EpollDispatcher::~EpollDispatcher() {
  close(epollfd_);
}

int EpollDispatcher::Add(Channel* channel) {
  return epollControl(channel, EPOLL_CTL_ADD);
}

int EpollDispatcher::Delete(Channel* channel) {
  int ret = epollControl(channel, EPOLL_CTL_DEL);
  if (ret == -1) {
    Warn("epoll_ctl del error, fd=%d, channel=%p", channel->Fd(), channel);
  }

  // TODO
  // 需要把当前channel的fd资源删除
  // 本意要执行destroy的回调。但tm需要仔细思考思考
  return 0;
}

int EpollDispatcher::Update(Channel* channel) {
  return epollControl(channel, EPOLL_CTL_MOD);
}

int EpollDispatcher::Dispatch(int timeoutMs) {
  int numEvents = epoll_wait(epollfd_, &*events_.begin(),
                         static_cast<int>(events_.size()),
                         timeoutMs);
  if (numEvents > 0) {
    Debug("EventLoop %p wakeup with %d events handlers", loop_, numEvents);
    // 我们的设计更好, 直接在这里调用cb，
    // muduo还要两次循环, 这里一次，event loop还有一次
    for (int i = 0; i < numEvents; i++) {
      // 因为每个模型（select，poll，epoll）事件不一样，这里要把返回的事件
      // 统一抽象成FDEvent，channel会根据FDEvent来执行Callback
      auto fdEvent = toFDEvent(int(events_[i].events));
      int fd = events_[i].data.fd;
      loop_->ExecChannelCallback(fd, fdEvent);
    }
    // 检查一下， 不够扩容
    if (numEvents == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    Warn("epoll_wait timeout");
  } else {
    Warn("epoll_wait error");
  }

  return numEvents;
}


int EpollDispatcher::epollControl(Channel* channel, int op) const {

  // 因为channel是一层抽象，所以没办法只能使用抽象的FDEVENT
  // 所以在这里要做一层转义，
  // 如果是为了纯追求效率，channel只存Epoll的版本就好，
  // 但我想使用select跨平台的特性在本地调试，所以，我就抽象了一层。
  int events = 0;
  if (channel->IsReading()) {
    events |= EPOLLIN;
  }
  if (channel->IsWriting()) {
    events |= EPOLLOUT;
  }

  epoll_event ev{};
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = channel->Fd();
  ev.events = events;
  return epoll_ctl(epollfd_, op, channel->Fd(), &ev);
}

FDEvent EpollDispatcher::toFDEvent(int events) {
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
  if (events & EPOLLERR) {
    revents = FDEvent::ErrorEvent;
  }
  // 读和紧急读事件
  if (events & (EPOLLIN | EPOLLPRI)) {
    revents = FDEvent::ReadEvent;
  }
  if (events & EPOLLOUT) {
    revents = FDEvent::WriteEvent;
  }
  return revents;
}