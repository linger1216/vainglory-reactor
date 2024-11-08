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
  explicit INetAddress(uint16_t port, const std::string& ip = "127.0.0.1");
  explicit INetAddress(const struct sockaddr_in& addr);
  std::string Ip() const;
  // Ip:Port
  std::string IpPort() const;
  uint16_t Port() const;
  const sockaddr_in* GetSockAddr() const;
private:
  sockaddr_in addr_;
};


#endif//VAINGLORY_REACTOR_INET_ADDRESS_H
