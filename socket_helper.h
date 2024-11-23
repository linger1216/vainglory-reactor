//
// Created by Jingwei on 2024-11-16.
//

#ifndef VAINGLORY_REACTOR_SOCKET_HELPER_H
#define VAINGLORY_REACTOR_SOCKET_HELPER_H

#include "no_copyable.h"
#include <arpa/inet.h>

class SocketHelper : public NoCopyable{
public:
  static int CreateSocket(sa_family_t family);
  static int Connect(int socketFd, const struct sockaddr* addr);
  static int Bind(int socketFd, const struct sockaddr* addr);
  static int Listen(int socketFd);
  static int Accept(int socketFd, const struct sockaddr* addr);
  static ssize_t Read(int socketFd, void* buf, size_t count);
  static ssize_t Readv(int socketFd, const struct iovec* iov, int iovcnt);
  static ssize_t Write(int socketFd, const void* buf, size_t count);
  static void Close(int socketFd);
  static int ShutdownWrite(int socketFd);
  static void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
  static void ToIp(char* buf, size_t size, const struct sockaddr* addr);
  static int FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
  static int GetSocketError(int socketFd);
  static bool IsSelfConnect(int socketFd);
  static struct sockaddr_in GetLocalAddr(int socketFd);
  static struct sockaddr_in GetPeerAddr(int socketFd);
  static const struct sockaddr* to_sockaddr(const struct sockaddr_in* addr);
  static const struct sockaddr_in* to_sockaddr_in(const struct sockaddr* addr);

  // Set sockfd operations
  static int SetTcpNoDelay(int socketFd, bool on); // Nagle's algorithm
  static int SetReuseAddr(int socketFd, bool on);
  static int SetReusePort(int socketFd, bool on);
  static int SetKeepAlive(int socketFd, bool on);

};


#endif //VAINGLORY_REACTOR_SOCKET_HELPER_H
