
#include "epoll_dispatcher.h"
#include "inet_address.h"
#include "log.h"
#include "time_stamp.h"

int main() {
  TimeStamp xx(1731062271000000);
  Debug("%s", xx.ToString().c_str());

  INetAddress addr(80, "127.0.0.1");
  Info("ip:%s", addr.Ip().c_str());
  Warn("port:%d", addr.Port());
  Debug("ip port:%s", addr.IpPort().c_str());

  EpollDispatcher dispatcher(nullptr);
  return 0;
};
