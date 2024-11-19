
#include "tcp_server.h"
#include "tcp_connection.h"
#include "log.h"
#include "buffer.h"

void connectionCallback(const TcpConnection* conn){
  Debug("messageCallback");
}

void messageCallback(const TcpConnection* conn, Buffer* buffer, int n){
  char* msg = new char[n+1];
  buffer->Read(msg, n);
  msg[n] = '\0';
  Debug("recv msg:%s", msg);
  delete []msg;

  buffer->Append("hello");
}

void writeCallback(const TcpConnection* conn){
  Debug("messageCallback xxx");
}

int main() {
  TcpServer server(8888, 1, connectionCallback,messageCallback, writeCallback);
  server.Run();
  return 0;
};
