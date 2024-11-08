
#include "log.h"
#include "time_stamp.h"

int main() {
  TimeStamp now = TimeStamp(1731062271000000);
  Debug("%s", now.ToString().c_str());
  return 0;
};
