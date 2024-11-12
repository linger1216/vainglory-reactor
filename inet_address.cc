//
// Created by Jingwei on 2024-11-08.
//

#include "inet_address.h"
#include <cstring>
INetAddress::INetAddress(uint16_t port, const std::string &ip) {
  // ipv4
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  if (ip.empty()) {
    addr_.sin_addr.s_addr = INADDR_ANY;
  } else {
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  }
}
INetAddress::INetAddress(const sockaddr_in &addr):addr_(addr) {
}

std::string INetAddress::Ip() const {
  char buf [64];
  memset(buf, 0, sizeof(buf));
  // 网络字节序转本地字节序
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  return buf;
}

std::string INetAddress::IpPort() const {
  char buf [64];
  memset(buf, 0, sizeof(buf));
  // 网络字节序转本地字节序
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  snprintf(buf, sizeof(buf), "%s:%u", buf, ntohs(addr_.sin_port));
  return buf;
}

uint16_t INetAddress::Port() const {
  return ntohs(addr_.sin_port);
}

const sockaddr_in *INetAddress::GetSockAddr() const {
  return &addr_;
}