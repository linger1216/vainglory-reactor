//
// Created by Jingwei on 2024-11-02.
//

#include "work_thread.h"
#include "event_loop.h"
#include "log.h"
WorkThread::WorkThread(int index)
    : thread_(nullptr), eventLoop_(nullptr) {
  name_ = "WorkEventLoop:" + std::to_string(index);

  Debug("%s created", name_.c_str());
}

WorkThread::~WorkThread() {
  delete thread_;
  delete eventLoop_;
}

//
void WorkThread::Run(){
  Debug("%s run", name_.c_str());
  thread_ = new std::thread(&WorkThread::subRun, this);
  // 如果没有条件变量， 可能在Run执行完成后，eventLoop还没有初始化完成，
  // 在后期使用_eventLoop的时候，可能造成空指针异常，导致程序崩溃。
  std::unique_lock<std::mutex> uniqueLock(mutex_);
  while (eventLoop_ == nullptr) {
    // 等待条件变量唤醒
    cond_.wait(uniqueLock);
  }
}

void WorkThread::subRun(){
  {
    std::lock_guard<std::mutex> locker(mutex_);
    eventLoop_ = new EventLoop(name_.c_str());
  }
  // 唤醒条件量, 唤醒主线程
  cond_.notify_one();
  eventLoop_->Run();
}

EventLoop* WorkThread::getEventLoop() const {
  return eventLoop_;
}

