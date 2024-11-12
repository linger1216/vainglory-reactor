//
// Created by Jingwei on 2024-11-09.
//

#ifndef VAINGLORY_REACTOR_FD_EVENT_H
#define VAINGLORY_REACTOR_FD_EVENT_H

/*
 * 定义一些抽象的事件类型
 * 用以适配epoll/kqueue/select/poll等事件
 */

enum class FDEvent {
  None = 0x00,
  ErrorEvent = 0x01,
  ReadEvent = 0x01 << 1,
  WriteEvent = 0x01 << 2,
  CloseEvent = 0x01 << 3
};

inline const char* FDEventToString(FDEvent event) {
  switch (event) {
    case FDEvent::None:
      return "None";
    case FDEvent::ErrorEvent:
      return "ErrorEvent";
    case FDEvent::ReadEvent:
      return "ReadEvent";
    case FDEvent::WriteEvent:
      return "WriteEvent";
    case FDEvent::CloseEvent:
      return "CloseEvent";
    default:
      return "UnknownEvent";
  }
}


#endif //VAINGLORY_REACTOR_FD_EVENT_H
