//
// Created by Jingwei on 2024-11-14.
//

#include "tcp_client.h"

#include <utility>

TcpClient::TcpClient(EventLoop* loop,
                     const INetAddress& addr,
                     const std::string& name)
    : loop_(loop), addr_(addr), name_(name) {
}
TcpClient::~TcpClient() {
}
void TcpClient::Connect() {
}
void TcpClient::Disconnect() {
}
void TcpClient::Stop() {
}
void TcpClient::SetConnectionCallback(const ConnectionCallback& cb) {
  connectionCallback_ = cb;
}
void TcpClient::SetMessageCallback(const MessageCallback& cb) {
  messageCallback_ = cb;
}
void TcpClient::SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
  writeCompleteCallback_ = cb;
}
