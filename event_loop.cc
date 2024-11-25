//
// Created by Jingwei on 2024-11-07.
//

#include "event_loop.h"
#include "channel.h"
#include "epoll_dispatcher.h"
#include "log.h"
#include <cassert>
#include <csignal>
#include <sys/eventfd.h>
#include <sys/socket.h>

EventLoop::EventLoop() : EventLoop("MainEventLoop") {
}

EventLoop::EventLoop(const char* threadName)
    : threadId_(std::this_thread::get_id()),
      threadName_(threadName),
      isRunning_(false),
      dispatcher_(new EpollDispatcher(this)) {

  Debug("%s is create,", threadName);

  fd2ChannelMap_.clear();

  // TODO
  // 开启读事件, 每个EventLoop都将监听Epoll的EpollIN事件
  wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wakeupFd_ < 0) {
    Error("create wake up event fd failed");
  }

  wakeupChannel_ = new Channel(wakeupFd_, FDEvent::ReadEvent,
                               std::bind(&EventLoop::wakeupTaskRead, this),
                               nullptr,
                               nullptr, nullptr, this);

  AddChannelEventInLoop(wakeupChannel_);
}

EventLoop::~EventLoop() {
  Debug("EventLoop %p stop looping", this);
  wakeupChannel_->DisableAll();
  //  AddTask(wakeupChannel_, EventLoopOperator::Update);
  //  AddTask(wakeupChannel_, EventLoopOperator::Delete);
  delete wakeupChannel_;
  close(wakeupFd_);
  fd2ChannelMap_.clear();
}

void EventLoop::Run() {
  assert(isRunning_ == false);
  isRunning_ = true;
  //  Debug("EventLoop:%s is running", threadName_.c_str());
  while (isRunning_) {
    // Debug("EventLoop:%s is dispatch", threadName_.c_str());
    dispatcher_->Dispatch(TIMEOUT_MS);

    // 执行其他Loop派发过来需要处理的任务
    // main loop 注册一个cb, 需要sub loop来执行
    // 唤醒loop线程, 也就是Dispatch会唤醒，会自动进入到processTask的动作
    // Debug("EventLoop:%s is process task", threadName_.c_str());
    processTask();
  }
  Debug("EventLoop %p quit looping", this);
}
void EventLoop::ExecChannelCallback(int fd, FDEvent event) {
  auto it = fd2ChannelMap_.find(fd);
  assert(it != fd2ChannelMap_.end());
  Channel* channel = fd2ChannelMap_[fd];
  fd2ChannelMap_[fd]->ExecCallback(channel->Arg(), event);
}

bool EventLoop::IsInLoopThread() const {
  return threadId_ == std::this_thread::get_id();
}

void EventLoop::wakeupTaskRead() const {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    Error("wakeupRead reads %ld bytes instead of 8", n);
  }
  Debug("%s wakeupTask read signal", threadName_.c_str());
}

void EventLoop::wakeupTask() const {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    Error("wakeupTask writes %ld bytes instead of 8", n);
  }
  Debug("%s wakeupTask writes signal", threadName_.c_str());
}


// 1. 在自己的线程中，调用Quit
// 这种情况比较简单，如果能调用Quit的话，说明dispatch没有阻塞，再下一个循环后，就能退出了
// 2. 在其他的线程中，调用Quit, 需要唤醒
// 唤醒后，Dispatcher会立即返回，然后执行后面processTask，再到下一个循环，就退出
void EventLoop::Quit() {
  isRunning_ = false;
  if (!IsInLoopThread()) {
    wakeupTask();
  }
}

void EventLoop::processTask() {
  while (!functors_.empty()) {
    Functor node = nullptr;
    {
      std::lock_guard<std::mutex> locker(mutex_);
      node = functors_.front();
      functors_.pop_front();
    }
    if (node == nullptr) continue;
    node();
    //    Debug("%s processFunctor channel:%d【%s】[%p]", threadName_.c_str(), node->channel->Fd(), EventLoopOperatorToString(node->op)
    //    switch (node->op) {
    //      case EventLoopOperator::Add:
    //        handleAddOperatorTask(node->channel);
    //        break;
    //      case EventLoopOperator::Update:
    //        handleModifyOperatorTask(node->channel);
    //        break;
    //      case EventLoopOperator::Delete:
    //        handleDeleteOperatorTask(node->channel);
    //        break;
    //    }
    //    delete node;
  }
}

//int EventLoop::handleAddOperatorTask(Channel* channel) {
////  int fd = channel->Fd();
////  auto it = fd2ChannelMap_.find(fd);
////  if (it == fd2ChannelMap_.end()) {
////    fd2ChannelMap_.insert(std::make_pair(fd, channel));
////    return dispatcher_->Add(channel);
////  }
//  return 0;
// temp
//}

// 释放channel资源
int EventLoop::DestroyChannel(Channel* channel) {
  auto it = fd2ChannelMap_.find(channel->Fd());
  if (it != fd2ChannelMap_.end()) {
    fd2ChannelMap_.erase(it);
    close(channel->Fd());
    Debug("%s 删除fd2ChannelMap_中的channel映射, 关闭fd描述符", threadName_.c_str());
  }
  return 0;
}
std::thread::id EventLoop::GetThreadId() const {
  return threadId_;
}
const char* EventLoop::Name() const {
  return threadName_.c_str();
}
int EventLoop::RunInLoop(EventLoop::Functor fn) {
  return IsInLoopThread() ? fn() : QueueInLoop(fn);
}

int EventLoop::QueueInLoop(EventLoop::Functor fn) {

  {
    std::lock_guard<std::mutex> locker(mutex_);
    functors_.push_back(fn);
  }
  Debug("Queue has %d functors", functors_.size());
  if (!IsInLoopThread()) {
    wakeupTask();
  } else {
    processTask();
  }
  return 0;
}

int EventLoop::AddChannelEventInLoop(Channel* channel) {
  return RunInLoop(std::bind(&EventLoop::addChannelEvent, this, channel));
}
int EventLoop::UpdateChannelEventInLoop(Channel* channel) {
  return RunInLoop(std::bind(&EventLoop::updateChannelEvent, this, channel));
}
int EventLoop::DeleteChannelEventInLoop(Channel* channel) {
  return RunInLoop(std::bind(&EventLoop::deleteChannelEvent, this, channel));
}

int EventLoop::addChannelEvent(Channel* channel) {
  fd2ChannelMap_.insert(std::make_pair(channel->Fd(), channel));
  return dispatcher_->Add(channel);
}
int EventLoop::updateChannelEvent(Channel* channel) {
  return dispatcher_->Update(channel);
}
int EventLoop::deleteChannelEvent(Channel* channel) {
  fd2ChannelMap_.erase(channel->Fd());
  return dispatcher_->Delete(channel);
}
void EventLoop::AssertInLoop() {
  assert(IsInLoopThread());
}
