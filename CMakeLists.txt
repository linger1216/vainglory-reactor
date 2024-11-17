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
        channel.cc
        channel.h
        fd_event.h
        epoll_dispatcher.cc
        epoll_dispatcher.h
        dispatcher.h
        op_event.h
        work_thread.h
        work_thread.cc
        thread_pool.h
        thread_pool.cc
        tcp_connection.h
        tcp_connection.cc
        tcp_server.h
        tcp_server.cc
        buffer.h
        buffer.cc
        tcp_client.cc
        tcp_client.h
        callback.h
        socket.cc
        socket.h
        socket_helper.cc
        socket_helper.h
        network_encode.h
)