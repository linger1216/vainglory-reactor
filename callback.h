//
// Created by Jingwei on 2024-11-16.
//

#ifndef VAINGLORY_REACTOR_CALLBACK_H
#define VAINGLORY_REACTOR_CALLBACK_H

#include <functional>
class TcpConnection;
class Buffer;
class INetAddress;

using AcceptCallback =  std::function<void(int fd, const INetAddress*)>;
using ConnectionCallback = std::function<void (const TcpConnection* conn)>;
using MessageCallback = std::function<void (const TcpConnection* conn, Buffer*, int n)>;
using HighWaterMarkCallback = std::function<void (const TcpConnection* conn, std::size_t size)>;

#endif //VAINGLORY_REACTOR_CALLBACK_H
