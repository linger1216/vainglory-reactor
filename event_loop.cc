//
// Created by Jingwei on 2024-11-07.
//

#include "event_loop.h"

EventLoop::EventLoop() {

}

EventLoop::~EventLoop() {

}

bool EventLoop::isInLoopThread() const {
  return threadId_ == std::this_thread::get_id();
}

void EventLoop::loop() {

}

void EventLoop::abortNotInLoopThread(){
  // do some logging things
  // LOG_FATAL level
}


void EventLoop::assertInLoopThread() {
  if (!isInLoopThread()) {
    abortNotInLoopThread();
  }
}
void EventLoop::UpdateChannel(Channel *channel) {

}

void EventLoop::RemoveChannel(Channel* channel) {
}
