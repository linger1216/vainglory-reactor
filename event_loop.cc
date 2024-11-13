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

EventLoop::EventLoop() : EventLoop("MainThread") {
}

EventLoop::EventLoop(const char* threadName)
    : threadId_(std::this_thread::get_id()),
      threadName_(threadName),
      isRunning_(false),
      dispatcher_(new EpollDispatcher(this)) {

  fd2ChannelMap_.clear();

  // TODO
  // 开启读事件, 每个EventLoop都将监听Epoll的EpollIN事件
  wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (wakeupFd_ < 0) {
    Error("create event fd failed");
  }

  wakeupChannel_ = new Channel(wakeupFd_, FDEvent::ReadEvent,
                               std::bind(&EventLoop::wakeupTaskRead, this),
                               nullptr,
                               nullptr, nullptr, this);
  Debug("create wakeup channel fd:%d addr:%p", wakeupFd_, wakeupChannel_);

  AddTask(wakeupChannel_, EventLoopOperator::Add);
  Debug("EventLoop:%s is start, addr:%p", threadName, this);
}

EventLoop::~EventLoop() {
  Debug("EventLoop %p stop looping", this);
  wakeupChannel_->DisableAll();
  dispatcher_->Delete(wakeupChannel_);
  delete wakeupChannel_;
  wakeupChannel_ = nullptr;
  close(wakeupFd_);
}

void EventLoop::Run() {
  assert(isRunning_ == false);
  isRunning_ = true;
  Debug("EventLoop %p start looping", this);
  while (isRunning_) {
    dispatcher_->Dispatch(TIMEOUT_MS);

    // 执行其他Loop派发过来需要处理的任务
    // main loop 注册一个cb, 需要sub loop来执行
    // 唤醒loop线程, 也就是Dispatch会唤醒，会自动进入到processTask的动作
    processTask();
  }
  Debug("EventLoop %p quit looping", this);
}
void EventLoop::ExecChannelCallback(int fd, FDEvent event) {
  Debug("channel fd:%d has Event %s", FDEventToString(event));
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
    Error("EventLoop::wakeupRead() reads %ld bytes instead of 8", n);
  }
}

void EventLoop::wakeupTask() const {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    Error("EventLoop::wakeupTask() writes %ld bytes instead of 8", n);
  }
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
  // TODO 可能有点小问题，判断empty并没有加锁
  while (!taskQueue_.empty()) {
    Node* node = nullptr;
    {
      std::lock_guard<std::mutex> locker(mutex_);
      if (taskQueue_.empty()) continue;
      node = taskQueue_.front();
      taskQueue_.pop();
    }
    if (node == nullptr) continue;
    switch (node->op) {
      case EventLoopOperator::Add:
        handleAddOperatorTask(node->channel);
        break;
      case EventLoopOperator::Update:
        handleDeleteOperatorTask(node->channel);
        break;
      case EventLoopOperator::Delete:
        handleModifyOperatorTask(node->channel);
        break;
    }
    delete node;
  }
}

int EventLoop::handleAddOperatorTask(Channel* channel) {
  int fd = channel->Fd();
  auto it = fd2ChannelMap_.find(fd);
  if (it == fd2ChannelMap_.end()) {
    fd2ChannelMap_.insert(std::make_pair(fd, channel));
    return dispatcher_->Add(channel);
  }
  return 0;
}

int EventLoop::handleDeleteOperatorTask(Channel* channel) {
  int fd = channel->Fd();
  auto it = fd2ChannelMap_.find(fd);
  if (it != fd2ChannelMap_.end()) {
    return dispatcher_->Delete(channel);
  }
  return 0;
}
int EventLoop::handleModifyOperatorTask(Channel* channel) {
  int fd = channel->Fd();
  auto it = fd2ChannelMap_.find(fd);
  if (it != fd2ChannelMap_.end()) {
    return dispatcher_->Update(channel);
  }
  return 0;
}
int EventLoop::AddTask(Channel* channel, EventLoopOperator type) {
  Debug("AddTask %p", channel);
  assert(channel != nullptr);
  {
    std::lock_guard<std::mutex> locker(mutex_);
    taskQueue_.push(new Node(channel, type));
  }
  if (!IsInLoopThread()) {
    Debug("EventLoop::AddTask() in other thread");
    wakeupTask();
  } else {
    Debug("EventLoop::AddTask() in loop thread");
    processTask();
  }
  return 0;
}

// 释放channel资源
int EventLoop::FreeChannel(Channel* channel) {
  auto it = fd2ChannelMap_.find(channel->Fd());
  if (it != fd2ChannelMap_.end()) {
    fd2ChannelMap_.erase(it);
    close(channel->Fd());
    delete channel;
  }
  return 0;
}
