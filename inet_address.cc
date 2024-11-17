//
// Created by Jingwei on 2024-11-08.
//

#include "inet_address.h"
#include <cstring>
#include "network_encode.h"
#include "socket_helper.h"
INetAddress::INetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
  if (ipv6) {
    std::memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    auto ip = loopbackOnly? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = hostToNetwork16(port);
  } else {
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    auto ip = loopbackOnly? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_port = hostToNetwork16(port);
  }
}

INetAddress::INetAddress(const sockaddr_in &addr):addr_(addr) {
}

INetAddress::INetAddress(const sockaddr_in6& addr):addr6_(addr) {
}

std::string INetAddress::Ip() const {
  char buf [64];
  std::memset(buf, 0, sizeof(buf));
  SocketHelper::ToIp(buf, sizeof(buf), GetSockAddr());
  return buf;
}

std::string INetAddress::IpPort() const {
  char buf[64];
  std::memset(buf, 0, sizeof(buf));
  SocketHelper::ToIpPort(buf, sizeof(buf), GetSockAddr());
  return buf;
}

uint16_t INetAddress::Port() const {
  return networkToHost16(addr_.sin_port);
}

const sockaddr *INetAddress::GetSockAddr() const {
  return SocketHelper::to_sockaddr(&addr6_);
}
void INetAddress::SetSockAddrInet6(const sockaddr_in6& addr6) {
  addr6_ = addr6;
}
