//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_CHANNEL_H
#define VAINGLORY_REACTOR_CHANNEL_H

/*
 * 封装了fd和感兴趣的event，如：EPOLLIN EPOLLOUT
 */

#include "fd_event.h"
#include "no_copyable.h"
#include "time_stamp.h"
#include <functional>
#include <memory>

class EventLoop;

class Channel : public NoCopyable {
public:
  using EventCallback = std::function<void(void* arg)>;
public:
  Channel(int fd, FDEvent events,
          EventCallback readCallback, EventCallback writeCallback,
          EventCallback closeCallback,EventCallback errorCallback,
          void* arg);
  ~Channel();

  void* Arg();

  // 得到poller返回的， 来处理相对应的事情
  void ExecCallback(void* arg, FDEvent event);

  int Fd() const;
  int Events() const;

  // 控制监听事件
  void EnableReading();
  void DisableReading();
  void EnableWriting();
  void DisableWriting();
  void DisableAll();
  bool IsNoneEvent() const;
  bool IsReading() const;
  bool IsWriting() const;

  // 防止当channel被手动remove掉，channel还在执行回调。
  void Tie(const std::shared_ptr<void>&);
private:
  void execCallbackWithGuard(void* arg, FDEvent event);

private:
  EventLoop* loop_;
  const int fd_;
  int events_; // 注册fd感兴趣的事件

  // 防止channel 被其他线程删除，此时还在执行回调操作
  std::weak_ptr<void> tie_;
  bool tied_;

  // 因为channel可以通过dispatcher获知fd最终发生的具体事件
  // 所以定义相关的callback函数
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;

  // 回调函数的参数
  // 指向: tcp connection 对象
  void* arg_;
};


#endif//VAINGLORY_REACTOR_CHANNEL_H
