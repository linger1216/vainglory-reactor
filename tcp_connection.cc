//
// Created by Jingwei on 2024-11-03.
//

#include "tcp_connection.h"

#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "log.h"
#include <utility>


TcpConnection::TcpConnection(int fd, EventLoop* eventLoop,
                             ConnectionCallback connectionCallback,
                             CloseCallback closeCallback,
                             MessageCallback messageCallback,
                             WriteCompleteCallback writeCompleteCallback)
    : _eventLoop(eventLoop),
      connectionCallback_(std::move(connectionCallback)),
      closeCallback_(std::move(closeCallback)),
      messageCallback_(std::move(messageCallback)),
      writeCompleteCallback_(std::move(writeCompleteCallback)) {

  _name = "TcpConnection-" + std::to_string(fd);
  _readBuf = new Buffer(BUFFER_SIZE);
  _writeBuf = new Buffer(BUFFER_SIZE);

  auto readHandler = std::bind(&TcpConnection::readHandler, this, std::placeholders::_1);
  auto writeHandler = std::bind(&TcpConnection::writeHandler, this, std::placeholders::_1);
  auto destroyHandler = std::bind(&TcpConnection::destroyHandler, this, std::placeholders::_1);

  _channel = new Channel(fd, FDEvent::ReadEvent,
                         readHandler,
                         writeHandler,
                         nullptr,
                         destroyHandler,
                         this);

  _eventLoop->AddTask(_channel, EventLoopOperator::Add);
}

TcpConnection::~TcpConnection() {
  delete _readBuf;
  delete _writeBuf;
  _eventLoop->FreeChannel(_channel);
  delete _channel;
  Debug("连接断开, 释放资源, connName: %s", _name.c_str());
}

int TcpConnection::readHandler(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  int count = conn->_readBuf->ReadSocket(conn->_channel->Fd());
  if (count > 0) {
    Debug("read %d bytes\n", count);
  } else {
    // 没有读到数据，关闭连接
    Debug("read 0 bytes, means connection is down, so we close connection\n");
    conn->_eventLoop->AddTask(conn->_channel, EventLoopOperator::Delete);
  }
  return 0;
}

int TcpConnection::writeHandler(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  int count = conn->_writeBuf->SendSocket(conn->_channel->Fd());
  if (count > 0) {
    if (conn->_writeBuf->GetWriteableSize() == 0) {
      Debug("write %d bytes\n", count);
      conn->_channel->EnableWriting();
    }
  } else {
    // 没有写到数据，关闭连接
    Debug("write 0 bytes, means connection is down, so we close connection\n");
    conn->_eventLoop->AddTask(conn->_channel, EventLoopOperator::Delete);
  }
  return 0;
}

int TcpConnection::destroyHandler(void* arg) {
  auto conn = static_cast<TcpConnection*>(arg);
  delete conn;
  return 0;
}
int TcpConnection::closeHandler(void* arg) {
  return 0;
}
