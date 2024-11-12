//
// Created by Jingwei on 2024-11-03.
//

#include "buffer.h"
#include <bits/types/struct_iovec.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/uio.h>

Buffer::~Buffer() {
  delete [] data_;
}

Buffer::Buffer(int size)
    : readOffset_(0), writeOffset_(0),
      capacity_(size){
  data_ = new char [size];
  memset(data_, 0, size);
}

void Buffer::Extend(int size) {

  // 1. 内存够用
  if(GetWriteableSize() > size) {
    return;
  }

  // 2. 内存需要合并才够用
  // 剩余可写 + 已读 > size
  if (GetReadableSize() + GetWriteableSize() >= size) {
    // 移动数据
    memmove(data_, data_ + readOffset_, GetReadableSize());
    // 更新指针
    readOffset_ = 0;
    writeOffset_ = GetReadableSize();
    return;
  }

  // 3. 扩容
  auto buf = static_cast<char*>(realloc(data_, capacity_ + size));
  if (buf == nullptr) return;
  memset(buf + writeOffset_, 0, size);
  data_ = buf;
  capacity_ += size;

}

int Buffer::GetWriteableSize() const {
  return capacity_ - writeOffset_;
}

int Buffer::GetReadableSize() const {
  return writeOffset_ - readOffset_;
}

int Buffer::Append(const char* buf, int size) {
  if (buf == nullptr || size <= 0) return -1;

  Extend(size);

  memcpy(data_ + writeOffset_, buf, size);
  writeOffset_ += size;
  return size;
}

int Buffer::Append(const char* buf) {
  return Append(buf, static_cast<int>(strlen(buf)));
}

int Buffer::ReadSocket(int fd) {
  int writeable = GetWriteableSize();

  // [2] 长度根据实际需求来
  iovec iov[2];
  iov[0].iov_base = data_ + writeOffset_;
  iov[0].iov_len = writeable;

  int TEMP_BUFFER_SIZE = 40960;
  char* tmp = new char [TEMP_BUFFER_SIZE];
  memset(tmp, 0, TEMP_BUFFER_SIZE);
  iov[0].iov_base = tmp;
  iov[0].iov_len = TEMP_BUFFER_SIZE;
  int n = static_cast<int>(readv(fd, iov, 2));
  if (n < 0) {
    return -1;
  } else if (n <= writeable) {
    writeOffset_ += n;
  } else {
    writeOffset_ = capacity_;
    Append(tmp, n - writeable);
  }
  delete []tmp;
  return n;
}

int Buffer::SendSocket(int socket) {
  int readable = GetReadableSize();
  if (readable > 0) {
    int count = static_cast<int>(send(socket, data_ + readOffset_,
                                      readable, MSG_NOSIGNAL));
    if (count > 0) {
      readOffset_ += count;

      // TODO: 优化
      usleep(1);
    }
    return count;
  }
  return 0;
}
