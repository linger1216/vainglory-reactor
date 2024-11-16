//
// Created by Jingwei on 2024-11-16.
//

#include "socket_helper.h"
#include "log.h"

int SocketHelper::CreateSocket(sa_family_t family) {
  /*
    创建一个套接字（socket）并返回其文件描述符 socketFd。
    family 指定地址族，例如 AF_INET 表示 IPv4。
    SOCK_STREAM 表示使用面向连接的 TCP 协议。
    SOCK_NONBLOCK 表示套接字为非阻塞模式。
    SOCK_CLOEXEC 表示在执行 exec 系统调用时自动关闭该套接字。
    IPPROTO_TCP 指定传输协议为 TCP
   */
  int socketFd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (socketFd < 0) {
    Error("create socket error:%d", socketFd);
  }
  return socketFd;
}

int SocketHelper::Connect(int socketFd, const struct sockaddr* addr) {
  return ::connect(socketFd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

int SocketHelper::Bind(int socketFd, const struct sockaddr* addr) {
  int ret = ::bind(socketFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0) {
    Error("bind socket error:%d", ret);
  }
  return ret;
}

int SocketHelper::Listen(int socketFd) {
  int ret = ::listen(socketFd, SOMAXCONN);
  if (ret < 0) {
    Error("listen socket error:%d", ret);
  }
  return ret;
}

int SocketHelper::Accept(int socketFd, struct sockaddr_in6* addr) {
  auto addrLen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
  int clientFd = ::accept4(socketFd,
                          const_cast<sockaddr*>(cast_sockaddr(addr)),
                          &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (clientFd == -1) {
    Error("accept connection error");
  }

  if (clientFd < 0) {
    int savedErrno = errno;
    // the meaning of the errno??
    switch (savedErrno){
      // temporary error
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        // expected errors
        break;
      // fatal error
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        Error("accept connection error: %s", strerror(savedErrno));
        break;
      default:
        break;
    }
    Warn("accept connection error: %s", strerror(savedErrno));
  }
  return clientFd;
}
