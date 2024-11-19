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

  Debug("%s is create, [%p]", threadName, std::this_thread::get_id());

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
  AddTask(wakeupChannel_, EventLoopOperator::Add);
}

EventLoop::~EventLoop() {
  Debug("EventLoop %p stop looping", this);
  wakeupChannel_->DisableAll();
  AddTask(wakeupChannel_, EventLoopOperator::Update);
  AddTask(wakeupChannel_, EventLoopOperator::Delete);
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
  Debug("%s wakeupTask read signal [%p]", threadName_.c_str(), std::this_thread::get_id());
}

void EventLoop::wakeupTask() const {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    Error("wakeupTask writes %ld bytes instead of 8", n);
  }
  Debug("%s wakeupTask writes signal [%p]", threadName_.c_str(), std::this_thread::get_id());
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
      taskQueue_.pop_front();
    }
    if (node == nullptr) continue;

    Debug("%s processTask channel:%d【%s】[%p]", threadName_.c_str(), node->channel->Fd(), EventLoopOperatorToString(node->op), std::this_thread::get_id());
    switch (node->op) {
      case EventLoopOperator::Add:
        handleAddOperatorTask(node->channel);
        break;
      case EventLoopOperator::Update:
        handleModifyOperatorTask(node->channel);
        break;
      case EventLoopOperator::Delete:
        handleDeleteOperatorTask(node->channel);
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
  assert(channel != nullptr);
  {
    std::lock_guard<std::mutex> locker(mutex_);
    auto node = new Node(channel, type);
//    Debug("%s addTask channel:%d【%s】[%p]", threadName_.c_str(), channel->Fd(), EventLoopOperatorToString(type), std::this_thread::get_id());
//    taskQueue_.push(node);
    taskQueue_.push_back(node);
  }
  for(auto iter = taskQueue_.begin(); iter != taskQueue_.end(); iter++) {
    Debug("%s QUEUE -> channel:%d【%s】[%p]", threadName_.c_str(), channel->Fd(), EventLoopOperatorToString(type), std::this_thread::get_id());
  }

  if (!IsInLoopThread()) {
    wakeupTask();
  } else {
    processTask();
  }
  return 0;
}

// 释放channel资源
int EventLoop::DestroyChannel(Channel* channel) {
  auto it = fd2ChannelMap_.find(channel->Fd());
  if (it != fd2ChannelMap_.end()) {
    fd2ChannelMap_.erase(it);
    close(channel->Fd());
    Debug("%s 删除fd2ChannelMap_中的channel映射, 关闭fd描述符 [%p]", threadName_.c_str(), std::this_thread::get_id());
  }
  return 0;
}
std::thread::id EventLoop::GetThreadId() const {
  return threadId_;
}
const char* EventLoop::Name() const {
  return threadName_.c_str();
}
