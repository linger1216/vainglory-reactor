//
// Created by Jingwei on 2024-11-16.
//

#include "socket_helper.h"
#include "log.h"
#include "network_encode.h"
#include <cassert>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <unistd.h>
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

int SocketHelper::Accept(int socketFd, const struct sockaddr* addr) {
  auto addrLen = static_cast<socklen_t>(sizeof(struct sockaddr_in));
  int clientFd = ::accept4(socketFd,
                           const_cast<sockaddr*>(addr),
                           &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (clientFd == -1) {
    Error("accept connection error");
  }

  if (clientFd < 0) {
    int savedErrno = errno;
    // the meaning of the errno??
    switch (savedErrno) {
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

ssize_t SocketHelper::Read(int socketFd, void* buf, size_t count) {
  return ::read(socketFd, buf, count);
}
ssize_t SocketHelper::Readv(int socketFd, const struct iovec* iov, int iovcnt) {
  return ::readv(socketFd, iov, iovcnt);
}
ssize_t SocketHelper::Write(int socketFd, const void* buf, size_t count) {
  return ::write(socketFd, buf, count);
}
void SocketHelper::Close(int socketFd) {
  int ret = ::close(socketFd);
  if (ret < 0) {
    Warn("close socket error:%d", ret);
  }
}
int SocketHelper::ShutdownWrite(int socketFd) {
  /*
   * 这个函数调用用于关闭指定套接字 socketFd 的写入功能。
   * SHUT_WR 参数表示关闭写入方向，即不再发送数据，但仍然可以接收数据。
   */
  int ret = ::shutdown(socketFd, SHUT_WR);
  if (ret < 0) {
    Warn("shutdown socket error:%d", ret);
  }
  return ret;
}

void SocketHelper::ToIpPort(char* buf, size_t size, const struct sockaddr* addr) {
  ToIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = to_sockaddr_in(addr);
  uint16_t port = networkToHost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, "%u", port);
}

void SocketHelper::ToIp(char* buf, size_t size, const struct sockaddr* addr) {
  if (addr->sa_family == AF_INET) {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in* addr4 = to_sockaddr_in(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  } else if (addr->sa_family == AF_INET6) {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6* addr6 = to_sockaddr_in6(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
}
int SocketHelper::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  return ::inet_pton(AF_INET, ip, &addr->sin_addr);
}
int SocketHelper::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = hostToNetwork16(port);
  return ::inet_pton(AF_INET, ip, &addr->sin6_addr);
}

int SocketHelper::GetSocketError(int socketFd) {
  int optval;
  auto optlen = static_cast<socklen_t>(sizeof optval);
  // get options at the socket api level on sockets
  if (::getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

bool SocketHelper::IsSelfConnect(int socketFd) {
  return false;
}
/*
 * 获取本地地址
 */
struct sockaddr_in6 SocketHelper::GetLocalAddr(int socketFd) {
  struct sockaddr_in6 localAddr;
  bzero(&localAddr, sizeof localAddr);
  auto addrLen = static_cast<socklen_t>(sizeof localAddr);
  int ret = ::getsockname(socketFd, to_sockaddr(&localAddr),
                          &addrLen);
  if (ret < 0) {
    Warn("Error in sockets::GetLocalAddr");
  }
  return localAddr;
}

/*
 * 获取对端地址
 */
struct sockaddr_in6 SocketHelper::GetPeerAddr(int socketFd) {
  struct sockaddr_in6 peerAddr;
  bzero(&peerAddr, sizeof peerAddr);
  socklen_t addrLen = static_cast<socklen_t>(sizeof peerAddr);
  int ret = ::getpeername(socketFd, to_sockaddr(&peerAddr),
                          &addrLen);
  if (ret < 0) {
    Warn("Error in sockets::GetPeerAddr");
  }

  return peerAddr;
}

const struct sockaddr* SocketHelper::to_sockaddr(const struct sockaddr_in* addr) {
  return reinterpret_cast<const struct sockaddr*>(addr);
}
struct sockaddr* SocketHelper::to_sockaddr(struct sockaddr_in6* addr) {
  return reinterpret_cast<struct sockaddr*>(addr);
}
const struct sockaddr* SocketHelper::to_sockaddr(const struct sockaddr_in6* addr) {
  return reinterpret_cast<const struct sockaddr*>(addr);
}
const struct sockaddr_in* SocketHelper::to_sockaddr_in(const struct sockaddr* addr) {
  return reinterpret_cast<const struct sockaddr_in*>(addr);
}
const struct sockaddr_in6* SocketHelper::to_sockaddr_in6(const struct sockaddr* addr) {
  return reinterpret_cast<const struct sockaddr_in6*>(addr);
}
int SocketHelper::SetTcpNoDelay(int socketFd, bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(socketFd, IPPROTO_TCP,
               TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    Warn("Error in sockets::SetTcpNoDelay %d", ret);
  }
  return ret;
}
int SocketHelper::SetReuseAddr(int socketFd, bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    Warn("Error in sockets::SetReuseAddr");
  }
  return ret;
}
int SocketHelper::SetReusePort(int socketFd, bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(socketFd, SOL_SOCKET, SO_REUSEPORT,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    Warn("Error in sockets::SetReusePort");
  }
  return ret;
}
int SocketHelper::SetKeepAlive(int socketFd, bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(socketFd, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    Warn("Error in sockets::SetKeepAlive");
  }
  return ret;
}
