#pragma once
#include <map>
#include <functional>
#include <memory>

template <typename _Kty> struct
with_destructor {
    std::map<_Kty,std::function<void()>> functors;
    
public:
    template <typename Callable>
    void add_raii(_Kty key, Callable fun);

    void remove_raii(_Kty key);
    
    virtual ~with_destructor();
};


template <typename _Kty>
template <typename Callable>
void with_destructor<_Kty>::add_raii(_Kty key, Callable fun) {
    functors[std::move(key)] = std::move(fun);
}

template <typename _Kty>
void with_destructor<_Kty>::remove_raii(_Kty key) {
    functors.erase(std::move(key));
}

template <typename _Kty>
with_destructor<_Kty>::~with_destructor() {
    while(functors.size()) {
        const auto front = functors.begin();
        front->second();
        functors.erase(front->first);
    }
}