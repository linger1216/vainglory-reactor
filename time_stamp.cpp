//
// Created by Jingwei on 2024-11-08.
//

#include "time_stamp.h"
#include <sys/time.h>

TimeStamp::TimeStamp() : microSeconds_(0) {}
TimeStamp::~TimeStamp() {}
TimeStamp::TimeStamp(int64_t microSeconds) : microSeconds_(microSeconds) {
}
TimeStamp TimeStamp::Now() {
  timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}
std::string TimeStamp::ToString() const {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(microSeconds_ / kMicroSecondsPerSecond);
  tm tm_time;
  gmtime_r(&seconds, &tm_time);
  int microseconds = static_cast<int>(microSeconds_ % kMicroSecondsPerSecond);
  snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  return buf;
}
