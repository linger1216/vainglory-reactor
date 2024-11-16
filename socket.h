//
// Created by Jingwei on 2024-11-16.
//

#ifndef VAINGLORY_REACTOR_SOCKET_H
#define VAINGLORY_REACTOR_SOCKET_H

#include "no_copyable.h"
#include <string>
struct tcp_info;
class INetAddress;

class Socket : public NoCopyable{
  public:
    explicit Socket(int sockfd);
    ~Socket();
    int Fd() const;
    int GetTcpInfo(struct tcp_info* tcpi) const;
    std::string GetTcpInfoString();

    void BindAddress(const INetAddress& addr);
    void Listen();
    int Accept(INetAddress* peeraddr);
    void ShutdownWrite();

    // Set sockfd operations
    void SetTcpNoDelay(bool on); // Nagle's algorithm
    void SetReuseAddr(bool on);
    void SetReusePort(bool on);
    void SetKeepAlive(bool on);

  private:
    int sockfd_;
};


#endif //VAINGLORY_REACTOR_SOCKET_H
