//
// Created by Jingwei on 2024-11-08.
//

#include "inet_address.h"
#include "network_encode.h"
#include "socket_helper.h"
#include <cstring>
INetAddress::INetAddress(uint16_t port, bool loopbackOnly) {
  std::memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  auto ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
  addr_.sin_port = hostToNetwork16(port);
}

INetAddress::INetAddress(const sockaddr_in* addr):addr_(*addr) {
}
INetAddress::INetAddress(const INetAddress& r) : addr_(r.addr_){
}
INetAddress& INetAddress::operator=(const INetAddress& r) {
  this->addr_ = r.addr_;
  return *this;
}


std::string INetAddress::Ip() const {
  char buf[64];
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

const sockaddr* INetAddress::GetSockAddr() const {
  return SocketHelper::to_sockaddr(&addr_);
}
