cmake_minimum_required(VERSION 3.22.1)
project(vainglory-reactor)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DDEBUG)

add_executable(vainglory_reactor main.cc
        current_thread.h
        log.h
        no_copyable.h
        event_loop.cc
        event_loop.h
        thread.cc
        thread.h
        time_stamp.cc
        time_stamp.h
        inet_address.cc
        inet_address.h
        tcp_server.cc
        tcp_server.h
        channel.cc
        channel.h
        fd_event.h
        epoll_dispatcher.cc
        epoll_dispatcher.h)