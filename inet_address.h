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
  explicit INetAddress(uint16_t port, bool loopbackOnly = false);
  explicit INetAddress(const sockaddr_in* addr);
  explicit INetAddress(const INetAddress&);
  INetAddress& operator=(const INetAddress&);
  std::string Ip() const;
  std::string IpPort() const;
  uint16_t Port() const;
  const sockaddr* GetSockAddr() const;
private:
  sockaddr_in addr_;
};


#endif//VAINGLORY_REACTOR_INET_ADDRESS_H
