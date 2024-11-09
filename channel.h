//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_CHANNEL_H
#define VAINGLORY_REACTOR_CHANNEL_H

/*
 * 封装了fd和感兴趣的event，如：EPOLLIN EPOLLOUT
 *
 */

#include "no_copyable.h"
#include <functional>
#include <memory>
#include "time_stamp.h"

class EventLoop;
class TimeStamp;

class Channel : public NoCopyable {
public:
  using EventCallback = std::function<void(TimeStamp)>;
public:
  Channel(EventLoop* loop, int fd,
          EventCallback readCallback,
          EventCallback writeCallback,
          EventCallback closeCallback,
          EventCallback errorCallback);
  ~Channel();

  // 得到poller返回的， 来处理相对应的事情
  void HandleEvent(const TimeStamp& receiveTime);
  void HandleEventWithGuard(const TimeStamp& receiveTime);

  int fd() const;
  int events() const;

  // 当dispatcher监听到事件的时候，需要用这个接口，将channel的revents设置好
  void SetREvents(int revents);

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

  // 为Poller所用
  int Index() const;
  void SetIndex(int index);

  EventLoop* OwnerLoop();

  // 在channel所属的EventLoop中， 移除该channel
  // 还不明白为啥要这样用
  void Remove();

private:
  void update();

private:
  EventLoop* loop_;
  const int fd_;
  int events_; // 注册fd感兴趣的事件
  int revents_; // dispatcher 返回的具体发生的事件
  int index_;

  // 防止channel 被其他线程删除，此时还在执行回调操作
  std::weak_ptr<void> tie_;
  bool tied_;

  // 因为channel可以通过dispatcher获知fd最终发生的具体事件
  // 所以定义相关的callback函数
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};


#endif//VAINGLORY_REACTOR_CHANNEL_H
