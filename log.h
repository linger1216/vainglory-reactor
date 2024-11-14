// -*- mode: c++ -*-

// logy v1.2 -- A simplistic, light-weight, single-header C++ logger
// (!) Summer 2018 by Giovanni Squillero <giovanni.squillero@polito.it>
// This code has been dedicated to the public domain
// Project page: https://github.com/squillero/logy

#pragma once

#include <chrono>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>
#include <iomanip>

#define FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

// vectors & initializer_list are supported using sfinae

template<typename T>
struct is_rangeloop_supported {
  static const bool value = false;
};
template<typename T>
struct is_rangeloop_supported<std::vector<T>> {
  static const bool value = true;
};
template<typename T>
struct is_rangeloop_supported<std::initializer_list<T>> {
  static const bool value = true;
};

template<typename T>
static inline typename std::enable_if<is_rangeloop_supported<T>::value, std::string>::type tag_expand(T arg);
template<typename T>
static inline typename std::enable_if<!is_rangeloop_supported<T>::value, std::string>::type tag_expand(T arg);

template<typename T>
static inline typename std::enable_if<is_rangeloop_supported<T>::value, std::string>::type tag_expand(T arg) {
  std::ostringstream ss;
  ss << "[";
  for (const auto e: arg)
    ss << " " << tag_expand(e);
  ss << " ]";
  return ss.str();
}

template<typename T>
static inline typename std::enable_if<!is_rangeloop_supported<T>::value, std::string>::type tag_expand(T arg) {
  std::ostringstream ss;
  ss << arg;
  return ss.str();
}

/**
 * Alt take using simple tag dispatch (requires C++14)
 *
 * template<typename T> static inline std::string tag_expand(T arg);
 *
 * template<typename T>
 * static inline std::string tag_expand(T arg, std::true_type) {
 *     std::ostringstream ss;
 *     ss << "[";
 *     for(const auto e : arg)
 *         ss << " " << tag_expand(e);
 *     ss << " ]";
 *     return ss.str();
 * }
 *
 * template<typename T>
 * static inline std::string tag_expand(T arg, std::false_type) {
 *     std::ostringstream ss;
 *     ss << arg;
 *     return ss.str();
 * }
 *
 * template<typename T>
 * static inline std::string tag_expand(T arg) {
 *     return tag_expand(arg, std::conditional_t<is_rangeloop_supported<T>::value, std::true_type, std::false_type>{});  // c++14
 * }
 **/

// helper functions

// 原来的 logy_header
//static inline void logy_header(const char* tag) {
//    char timestamp[100] = "";
//    std::time_t t = std::time(nullptr);
//    std::strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", std::localtime(&t));
//    std::fprintf(stderr, "%s%s", timestamp, tag);
//}

static inline void logy_header(const char* tag) {
  char timestamp[100] = "";
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count();

  // 格式化时间戳，包括毫秒
  strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &now_tm);
  std::snprintf(timestamp + std::strlen(timestamp), sizeof(timestamp) - std::strlen(timestamp), ".%03ld]", now_ms);

//  std::string pos = std::string(__FILE__);
  // 输出到stderr
  std::fprintf(stderr, "%s[%s] ", timestamp, tag);
}

static inline void logy_helper() {
  std::cerr << std::endl;
}

template<typename F, typename... R>
static inline void logy_helper(F first, R&&... rest) {
  std::cerr << " " << tag_expand(first);
  logy_helper(std::forward<R>(rest)...);
}

// good old printf syntax

template<typename... T>
void _Debug(T... args) {
  logy_header("DEBUG");
  std::fprintf(stderr, args...);
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
}

template<typename... T>
void _Info(T... args) {
  logy_header("INFO");
  std::fprintf(stderr, args...);
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
}

template<typename... T>
void _Warning(T... args) {
  logy_header("WARNING");
  std::fprintf(stderr, args...);
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
}

template<typename... T>
void _Error(T... args) {
  logy_header("ERROR");
  std::fprintf(stderr, args...);
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
  std::exit(0);
}

template<typename... T>
void _Silent(T... args) {
  logy_header(" ");
  std::fprintf(stderr, args...);
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
}

// redundant, strictly speaking, but avoids the unaesthetic format-string-is-not-a-literal warning
static void _Debug(const char* arg) {
  logy_header("DEBUG");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Info(const char* arg) {
  logy_header("INFO");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Warning(const char* arg) {
  logy_header("WARNING");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Error(const char* arg) {
  logy_header("ERROR");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
  std::exit(0);
}
static void _Silent(const char* arg) {
  logy_header(" ");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}

// "just print it" syntax

template<typename... T>
static inline void _Debug2(T... args) {
  logy_header(" DEBUG");
  logy_helper(std::forward<T>(args)...);
}

template<typename... T>
static inline void _Info2(T... args) {
  logy_header(" INFO");
  logy_helper(std::forward<T>(args)...);
}

template<typename... T>
static inline void _Warning2(T... args) {
  logy_header(" WARNING");
  logy_helper(std::forward<T>(args)...);
}

template<typename... T>
static inline void _Error2(T... args) {
  logy_header(" ERROR");
  logy_helper(std::forward<T>(args)...);
  std::exit(0);
}

template<typename... T>
static inline void _Silent2(T... args) {
  logy_header("");
  logy_helper(std::forward<T>(args)...);
}

#if defined(DEBUG) || defined(LOGGING_DEBUG)

#define Debug(...) _Debug(__VA_ARGS__)
#define Info(...) _Info(__VA_ARGS__)
#define Warn(...) _Warning(__VA_ARGS__)
#define Error(...) _Error(__VA_ARGS__)
#define LOG_DEBUG(...) _Debug2(__VA_ARGS__)
#define LOG_INFO(...) _Info2(__VA_ARGS__)
#define LOG_WARN(...) _Warning2(__VA_ARGS__)
#define LOG_ERROR(...) _Error2(__VA_ARGS__)

#else

#define Debug(...) ((void) 0)
#define Info(...) ((void) 0)
#define Warning(...) ((void) 0)
#define LOG_DEBUG(...) ((void) 0)
#define LOG_INFO(...) ((void) 0)
#define LOG_WARNING(...) ((void) 0)

#endif

#define Logy(...) _Silent(__VA_ARGS__)
#define LOGY(...) _Silent2(__VA_ARGS__)
