#pragma once
#include <TDD/ilistener.h>
#include <functional>

template <typename T>
class listener final : public ilistener<T> {
    std::function<void(T&&)> behavior;
    
    virtual void handle_impl(T&& arg) override;

public:
    template <typename LAMBDA>
    listener(LAMBDA lam);
    
    listener(const listener& other) = delete;
    
    ~listener();
};


template <typename T>
void listener<T>::handle_impl(T&& arg) {
    behavior(std::move(arg));
}

template <typename T>
template <typename LAMBDA>
listener<T>::listener(LAMBDA lam)
  : behavior(std::forward<LAMBDA>(lam))
{
}


template <typename T>
listener<T>::~listener() {
}