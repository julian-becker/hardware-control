#pragma once
#include <memory>

template <typename T> struct
inotifyable {
    void handle(T&& val);
    
private:
    virtual void handle_impl(T&&) = 0;
};


template <typename T>
void inotifyable<T>::handle(T&& val) {
    handle_impl(std::move(val));
}
