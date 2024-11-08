//
// Created by Jingwei on 2024-11-07.
//

#ifndef VAINGLORY_REACTOR_THREAD_H
#define VAINGLORY_REACTOR_THREAD_H

#include "no_copyable.h"
#include <functional>
#include <thread>

class Thread : public NoCopyable{
public:
  using ThreadFunc =  std::function<void ()>;

  Thread(const ThreadFunc&);
  ~Thread();

  void start();
  int join();

  bool started();
  pid_t tid();

private:
  bool started_;
  bool joined_;
  pid_t tid_;
  std::thread pthreadId_;
  ThreadFunc func_;
};


#endif//VAINGLORY_REACTOR_THREAD_H
