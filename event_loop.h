//
// Created by Jingwei on 2024-11-07.
//

#ifndef VAINGLORY_REACTOR_EVENT_LOOP_H
#define VAINGLORY_REACTOR_EVENT_LOOP_H

#include "fd_event.h"
#include "no_copyable.h"
#include <thread>
#include <queue>
#include <list>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <functional>
#include "time_stamp.h"


/*
 * 有一个逻辑很奇怪，既然是one loop per thread
 * 再程序启动的时候，开始多少个loop就开启了多少个线程， 既然一开始都分配好了
 * 无非就是运行期间增加channel，为什么loop还会切换线程？
 * 这一点还没有想通。
 */


enum class EventLoopOperator:char {
  Add = 0,
  Update = 1,
  Delete = 2,
};


inline const char* EventLoopOperatorToString(EventLoopOperator event) {
  switch (event) {
    case EventLoopOperator::Add:
      return "Add";
    case EventLoopOperator::Update:
      return "Update";
    case EventLoopOperator::Delete:
      return "Delete";
    default:
      return "Unknown";
  }
}


class Channel;
class IDispatcher;

class EventLoop : public NoCopyable {
public:
  using Functor = std::function<void()>;
public:
  EventLoop();
  explicit EventLoop(const char* threadName);
  ~EventLoop();
public:
  // 开启事件循环
  void Run();
  // 退出事件循环
  void Quit();

  std::thread::id GetThreadId() const;
  // 判断当前loop是否在自己的线程中
  bool IsInLoopThread() const;

  // dispatch检测到某个channel的事件被激活, 调用了该函数
  // 该函数寻找到对应的Channel，进行对应的事件回调。
  void ExecChannelCallback(int fd, FDEvent event);

  // 添加任务到任务队列
  int AddTask(Channel* channel, EventLoopOperator type);

  // 删除channel对象
  int DestroyChannel(Channel* channel);

  // 返回当前loop的名字
  const char* Name() const;
private:
  const int TIMEOUT_MS = 10000;
  struct Node {
    Node(Channel* channel, EventLoopOperator op) : op(op), channel(channel) {}
    EventLoopOperator op;
    Channel* channel;
  };

private:
  // 线程被唤醒后的读回调 （将唤醒发送的数据，读出来，清空读缓存区）
  void wakeupTaskRead() const;
  // 唤醒线程处理任务 （向唤醒的线程写数据， 唤醒线程）
  void wakeupTask() const;

  // 任务相关

  // 处理任务队列中的任务
  // 任务中的函数，可能会被多个Loop来调用， 所以会有互斥访问
  // 所以要加锁
  void processTask();
  // 处理AddTask中的ADD事件
  int handleAddOperatorTask(Channel* channel);
  // 处理AddTask中的删除事件
  int handleDeleteOperatorTask(Channel* channel);
  // 处理AddTask中的修改事件
  int handleModifyOperatorTask(Channel* channel);

private:
  // 退出标志
  std::atomic_bool isRunning_;

  // 事件分发器抽象
  std::unique_ptr<IDispatcher> dispatcher_;

  // 任务队列, 用来存储任务，类似muduo的pending functor
//  std::queue<Node*> taskQueue_;
  std::list<Node*> taskQueue_;
  // 用来保护上述队列的互斥锁
  std::mutex mutex_;

  // channel_ Map，映射关系描述符和channel
  std::unordered_map<int, Channel*> fd2ChannelMap_;

  // 线程相关
  // 记录当前loop所在线程id
  std::thread::id threadId_;
  std::string threadName_;


  // 线程通信 有两个方案，
  // 1. 是socket pair
  // [0]: 发送数据
  // [1]: 接收数据
  //  int socketPair_[2];

  // 2. 是event fd
  // 当mainloop获取一个新用户的channel后，通过轮询算法选择一个sub loop
  // 通过该成员唤醒sub loop的睡眠，以便处理新用户的注册事件
  int wakeupFd_;
  Channel* wakeupChannel_;
};


#endif//VAINGLORY_REACTOR_EVENT_LOOP_H
