//
// Created by Jingwei on 2024-11-07.
//

#ifndef VAINGLORY_REACTOR_CURRENT_THREAD_H
#define VAINGLORY_REACTOR_CURRENT_THREAD_H

#include <unistd.h>
#include <thread>

namespace current_thread {

  inline int pid(){
    return getpid();
  }

  inline std::thread::id tid(){
    return std::this_thread::get_id();
  }
}


#endif//VAINGLORY_REACTOR_CURRENT_THREAD_H
