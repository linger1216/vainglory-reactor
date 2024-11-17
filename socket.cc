//
// Created by Jingwei on 2024-11-16.
//

#include "socket.h"
#include "socket_helper.h"
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "inet_address.h"
Socket::Socket(int socketFd): sockfd_(socketFd) {
}

Socket::~Socket() {
  close(sockfd_);
}

int Socket::Fd() const {
  return sockfd_;
}

int Socket::GetTcpInfo(struct tcp_info* tcpi) const {
  socklen_t len = sizeof(*tcpi);
  bzero(tcpi, len);
  return getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len);
}

std::string Socket::GetTcpInfoString() const {
  struct tcp_info tcpi;
  if (0 == GetTcpInfo(&tcpi)) {
    char buf[128];
    bzero(buf, sizeof buf);
    snprintf(buf, sizeof buf, "unrecovered=%u "
                       "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                       "lost=%u retrans=%u rtt=%u rttvar=%u "
                       "sshthresh=%u cwnd=&=%u total_retrans=%u",
             tcpi.tcpi_retransmits,
             tcpi.tcpi_rto, tcpi.tcpi_ato,
             tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost, tcpi.tcpi_retrans,
             tcpi.tcpi_rtt, tcpi.tcpi_rttvar,
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);
    return buf;
  }
  return "";
}

void Socket::BindAddress(const INetAddress& addr) const {
  SocketHelper::Bind(sockfd_, addr.GetSockAddr());
}

void Socket::Listen() const {
  SocketHelper::Listen(sockfd_);
}

/*
 * 连接成功，将客户端的地址信息设置到 peerAddr 中
 */
int Socket::Accept(INetAddress* peerAddr) const {
  struct sockaddr_in6 addr;
  std::memset(&addr, 0, sizeof addr);
  int clientFd = SocketHelper::Accept(sockfd_, &addr);
  if (clientFd >= 0) {
    peerAddr->SetSockAddrInet6(addr);
  }
  return clientFd;
}

void Socket::ShutdownWrite() const {
  SocketHelper::ShutdownWrite(sockfd_);
}

void Socket::SetTcpNoDelay(bool on) const {
  SocketHelper::SetTcpNoDelay(sockfd_, on);
}

void Socket::SetReuseAddr(bool on) const {
  SocketHelper::SetReuseAddr(sockfd_, on);
}

void Socket::SetReusePort(bool on) const {
  SocketHelper::SetReusePort(sockfd_, on);
}

void Socket::SetKeepAlive(bool on) const {
  SocketHelper::SetKeepAlive(sockfd_, on);
}
