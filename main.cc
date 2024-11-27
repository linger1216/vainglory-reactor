
#include "tcp_server.h"
#include "tcp_connection.h"
#include "log.h"
#include "buffer.h"


void connectionCallback(const TcpConnection* conn){
  Debug("APP connectionCallback %p", conn);
}

void destroyCallback(const TcpConnection* conn){
  Debug("APP destroyCallback %p", conn);
}

void writeCallback(const TcpConnection* conn){
  Debug("APP writeCallback %p", conn);
}

void messageCallback(const TcpConnection* conn, Buffer* buffer, int n){
  char* msg = new char[n+1];
  buffer->Read(msg, n);
  msg[n] = '\0';
  Debug("recv msg:%s", msg);
  delete []msg;

  // why peer cannot receive the message
  buffer->Append("hello");
}

int main() {
  TcpServer server(8888, 1,
                   connectionCallback,
                   destroyCallback,
                   messageCallback,
                   writeCallback);
  server.Run();
  return 0;
};
