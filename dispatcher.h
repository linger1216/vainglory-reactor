//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_DISPATCHER_H
#define VAINGLORY_REACTOR_DISPATCHER_H


class Channel;
class IDispatcher {
public:
  virtual ~IDispatcher() = default;
  virtual int Add(Channel* channel) = 0;
  virtual int Delete(Channel* channel) = 0;
  virtual int Update(Channel* channel) = 0;
  // timeout: ms
  virtual int Dispatch( int timeoutMs) = 0;
};



#endif //VAINGLORY_REACTOR_DISPATCHER_H
