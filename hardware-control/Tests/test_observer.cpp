//
//  test_observer.cpp
//  hardware-control
//
//  Created by Julian Becker on 03.09.15.
//  Copyright (c) 2015 Julian Becker. All rights reserved.
//
#include "../catch.h"
#include <stdio.h>
#include <memory>


template <typename T> struct
inotifyable {
    virtual void handle(T&&) = 0;
};

template <typename T> struct
inotify_dead {
    virtual void notify_dead(T*) = 0;
};

template <typename T> struct
ilistener;

template <typename T> struct
iobservable;


template <typename T> struct
ilistener : inotifyable<T>, inotify_dead<iobservable<T>> {
    virtual void set_observer(iobservable<T>*) = 0;
};

template <typename T> struct
iobservable : inotify_dead<ilistener<T>> {
};


template <typename T>
class listener;



template <typename T>
class observable : public iobservable<T> {
    T value;
    ilistener<T>* lstnr = nullptr;
    
public:
    observable(T value)
    : value(value)
    {
    }
    
    ~observable() {
        if(lstnr)
            lstnr->notify_dead(this);
    }

    void registerListener(ilistener<T>& l) {
        l.set_observer(this);
        lstnr = &l;
    }
    
    observable& operator=(const T& val) {
        value = val;
        if(lstnr)
            lstnr->handle(T(val));
        return *this;
    }
    
    operator const T& () const {
        return value;
    }
    
    void notify_dead(ilistener<T>* l) override {
        if(l == lstnr)
            lstnr = 0;
    }

};




template <typename T>
class listener final : public ilistener<T> {
    std::function<void(T&&)> behavior;
    iobservable<T>* obs;
    friend class observable<T>;
    
public:
    template <typename LAMBDA>
    listener(LAMBDA lam)
      : behavior(std::forward<LAMBDA>(lam)),
        obs(nullptr)
    {
    }
    
    void handle(T&& arg) override {
        behavior(std::move(arg));
    }
    
    void notify_dead(iobservable<T>* o) override {
        if(o == obs)
            obs = nullptr;
    }
    
    void set_observer(iobservable<T>* o) override {
        obs = o;
    }
    
    ~listener() {
        if(obs)
            obs->notify_dead(this);
    }
};



TEST_CASE( "Create an observable", "[observable]" ) {
    WHEN("constructing an observable<int>") {
        THEN("no exception is thrown") {
            REQUIRE_NOTHROW(observable<int>(0));
        }
    }
    
    WHEN("consturcting an observable<T> with T throwing in its constructor") {
        struct t_throws_in_constructor {
            t_throws_in_constructor() { throw nullptr; }
        };
        THEN("the constructor of observable<T> will also throw") {
            REQUIRE_THROWS_AS(t_throws_in_constructor(), std::nullptr_t);
        }
    }
}






TEST_CASE( "Create a listener", "[listener]" ) {
    WHEN("constructing an listener<int>") {
        THEN("no exception is thrown") {
            REQUIRE_NOTHROW(listener<int>([](int&&){}));
        }
    }
}

TEST_CASE("Register listener at observable", "[observable][listener]") {
    GIVEN("a listener and an observable") {
        observable<int> obs(0);
        listener<int>   lst([](int&&){});
        WHEN("registering the listener at the observable") {
            THEN("no exception is thrown") {
                REQUIRE_NOTHROW(obs.registerListener(lst));
            }
        }
    }
}

TEST_CASE("Assign value to observable","[observable]") {
    GIVEN("We may assign an value to an observable") {
        observable<int> obs(0);
        obs = 1;
    }

    WHEN("an observable is constructed") {
        observable<int> obs(0x1337);
        THEN("the value it holds is the value given the constructor") {
            REQUIRE(0x1337 == obs);
        }
    }
    
    WHEN("a value is assigned to an observable") {
        observable<int> obs(0);
        obs = 42;
        THEN("the same value can be retrieved back from the observable") {
            REQUIRE(42 == obs);
        }
    }
}

TEST_CASE("Assigning value triggers observer", "[observable][listener]") {
    GIVEN("an observable<int> and a listener<int>") {
        observable<int> obs(0);
        bool triggered = false;
        listener<int> lstnr([&triggered](int&&){ triggered = true; });
        WHEN("listener registers at observable and a value is put into the observable") {
            obs.registerListener(lstnr);
            obs = 42;
            THEN("the listener is triggered") {
                REQUIRE(triggered);
            }
        }
    }
}

TEST_CASE("destroying listener before observable must be safe", "[observable][listener]") {
    GIVEN("an observable<int> and a listener<int>") {
        auto obs = std::make_shared<observable<int>>(0);
        bool triggered = false;
        auto lstnr = std::make_shared<listener<int>>([&triggered](int&&){ triggered = true; });
        
        WHEN("the listener is destroyed and the observable triggered") {
            obs->registerListener(*lstnr);
            lstnr=0;
            *obs = 37;
            THEN("the program does not crash and the registered listener is still not triggered") {
                REQUIRE(!triggered);
            }
        }
    }
}


TEST_CASE("destroying observable before listener must be safe", "[observable][listener]") {
    GIVEN("an observable<int> and a listener<int>") {
        auto obs = std::make_shared<observable<int>>(0);
        auto lstnr = std::make_shared<listener<int>>([](int&&){});
        
        WHEN("the observable is destroyed the registered listener can be safely destructed") {
            obs->registerListener(*lstnr);
            obs = nullptr;
            
            int* x;
            
            THEN("the program does not crash and the registered listener is still not triggered") {
                lstnr = nullptr;
            }
        }
    }
}