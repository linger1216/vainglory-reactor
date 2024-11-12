#pragma once
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#define FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define LOG_ARGS(args, ...) args##__VA_ARGS__

#if DEBUG
#define LOG(type, fmt, args...)                                                                                      \
  do {                                                                                                               \
    std::stringstream ss;                                                                                            \
    auto now = std::chrono::system_clock::now();                                                                     \
    auto now_c = std::chrono::system_clock::to_time_t(now);                                                          \
    std::tm now_tm = *std::localtime(&now_c);                                                                        \
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;              \
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count(); \
    ss << " [" << type << "] " << std::string(FILE_NAME) << "@" << __FUNCTION__ << ", line:" << __LINE__ << " ";     \
    ss << fmt;                                                                                                       \
    printf(ss.str().c_str(), ##args);                                                                                \
    printf("\n");                                                                                                    \
  } while (0)
#define Debug(fmt, args...) LOG("DEBUG", fmt, ##args)
#define Info(fmt, args...) LOG("INFO", fmt, ##args)
#define Warn(fmt, args...) LOG("WARN", fmt, ##args)
#define Error(fmt, args...)    \
  do {                         \
    LOG("ERROR", fmt, ##args); \
    std::exit(0);              \
  } while (0)
#else
#define LOG(fmt, args...)
#define Debug(fmt, args...)
#define Info(fmt, args...)
#define Warn(fmt, args...)
#define Error(fmt, args...)
#endif