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
#include <queue>
#include <functional>

template <typename T> struct
inotifyable {
    void handle(T&& val) {
        handle_impl(std::move(val));
    }
private:
    virtual void handle_impl(T&&) = 0;
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
            THEN("the functor is called when the with_destructor-instance is destroyed") {
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
                THEN("the functor is not called when the with_destructor-instance is destroyed") {
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
    void registerListener(ilistener<T>& l) {
        registerListener_impl(l);
    }

    void unregisterListener(ilistener<T>& l) {
        unregisterListener_impl(l);
    }
    
private:
    virtual void registerListener_impl(ilistener<T>& l) = 0;

    virtual void unregisterListener_impl(ilistener<T>& l) = 0;
};


template <typename T> struct
ilistener : inotifyable<T>, with_destructor<iobservable<T>*> {
};



template <typename T>
class observable : public iobservable<T> {
    T value;
    std::set<ilistener<T>*> listeners;

    void registerListener_impl(ilistener<T>& l) override {
        listeners.insert(&l);
        l.add_raii(this,[this,&l]{ this->unregisterListener(l); });
    }

    void unregisterListener_impl(ilistener<T>& l) override {
        listeners.erase(&l);
    }
    
public:
    explicit observable(T value)
    : value(value)
    {
    }
    
    observable(const observable& other)
    : value(other.value), listeners(other.listeners)
    {
        for(auto& l : listeners)
            l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
    }

    observable(observable&& other)
    : value(std::move(other.value)), listeners(std::move(other.listeners))
    {
        for(auto& l : listeners) {
            l->remove_raii(&other);
            l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
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
            l->add_raii(this,[this,&l]{ this->unregisterListener(*l); });
        }
        return *this;
    }

    ~observable() {
        for(auto& l : listeners)
            l->remove_raii(this);
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
    
    virtual void handle_impl(T&& arg) override {
        behavior(std::move(arg));
    }

public:
    template <typename LAMBDA>
    listener(LAMBDA lam)
      : behavior(std::forward<LAMBDA>(lam))
    {
    }
    
    listener(const listener& other) = delete;
    
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
        private:
            void handle_impl(int&& i) override {
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


TEST_CASE("Observable copy does not result in segfault", "[observable]") {
    GIVEN("an observable and a listener registered at the observable") {
        struct test_listener final : ilistener<int> {
            int value = 0;
            bool triggered = false;
        private:
            void handle_impl(int&& i) override {
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
        private:
            void handle_impl(int&& i) override {
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
        private:
            void handle_impl(int&& i) override {
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
        private:
            void handle_impl(int&& i) override {
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

TEST_CASE("moving an observable into another observable keeps the listeners intact","[observable][listener]") {
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
        
        concrete(observable<T1>& o, std::function<T(T1)> fun)
        : val(fun(static_cast<const T1&>(o))),
          behavior(std::move(fun))
        {
            o.registerListener(*this);
        }
        
    private:
        void handle_impl(T1&& valIn) override {
            val = behavior(std::move(valIn));
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

struct
ischeduler {
    template <typename Callable>
    void schedule(Callable&& fun) {
        schedule_impl(std::forward<Callable>(fun));
    }
    
private:
    virtual void schedule_impl(std::function<void()> f) = 0;
};

template <typename T> class
observable_del_dispatch : public iobservable<T> {
    std::set<ilistener<T>*> listeners;
    ischeduler* const scheduler = nullptr;

    void registerListener_impl(ilistener<T>& l) override {
        listeners.insert(&l);
    }

    void unregisterListener_impl(ilistener<T>& l) override {
        listeners.erase(&l);
    }
    
public:
    observable_del_dispatch(ischeduler& s)
        : scheduler(&s)
    {
    }
    
    
    observable_del_dispatch& operator = (const T& val) {
        for(auto& lstnr : listeners)
            scheduler->schedule([val,lstnr]{
                lstnr->handle(T(val));
            });
        return *this;
    }
};

TEST_CASE("observable with delegated dispatch","[observable_del_dispatch]") {
    struct test_scheduler : ischeduler {
        size_t called = 0u;
        std::queue<std::function<void()>> tasks;
    private:
        void schedule_impl(std::function<void()> f) override {
            ++called;
            tasks.emplace(std::move(f));
        }
    } scheduler;
    
    struct test_listener : ilistener<int> {
        bool triggered = false;
    private:
        void handle_impl(int&& val) {
            triggered = true;
        }
    } lstnr1, lstnr2, lstnr3;
    
    GIVEN("an observable with delegated dispatch") {
        observable_del_dispatch<int> obs(scheduler);
        WHEN("the observable is triggered") {
            obs = 14;
            THEN("no work is delegated to the scheduler if no listeners are registered at the observable") {
                REQUIRE(!scheduler.called);
            }
        }
        
        AND_WHEN("the observable is triggered after a listener is registered") {
            obs.registerListener(lstnr1);
            obs = 14;
            THEN("work is delegated to the scheduler") {
                REQUIRE(scheduler.called == 1);
            }
            AND_THEN("the listener is called with the value assigned if the scheduled task is executed") {
                REQUIRE(!lstnr1.triggered);
                scheduler.tasks.front()();
                REQUIRE(lstnr1.triggered);
                scheduler.tasks.pop();
            }
        }
        
        AND_WHEN("the observable is triggered after multiple listeners are registered") {
            obs.registerListener(lstnr1);
            obs.registerListener(lstnr2);
            obs.registerListener(lstnr3);
            obs = 14;
            THEN("work is delegated to the scheduler in the form of several tasks") {
                REQUIRE(scheduler.called == 3);
            }
            AND_THEN("the listener is called with the value assigned if the scheduled task is executed") {
                REQUIRE(!lstnr1.triggered);
                REQUIRE(!lstnr2.triggered);
                REQUIRE(!lstnr3.triggered);
                while(scheduler.tasks.size()) {
                    scheduler.tasks.front()();
                    scheduler.tasks.pop();
                }
                REQUIRE(lstnr1.triggered);
                REQUIRE(lstnr2.triggered);
                REQUIRE(lstnr3.triggered);
            }
        }
        
        AND_WHEN("the observable is triggered after a listener has registered and unregistered afterwards") {
            obs.registerListener(lstnr1);
            obs.registerListener(lstnr2);
            obs.registerListener(lstnr3);
            obs.unregisterListener(lstnr1);
            obs.unregisterListener(lstnr2);
            obs.unregisterListener(lstnr3);
            obs = 14;

            THEN("the listener is not called and no task is scheduled") {
                REQUIRE(!lstnr1.triggered);
                REQUIRE(!lstnr2.triggered);
                REQUIRE(!lstnr3.triggered);
                REQUIRE(scheduler.tasks.size() == 0);
            }
        }
    }
}

TEST_CASE("observable_del_dispatch task lifetime","[observable_del_dispatch]") {
    struct test_scheduler : ischeduler {
        size_t called = 0u;
        std::queue<std::function<void()>> tasks;
    private:
        void schedule_impl(std::function<void()> f) override {
            ++called;
            tasks.emplace(std::move(f));
        }
    } scheduler;
    
    struct test_listener : ilistener<int> {
        bool triggered = false;
    private:
        void handle_impl(int&& val) {
            triggered = true;
        }
    };
    
    GIVEN("an observable and a listener that is registered for the observable") {
        auto observable = std::make_shared<observable_del_dispatch<int>>(scheduler);
        auto listener = std::make_shared<test_listener>();
        observable->registerListener(*listener);
        WHEN("the observable is changed and the listener destroyed") {
            *observable = 42;
            listener = nullptr;
            THEN("no task is scheduled") {
                REQUIRE(scheduler.tasks.size() == 0);
            }
        }
    }
}
