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
#include <list>
#include <map>
#include <functional>

template <typename T> struct
inotifyable {
    virtual void handle(T&&) = 0;
};

template <typename _Kty> struct
with_destructor {
    std::map<_Kty,std::function<void()>> functors;
public:
    template <typename Callable>
    void add_raii(_Kty key, Callable fun) {
        functors[std::move(key)] = std::move(fun);
    }

    void remove_raii(_Kty key) {
        functors.erase(std::move(key));
    }
    
    virtual ~with_destructor() {
        for(auto& f : functors)
            f.second();
    }
};

template <typename T> struct
ilistener;

template <typename T> struct
iobservable;


template <typename T> struct
ilistener : inotifyable<T>, with_destructor<iobservable<T>*> {
};

template <typename T> struct
iobservable {
};


template <typename T>
class listener;



template <typename T>
class observable : public iobservable<T> {
    T value;
    std::set<ilistener<T>*> listeners;
    
public:
    observable(T value)
    : value(value)
    {
    }
    
    ~observable() {
        for(auto& l : listeners)
            l->remove_raii(this);
    }

    void registerListener(ilistener<T>& l) {
        listeners.insert(&l);
        l.add_raii(this,[this,&l]{ unregisterListener(l); });
    }

    void unregisterListener(ilistener<T>& l) {
        listeners.erase(&l);
    }
    
    observable& operator=(const T& val) {
        value = val;
        for(auto& lstnr : listeners)
            lstnr->handle(T(val));
        return *this;
    }
    
    operator const T& () const {
        return value;
    }

};




template <typename T>
class listener final : public ilistener<T> {
    std::function<void(T&&)> behavior;
    
public:
    template <typename LAMBDA>
    listener(LAMBDA lam)
      : behavior(std::forward<LAMBDA>(lam))
    {
    }
    
    void handle(T&& arg) override {
        behavior(std::move(arg));
    }
    
    ~listener() {
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


TEST_CASE("Assigning value triggers all listeners", "[observable][listener]") {
    GIVEN("an observable<int> and a several listener<int>") {
        observable<int> obs(0);
        bool triggered1 = false, triggered2 = false, triggered3 = false;
        listener<int> lstnr1([&triggered1](int&&){ triggered1 = true; });
        listener<int> lstnr2([&triggered2](int&&){ triggered2 = true; });
        listener<int> lstnr3([&triggered3](int&&){ triggered3 = true; });
        WHEN("several listeners registers at observable and a value is put into the observable") {
            obs.registerListener(lstnr1);
            obs.registerListener(lstnr2);
            obs.registerListener(lstnr3);
            obs = 42;
            THEN("all registered listeners are triggered") {
                REQUIRE(triggered1);
                REQUIRE(triggered2);
                REQUIRE(triggered3);
            }
        }
    }
}


TEST_CASE("Unregister listener", "[observable][listener]") {
    GIVEN("an observable<int> and a listener<int>") {
        observable<int> obs(0);
        bool triggered = false;
        listener<int> lstnr([&triggered](int&&){ triggered = true; });
        WHEN("listener registers and unregisteres at the observable and a value is put into the observable") {
            obs.registerListener(lstnr);
            obs.unregisterListener(lstnr);
            obs = 42;
            THEN("the listener is not triggered") {
                REQUIRE(!triggered);
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