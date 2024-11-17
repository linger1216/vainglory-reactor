//
// Created by Jingwei on 2024-11-08.
//

#ifndef VAINGLORY_REACTOR_INET_ADDRESS_H
#define VAINGLORY_REACTOR_INET_ADDRESS_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cinttypes>
#include <string>

class INetAddress {
public:
  explicit INetAddress(uint16_t port, bool loopbackOnly = false, bool ipv6 = false);
  explicit INetAddress(const struct sockaddr_in& addr);
  explicit INetAddress(const struct sockaddr_in6& addr);
  std::string Ip() const;
  std::string IpPort() const;
  uint16_t Port() const;
  const sockaddr* GetSockAddr() const;
  void SetSockAddrInet6(const struct sockaddr_in6& addr6);
private:
  union {
    sockaddr_in addr_;
    sockaddr_in6 addr6_;
  };
};


#endif//VAINGLORY_REACTOR_INET_ADDRESS_H
