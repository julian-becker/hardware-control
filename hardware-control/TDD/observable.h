#pragma once
#include <TDD/iobservable.h>
#include <TDD/ilistener.h>
#include <set>


template <typename T>
class observable : public iobservable<T> {
    T value;
    std::set<ilistener<T>*> listeners;

    void registerListener_impl(ilistener<T>& l) override;

    void unregisterListener_impl(ilistener<T>& l) override;
    
public:
    explicit observable(T value);
    
    observable(const observable& other);

    observable(observable&& other);
    
    observable& operator = (const observable& other);
    
    observable& operator = (observable&& other);

    ~observable();

    observable& operator=(const T& val);
    
    operator const T& () const;

};


template <typename T>
void observable<T>::registerListener_impl(ilistener<T>& l) {
    listeners.insert(&l);
    l.add_raii(this,[this,&l]{ this->unregisterListener(l); });
}

template <typename T>
void observable<T>::unregisterListener_impl(ilistener<T>& l) {
    listeners.erase(&l);
}

template <typename T>
observable<T>::observable(T value)
: value(value)
{
}

template <typename T>
observable<T>::observable(const observable& other)
: value(other.value), listeners(other.listeners)
{
    for(auto& l : listeners)
        l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
}

template <typename T>
observable<T>::observable(observable&& other)
: value(std::move(other.value)), listeners(std::move(other.listeners))
{
    for(auto& l : listeners) {
        l->remove_raii(&other);
        l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
    }
}

template <typename T>
observable<T>& observable<T>::operator = (const observable& other) {
    if(this != &other) {
        observable copy(other);
        *this = std::move(copy);
    }
    return *this;
}

template <typename T>
observable<T>& observable<T>::operator = (observable&& other) {
    listeners = std::move(other.listeners);
    value = std::move(value);
    for(auto& l : listeners) {
        l->remove_raii(&other);
        l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
    }
    return *this;
}

template <typename T>
observable<T>::~observable() {
    for(auto& l : listeners)
        l->remove_raii(this);
}

template <typename T>
observable<T>& observable<T>::operator=(const T& val) {
    value = val;
    for(auto& lstnr : listeners)
        lstnr->handle(T(val));
    return *this;
}

template <typename T>
observable<T>::operator const T& () const {
    return value;
}
