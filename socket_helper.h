//
// Created by Jingwei on 2024-11-16.
//

#ifndef VAINGLORY_REACTOR_SOCKET_HELPER_H
#define VAINGLORY_REACTOR_SOCKET_HELPER_H

#include "no_copyable.h"
#include <arpa/inet.h>

class SocketHelper : public NoCopyable{
  static int CreateSocket(sa_family_t family);
  static int Connect(int socketFd, const struct sockaddr* addr);
  static int Bind(int socketFd, const struct sockaddr* addr);
  static int Listen(int socketFd);
  static int Accept(int socketFd, struct sockaddr_in6* addr);
  static ssize_t Read(int socketFd, void* buf, size_t count);
  static ssize_t Readv(int socketFd, const struct iovec* iov, int iovcnt);
  static ssize_t Write(int socketFd, const void* buf, size_t count);
  static void Close(int socketFd);
  static void ShutdownWrite(int socketFd);
  static void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
  static void ToIp(char* buf, size_t size, const struct sockaddr* addr);
  static void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
  static void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
  static int GetSocketError(int socketFd);
  static bool isSelfConnect(int socketFd);

private:
  static const struct sockaddr* cast_sockaddr(const struct sockaddr_in6* addr);
  static const struct sockaddr* cast_sockaddr(const struct sockaddr_in* addr);
  static const struct sockaddr_in* cast_sockaddr_in(const struct sockaddr* addr);
  static const struct sockaddr_in6* cast_sockaddr_in6(const struct sockaddr* addr);
};


#endif //VAINGLORY_REACTOR_SOCKET_HELPER_H
