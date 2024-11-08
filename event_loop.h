//
// Created by Jingwei on 2024-11-07.
//

#ifndef VAINGLORY_REACTOR_EVENT_LOOP_H
#define VAINGLORY_REACTOR_EVENT_LOOP_H

#include "no_copyable.h"
#include <thread>

class EventLoop : public NoCopyable {
public:
  EventLoop();
  ~EventLoop();
  void loop();
  void assertInLoopThread();
  bool isInLoopThread() const;
private:
  void abortNotInLoopThread();
  bool looping_;
  const std::thread::id threadId_;
};


#endif//VAINGLORY_REACTOR_EVENT_LOOP_H
