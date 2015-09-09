#pragma once

template <typename T> struct
ilistener;

template <typename T> struct
iobservable {
    void registerListener(ilistener<T>& l);

    void unregisterListener(ilistener<T>& l);
    
private:
    virtual void registerListener_impl(ilistener<T>& l) = 0;

    virtual void unregisterListener_impl(ilistener<T>& l) = 0;
};



template <typename T>
void iobservable<T>::registerListener(ilistener<T>& l) {
    registerListener_impl(l);
}

template <typename T>
void iobservable<T>::unregisterListener(ilistener<T>& l) {
    unregisterListener_impl(l);
}