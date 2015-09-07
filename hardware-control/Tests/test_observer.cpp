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
#include <string>
#include <sstream>
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


TEST_CASE("with_destructor::add_raii","[with_destructor]") {
    GIVEN("an instance of with_destructor") {
        auto wd = std::make_shared<with_destructor<int>>();
        
        WHEN("A functor is added with add_raii(functor)") {
            bool called = false;
            std::function<void()> fun = [&called]{ called = true; };
            wd->add_raii(0, fun);
            THEN("the functor is called when the instance is destroyed") {
                REQUIRE(!called);
                wd = nullptr;
                REQUIRE(called);
            }
        }
    }
}


TEST_CASE("with_destructor::remove_raii","[with_destructor]") {
    GIVEN("an instance of with_destructor") {
        auto wd = std::make_shared<with_destructor<int>>();
        
        WHEN("A functor is added with add_raii(functor)") {
            bool called = false;
            std::function<void()> fun = [&called]{ called = true; };
            wd->add_raii(123, fun);
            AND_WHEN("remove_raii() is called with the key given in the add_raii()-call") {
                wd->remove_raii(123);
                THEN("the functor is not called when the instance is destroyed") {
                    REQUIRE(!called);
                    wd = nullptr;
                    REQUIRE(!called);
                }
            }
        }
    }
}


template <typename T> struct
ilistener;

template <typename T> struct
iobservable {
    virtual void registerListener(ilistener<T>& l) = 0;

    virtual void unregisterListener(ilistener<T>& l) = 0;
};


template <typename T> struct
ilistener : inotifyable<T>, with_destructor<iobservable<T>*> {
};



template <typename T>
class observable : public iobservable<T> {
    T value;
    std::set<ilistener<T>*> listeners;
    
public:
    explicit observable(T value)
    : value(value)
    {
    }
    
    observable(const observable& other)
    : value(other.value), listeners(other.listeners)
    {
        for(auto& l : listeners)
            l->add_raii(this,[this,&l]{ unregisterListener(*l); });
    }

    observable(observable&& other)
    : value(std::move(other.value)), listeners(std::move(other.listeners))
    {
        for(auto& l : listeners) {
            l->remove_raii(&other);
            l->add_raii(this,[this,&l]{ unregisterListener(*l); });
        }
    }
    
    observable& operator = (const observable& other) {
        if(this != &other) {
            observable copy(other);
            *this = std::move(copy);
        }
        return *this;
    }
    
    observable& operator = (observable&& other) {
        listeners = std::move(other.listeners);
        value = std::move(value);
        for(auto& l : listeners) {
            l->remove_raii(&other);
            l->add_raii(this,[this,&l]{ unregisterListener(*l); });
        }
        return *this;
    }

    ~observable() {
        for(auto& l : listeners)
            l->remove_raii(this);
    }

    void registerListener(ilistener<T>& l) override {
        listeners.insert(&l);
        l.add_raii(this,[this,&l]{ unregisterListener(l); });
    }

