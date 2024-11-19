//
// Created by Jingwei on 2024-11-09.
//

#include "channel.h"
#include "event_loop.h"
#include "fd_event.h"
#include "log.h"

Channel::Channel(int fd, FDEvent events,
                 Channel::EventCallback readCallback, Channel::EventCallback writeCallback,
                 Channel::EventCallback closeCallback, Channel::EventCallback errorCallback,
                 void* arg)
    : fd_(fd), events_(static_cast<int>(events)),
      tied_(false),
      arg_(arg),
      readCallback_(std::move(readCallback)),
      writeCallback_(std::move(writeCallback)),
      closeCallback_(std::move(closeCallback)),
      errorCallback_(std::move(errorCallback)) {
}

Channel::~Channel() {
}
int Channel::Fd() const {
  return fd_;
}
int Channel::Events() const {
  return events_;
}

void Channel::EnableReading() {
  events_ |= static_cast<int>(FDEvent::ReadEvent);
}
void Channel::DisableReading() {
  events_ &= ~static_cast<int>(FDEvent::ReadEvent);
}

void Channel::EnableWriting() {
  events_ |= static_cast<int>(FDEvent::WriteEvent);
}

void Channel::DisableWriting() {
  events_ &= ~static_cast<int>(FDEvent::WriteEvent);
}

void Channel::DisableAll() {
  events_ = static_cast<int>(FDEvent::None);
}

bool Channel::IsReading() const {
  return events_ & static_cast<int>(FDEvent::ReadEvent);
}
bool Channel::IsWriting() const {
  return events_ & static_cast<int>(FDEvent::WriteEvent);
}
bool Channel::IsNoneEvent() const {
  return events_ == static_cast<int>(FDEvent::None);
}

// TODO 什么时候会调用？
void Channel::Tie(const std::shared_ptr<void>& obj) {
  // 绑定了一个对象
  tie_ = obj;
  tied_ = true;
}

void Channel::ExecCallback(void* arg, FDEvent event) {
  if (tied_) {
    // 尝试弱指针提升强指针
    std::shared_ptr<void> guard = tie_.lock();
    // 如果成功，说明Channel仍然有效，否则已经释放了
    if (guard) {
      execCallbackWithGuard(arg, event);
    } else {
      Warn("channel object already down.");
    }
  } else {
    execCallbackWithGuard(arg, event);
  }
}
// TODO
// 这个事件是转义的,需要在poller那里做好适配
void Channel::execCallbackWithGuard(void* arg, FDEvent event) {
  if ((event == FDEvent::ReadEvent) && readCallback_) {
    Debug("Fd = %d Events = %d handle read event [%p]", fd_, event, std::this_thread::get_id());
    readCallback_(arg);
  }
  if ((event == FDEvent::WriteEvent) && writeCallback_) {
    Debug("Fd = %d Events = %d handle write event [%p]", fd_, event, std::this_thread::get_id());
    writeCallback_(arg);
  }
  if ((event == FDEvent::ErrorEvent) && errorCallback_) {
    Debug("Fd = %d Events = %d handle error event [%p]", fd_, event, std::this_thread::get_id());
    errorCallback_(arg);
  }
  if ((event == FDEvent::CloseEvent) && closeCallback_) {
    Debug("Fd = %d Events = %d handle close event [%p]", fd_, event, std::this_thread::get_id());
    closeCallback_(arg);
  }
}
void* Channel::Arg() {
  return arg_;
}
const char* Channel::EventsToString() const {
  return FDEventToString(static_cast<FDEvent>(events_));
}
