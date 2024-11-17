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
    explicit Socket(int socketFd);
    ~Socket();
    int Fd() const;
    int GetTcpInfo(struct tcp_info* tcpi) const;
    std::string GetTcpInfoString() const;

    void BindAddress(const INetAddress& addr) const;
    void Listen() const;
    int Accept(INetAddress* peeraddr) const;
    void ShutdownWrite() const;

    // Set sockfd operations
    void SetTcpNoDelay(bool on) const; // Nagle's algorithm
    void SetReuseAddr(bool on) const;
    void SetReusePort(bool on) const;
    void SetKeepAlive(bool on) const;

  private:
    int sockfd_;
};


#endif //VAINGLORY_REACTOR_SOCKET_H
