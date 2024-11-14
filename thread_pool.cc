//
// Created by Jingwei on 2024-11-02.
//

#include "thread_pool.h"
#include "event_loop.h"
#include "work_thread.h"
#include "log.h"

ThreadPool::~ThreadPool() {
  for (int i = 0; i < threadNum_; ++i) {
    auto thread = threads_[i];
    // todo
    // thread->Stop();
    delete thread;
  }
}

ThreadPool::ThreadPool(EventLoop* mainEventLoop, int threadNum)
    : mainEventLoop_(mainEventLoop),
      threadNum_(threadNum),
      currentThreadIndex_(0),
      isRunning_(false) {
  for (int i = 0; i < threadNum_; ++i) {
    threads_.push_back(new WorkThread(i));
  }
  Debug("ThreadPool is created %d thread", threadNum_);
}

void ThreadPool::Run() {
  if (!mainEventLoop_->IsInLoopThread()) {
    Error("main thread should call Run");
  }
  isRunning_ = true;
  for (int i = 0; i < threadNum_; ++i) {
    threads_[i]->Run();
  }
}

EventLoop* ThreadPool::TakeEventLoop() {
  if (mainEventLoop_->IsInLoopThread()) {
    Error("main thread should call TakeEventLoop");
  }

  // 从工作线程中取出一个合适的反应堆实例, 如果工作线程没有， 就用主线程的
  if (threadNum_ > 0) {
    return threads_[currentThreadIndex_++ % threadNum_]->getEventLoop();
  }
  return mainEventLoop_;
}
