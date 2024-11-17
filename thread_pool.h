//
// Created by Jingwei on 2024-11-02.
//

#ifndef REACTOR_CPP_THREAD_POOL_H
#define REACTOR_CPP_THREAD_POOL_H

#include "no_copyable.h"
#include <vector>
class WorkThread;
class EventLoop;

class ThreadPool : public NoCopyable{
public:
  ~ThreadPool();
  explicit ThreadPool(EventLoop* mainEventLoop, int threadNum);

public:
  void Run();
  EventLoop* GetNextEventLoop();
private:
  EventLoop* mainEventLoop_;

  // 是否启动
  bool isRunning_;

  // 工作线程数量
  int threadNum_;

  // 工作线程数组
  std::vector<WorkThread*> threads_;

  // 当前工作线程编号
  int currentThreadIndex_;
};


#endif//REACTOR_CPP_THREAD_POOL_H
