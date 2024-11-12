//
// Created by Jingwei on 2024-11-02.
//

#ifndef REACTOR_CPP_WORK_THREAD_H
#define REACTOR_CPP_WORK_THREAD_H

#include "no_copyable.h"
#include <condition_variable>
#include <mutex>
#include <thread>

class EventLoop;

class WorkThread : public NoCopyable{
public:
  ~WorkThread();
  explicit WorkThread(int index);
public:
  void Run();
  EventLoop* getEventLoop() const;
private:
  void subRun();
private:
  std::thread* thread_;
  std::string name_;
  std::mutex mutex_;
  std::condition_variable cond_;
  EventLoop* eventLoop_;
};


#endif//REACTOR_CPP_WORK_THREAD_H