    void unregisterListener(ilistener<T>& l) override {
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
    
    listener(const listener& other) = delete;
    
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


TEST_CASE("Observable is copyable", "[observable]") {
    observable<int> obs(0);
    observable<int> obsCopy(obs);
}


TEST_CASE("Observable copy behaves exactly like the original", "[observable]") {
    GIVEN("an observable and a listener registered at the observable") {
        struct test_listener final : ilistener<int> {
            int value = 0;
            bool triggered = false;
            
            void handle(int&& i) {
                value = i;
                triggered = true;
            }
        } lst;
    
        observable<int> obs(0);
        obs.registerListener(lst);
        
        THEN("creating a copy of the observabe and modifying the value of the copy will trigger all listeners that have been registered at the initial observable") {
            observable<int> obsCopy(obs);
            obsCopy = 42;
            REQUIRE(lst.triggered);
        }
    }
}


TEST_CASE("Observable copy does not result in segfault", "[observable][listener]") {
    GIVEN("an observable and a listener registered at the observable") {
        struct test_listener final : ilistener<int> {
            int value = 0;
            bool triggered = false;
            
            void handle(int&& i) {
                value = i;
                triggered = true;
            }
        };
        
        auto obs = std::make_shared<observable<int>>(0);
        bool triggered = false;
        auto lst = std::make_shared<test_listener>();
        obs->registerListener(*lst);
        
        THEN("creating a copy of the observabe and destroying the original as well as the listener will not result in a segfault") {
            observable<int> obsCopy(*obs);
            obs = nullptr;
            lst = nullptr;
        }

        AND_THEN("modifying the copied observable after the original has been destroyed still triggers the listener") {
            observable<int> obsCopy(*obs);
            obs = nullptr;
            obsCopy = 42;
            REQUIRE(lst->triggered);
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

TEST_CASE("Assigning value triggers observer", "[observable]") {
    GIVEN("an observable<int> and a listener<int>") {
        struct test_listener final : ilistener<int> {
            int value = 0;
            bool triggered = false;
            
            void handle(int&& i) {
                value = i;
                triggered = true;
            }
        } lstnr;
        
        observable<int> obs(0);
        WHEN("listener registers at observable and a value is put into the observable") {
            obs.registerListener(lstnr);
            obs = 42;
            THEN("the listener is triggered") {
                REQUIRE(lstnr.triggered);
            }
        }
    }
}


TEST_CASE("Assigning value triggers all listeners", "[observable]") {
    GIVEN("an observable<int> and a several listener<int>") {
        struct test_listener final : ilistener<int> {
            int value = 0;
            bool triggered = false;
            
            void handle(int&& i) {
                value = i;
                triggered = true;
            }
        } lstnr1, lstnr2, lstnr3;
        observable<int> obs(0);
        WHEN("several listeners registers at observable and a value is put into the observable") {
            obs.registerListener(lstnr1);
            obs.registerListener(lstnr2);
            obs.registerListener(lstnr3);
            obs = 42;
            THEN("all registered listeners are triggered") {
                REQUIRE(lstnr1.triggered);
                REQUIRE(lstnr2.triggered);
                REQUIRE(lstnr3.triggered);
            }
        }
    }
}


TEST_CASE("Unregister listener", "[observable]") {
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


TEST_CASE("destroying listener before observable must be safe", "[observable]") {
    GIVEN("an observable<int> and a listener<int>") {
        struct test_listener final : ilistener<int> {
            bool& triggered;
            test_listener(bool& triggered) : triggered(triggered) {}
            void handle(int&& i) {
                triggered = true;
            }
        };
    
        auto obs = std::make_shared<observable<int>>(0);
        bool triggered = false;
        auto lstnr = std::make_shared<test_listener>(triggered);
        
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

TEST_CASE("moving an observable into another observable keeps the listeners intact") {
    GIVEN("an observable and a listener") {
        observable<int> obs(0);
        bool triggered = false;
        auto lstnr = std::make_shared<listener<int>>([&triggered](int&&){ triggered = true; });
        WHEN("the listener is registered it the observable") {
            obs.registerListener(*lstnr);
            AND_WHEN("the observable is move-constructed into a different observable") {
                observable<int> obs2(std::move(obs));
                THEN("modifying the moved-to observable will trigger the listener") {
                    obs2 = 42;
                    REQUIRE(triggered);
                }
            }
            AND_WHEN("the observable is moved into a different observable") {
                observable<int> obs2(0);
                obs2 = std::move(obs);
                THEN("modifying the moved-to observable will trigger the listener") {
                    obs2 = 42;
                    REQUIRE(triggered);
                }
            }
        }
    }
}


template <typename T> class
dependent_value {
   
    struct holder {
        virtual ~holder() {}
        virtual const T& get_value() const = 0;
    };
    
    template <typename T1> struct
    concrete final : holder, ilistener<T1> {
        T val;
        std::function<T(T1)> behavior;

        const T& get_value() const override {
            return val;
        }
        
        void handle(T1&& valIn) {
            val = behavior(std::move(valIn));
        }
        
        concrete(observable<T1>& o, std::function<T(T1)> fun)
        : val(fun(static_cast<const T1&>(o))),
          behavior(std::move(fun))
        {
            o.registerListener(*this);
        }
    };
    
    std::shared_ptr<holder> impl;
    
public:
    template <typename T1, typename Callable>
    dependent_value(observable<T1>& obsIn, Callable fun)
        : impl(new concrete<T1>{obsIn,fun})
    {
    }
    
    operator const T& () const {
        return impl->get_value();
    }
};

TEST_CASE("dependent_value","[dependent_value]") {
    observable<int> obs(1);
    dependent_value<std::string> value(obs,[](int&& i){
        return (std::stringstream() << i).str();
    });
    
    WHEN("the value of the dependent value is retrieved") {
        THEN("the retrieved value it is equal to the behavior applied to the initial value of the observable") {
            REQUIRE(std::string("1") == static_cast<const std::string&>(value));
        }
    }
    
    AND_WHEN("assigning the observable a value") {
        obs = 7;
        THEN("the dependent value will be updated") {
            REQUIRE(std::string("7") == static_cast<const std::string&>(value));
        }
    }
}