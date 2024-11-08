#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#if DEBUG
/*
*  如果不加 do ... while(0) 在进行条件判断的时候(只有一句话), 省略了{}, 就会出现语法错误
*  if
*     xxxxx
*  else
*     xxxxx
*  宏被替换之后, 在 else 前面会出现一个 ;  --> 语法错误
*/
#define LOG(type, fmt, args...)                                                                                \
  do {                                                                                                         \
    char timeStr[32];                                                                                          \
    struct timeval tv = {0, 0};                                                                                       \
    struct tm *tmInfo;                                                                                         \
    long milliseconds;                                                                                         \
    gettimeofday(&tv, NULL);                                                                                   \
    tmInfo = localtime(&tv.tv_sec);                                                                            \
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tmInfo);                                           \
    milliseconds = tv.tv_usec / 1000;                                                                          \
    printf("%s.%03ld [%s]: %s@%s, line: %d ", timeStr, milliseconds, type, FILE_NAME, __FUNCTION__, __LINE__); \
    printf(fmt, ##args);                                                                                       \
    printf("\n");                                                                                              \
  } while (0)
#define Debug(fmt, args...) LOG("DEBUG", fmt, ##args)
#define Info(fmt, args...) LOG("INFO", fmt, ##args)
#define Warn(fmt, args...) LOG("WARN", fmt, ##args)
#define Error(fmt, args...)    \
  do {                         \
    LOG("ERROR", fmt, ##args); \
    exit(0);                   \
  } while (0)
#else
#define LOG(fmt, args...)
#define Debug(fmt, args...)
#define Error(fmt, args...)
#endif
