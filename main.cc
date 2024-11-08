
#include "log.h"
#include "time_stamp.h"
#include "inet_address.h"

int main() {
  TimeStamp now = TimeStamp(1731062271000000);
  Debug("%s", now.ToString().c_str());

  INetAddress addr(80, "127.0.0.1");
  Info("ip:%s", addr.Ip().c_str());
  Warn("port:%d", addr.Port());
  Debug("ip port:%s", addr.IpPort().c_str());
  return 0;
};
