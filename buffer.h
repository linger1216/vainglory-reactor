//
// Created by Jingwei on 2024-11-03.
//

#ifndef REACTOR_CPP_BUFFER_H
#define REACTOR_CPP_BUFFER_H

#include "no_copyable.h"
class Buffer : public NoCopyable{
public:
  ~Buffer();
  explicit Buffer(int size);
public:
  // 扩容
  void Extend(int size);
  // 还有剩余可写数据的数据长度
  int GetWriteableSize() const;
  // 还有多少数据没有读
  int GetReadableSize() const;

  int Append(const char* buf);
  int Append(const char* buf, int size);

  int ReadSocket(int fd, int* reason);
  int SendSocket(int socket);

  int Read(char* buf, int size);

private:
  const int TCP_BUFFER_SIZE = 64 * 1024;
  // 指向内存的指针
  char* data_;
  int capacity_;
  int readOffset_;
  int writeOffset_;
};


#endif//REACTOR_CPP_BUFFER_H
