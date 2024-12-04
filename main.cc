
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
  auto c = const_cast<TcpConnection*>(conn);

  auto size = 10 * 1024 * 1024;
  char content[size];
  std::memset(content, 'A', sizeof(content));
  content[size - 1] = '\0';
  c->Send(content, size);

//  c->Send("hello", 5);
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
