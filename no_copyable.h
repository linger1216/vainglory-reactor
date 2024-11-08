//
// Created by Jingwei on 2024-11-07.
//

#ifndef VAINGLORY_REACTOR_NO_COPYABLE_H
#define VAINGLORY_REACTOR_NO_COPYABLE_H

class NoCopyable {
protected:
    NoCopyable() = default;
    ~NoCopyable() = default;
private:
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable& operator=(const NoCopyable&) = delete;
};

#endif//VAINGLORY_REACTOR_NO_COPYABLE_H
