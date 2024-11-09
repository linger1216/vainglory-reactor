//
// Created by Jingwei on 2024-11-09.
//

#include "channel.h"
#include "event_loop.h"
#include "fd_event.h"
#include "log.h"

Channel::Channel(EventLoop* loop, int fd,
                 Channel::EventCallback readCallback,
                 Channel::EventCallback writeCallback,
                 Channel::EventCallback closeCallback,
                 Channel::EventCallback errorCallback)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1),
      tied_(false),
      readCallback_(std::move(readCallback)),
      writeCallback_(std::move(writeCallback)),
      closeCallback_(std::move(closeCallback)),
      errorCallback_(std::move(errorCallback)) {
}

Channel::~Channel() {
  Debug("~Channel need ensure no event to handle.");
}
int Channel::fd() const {
  return fd_;
}
int Channel::events() const {
  return events_;
}
void Channel::SetREvents(int revents) {
  revents_ = revents;
}
void Channel::EnableReading() {
  events_ |= static_cast<int>(FDEvent::ReadEvent);
  update();
}
void Channel::DisableReading() {
  events_ &= ~static_cast<int>(FDEvent::ReadEvent);
  update();
}

void Channel::EnableWriting() {
  events_ |= static_cast<int>(FDEvent::WriteEvent);
  update();
}

void Channel::DisableWriting() {
  events_ &= ~static_cast<int>(FDEvent::WriteEvent);
  update();
}

void Channel::DisableAll() {
  events_ = static_cast<int>(FDEvent::None);
  update();
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
void Channel::update() {
  loop_->UpdateChannel(this);
}

int Channel::Index() const {
  return index_;
}
void Channel::SetIndex(int index) {
  index_ = index;
}

EventLoop* Channel::OwnerLoop() {
  return loop_;
}
void Channel::Remove() {
  loop_->RemoveChannel(this);
}

// TODO 什么时候会调用？
void Channel::Tie(const std::shared_ptr<void>& obj) {
  // 绑定了一个对象
  tie_ = obj;
  tied_ = true;
}

void Channel::HandleEvent(const TimeStamp& receiveTime) {
  if (tied_) {
    // 尝试弱指针提升强指针
    std::shared_ptr<void> guard = tie_.lock();
    // 如果成功，说明Channel仍然有效，否则已经释放了
    if (guard) {
      HandleEventWithGuard(receiveTime);
    } else {
      Warn("channel object already down.");
    }
  } else {
    HandleEventWithGuard(receiveTime);
  }
}

// TODO
// 这个事件是转义的,需要在poller那里做好适配
void Channel::HandleEventWithGuard(const TimeStamp& receiveTime) {
  if ((revents_ & static_cast<int>(FDEvent::ReadEvent)) && readCallback_) {
    Debug("fd = %d events = %d handle read event", fd_, revents_);
    readCallback_(receiveTime);
  }
  if ((revents_ & static_cast<int>(FDEvent::WriteEvent)) && writeCallback_) {
    Debug("fd = %d events = %d handle write event", fd_, revents_);
    writeCallback_(receiveTime);
  }
  if (revents_ & static_cast<int>(FDEvent::ErrorEvent) && errorCallback_) {
    Debug("fd = %d events = %d handle error event", fd_, revents_);
    errorCallback_(receiveTime);
  }
  if (revents_ & static_cast<int>(FDEvent::CloseEvent) && closeCallback_) {
    Debug("fd = %d events = %d handle close event", fd_, revents_);
    closeCallback_(receiveTime);
  }
}
