//
// Created by Jingwei on 2024-11-08.
//

#ifndef VAINGLORY_REACTOR_TIME_STAMP_H
#define VAINGLORY_REACTOR_TIME_STAMP_H

#include <inttypes.h>
#include <string>

class TimeStamp {
public:
  TimeStamp();
  ~TimeStamp();
  explicit TimeStamp(int64_t microSeconds);
  std::string ToString() const;
  static TimeStamp Now();
  static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
  int64_t microSeconds_;
};


#endif//VAINGLORY_REACTOR_TIME_STAMP_H
