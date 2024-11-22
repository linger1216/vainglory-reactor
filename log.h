// -*- mode: c++ -*-

// logy v1.2 -- A simplistic, light-weight, single-header C++ logger
// (!) Summer 2018 by Giovanni Squillero <giovanni.squillero@polito.it>
// This code has been dedicated to the public domain
// Project page: https://github.com/squillero/logy

#pragma once

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

#define FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

static std::mutex g_logy_mutex;

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


static inline const char* tid() {
  auto tid = std::this_thread::get_id();
  std::stringstream ss;
  ss << tid;
  return ss.str().c_str();
}

static inline void logy_header(const char* tag) {
  char timestamp[100] = "";
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count();

  {
    std::lock_guard<std::mutex> guard(g_logy_mutex);
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S", &now_tm);
    std::snprintf(timestamp + std::strlen(timestamp), sizeof(timestamp) - std::strlen(timestamp), ".%03ld]", now_ms);
    std::fprintf(stderr, "%s[%s] ", timestamp, tag);
  }
}

template<typename... T>
static inline void logy_body(const char* file, int line, const char* func, T... args) {
  {
    std::lock_guard<std::mutex> guard(g_logy_mutex);
    //  std::fprintf(stderr, "[%s:%d] [%s] ", file, line, func); 带函数名

    std::fprintf(stderr, "[%s:%d] [%s] ", file, line, tid()); // 带线程id
    //    std::fprintf(stderr, "[%s:%d] ", file, line);
    std::fprintf(stderr, args...);
    std::fprintf(stderr, "\n");
    std::fflush(stderr);
  }
}

template<typename... T>
void _Debug(const char* file, int line, const char* func, T... args) {
  logy_header("DEBUG");
  logy_body(file, line, func, args...);
}

template<typename... T>
void _Info(const char* file, int line, const char* func, T... args) {
  logy_header("INFO");
  logy_body(file, line, func, args...);
}

template<typename... T>
void _Warning(const char* file, int line, const char* func, T... args) {
  logy_header("WARNING");
  logy_body(file, line, func, args...);
}

template<typename... T>
void _Error(const char* file, int line, const char* func, T... args) {
  logy_header("ERROR");
  logy_body(file, line, func, args...);
  std::exit(0);
}


// redundant, strictly speaking, but avoids the unaesthetic format-string-is-not-a-literal warning
static void _Debug(const char* file, int line, const char* func, const char* arg) {
  logy_header("DEBUG");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Info(const char* file, int line, const char* func, const char* arg) {
  logy_header("INFO");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Warning(const char* file, int line, const char* func, const char* arg) {
  logy_header("WARNING");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
}
static void _Error(const char* file, int line, const char* func, const char* arg) {
  logy_header("ERROR");
  std::fprintf(stderr, "%s\n", arg);
  std::fflush(stderr);
  std::exit(0);
}

#if defined(DEBUG) || defined(LOGGING_DEBUG)

#define Debug(...) _Debug(FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define Info(...) _Info(FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define Warn(...) _Warning(FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define Error(...) _Error(FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#else
#define Debug(...) ((void) 0)
#define Info(...) ((void) 0)
#define Warning(...) ((void) 0)
#endif
