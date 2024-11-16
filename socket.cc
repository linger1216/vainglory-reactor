//
// Created by Jingwei on 2024-11-16.
//

#include "socket.h"
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

Socket::Socket(int sockfd): sockfd_(sockfd) {
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

std::string Socket::GetTcpInfoString() {
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

void Socket::BindAddress(const INetAddress& addr) {

}

void Socket::Listen() {
}

int Socket::Accept(INetAddress* peeraddr) {
  return 0;
}

void Socket::ShutdownWrite() {
}

void Socket::SetTcpNoDelay(bool on) {
}

void Socket::SetReuseAddr(bool on) {
}

void Socket::SetReusePort(bool on) {
}

void Socket::SetKeepAlive(bool on) {
}
