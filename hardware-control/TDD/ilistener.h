#pragma once
#include <TDD/with_destructor.h>

template <typename T> struct
iobservable;


template <typename T> struct
ilistener
    : inotifyable<T>,
      with_destructor<iobservable<T>*>
{
};