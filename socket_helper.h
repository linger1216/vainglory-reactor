//
// Created by Jingwei on 2024-11-16.
//

#ifndef VAINGLORY_REACTOR_SOCKET_HELPER_H
#define VAINGLORY_REACTOR_SOCKET_HELPER_H

#include "no_copyable.h"
#include <arpa/inet.h>

class SocketHelper : public NoCopyable{
  static int CreateNonblockingOrDie(sa_family_t family);
  static int Connect(int sockfd, const struct sockaddr* addr);
  static void BindOrDie(int sockfd, const struct sockaddr* addr);
  static void ListenOrDie(int sockfd);
  static int Accept(int sockfd, struct sockaddr_in6* addr);
  static ssize_t Read(int sockfd, void* buf, size_t count);
  static ssize_t Readv(int sockfd, const struct iovec* iov, int iovcnt);
  static ssize_t Write(int sockfd, const void* buf, size_t count);
  static void Close(int sockfd);
  static void ShutdownWrite(int sockfd);
  static void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
  static void ToIp(char* buf, size_t size, const struct sockaddr* addr);
  static void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
  static void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
  static int GetSocketError(int sockfd);
  static const struct sockaddr* Cast_sockaddr(const struct sockaddr_in* addr);
  static const struct sockaddr_in* Cast_sockaddr_in(const struct sockaddr* addr);
  static bool isSelfConnect(int sockfd);
};


#endif //VAINGLORY_REACTOR_SOCKET_HELPER_H
