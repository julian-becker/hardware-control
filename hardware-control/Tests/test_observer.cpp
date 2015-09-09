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
#include <tuple>
#include <functional>
#include <TDD/inotifyable.h>
#include <TDD/with_destructor.h>
#include <TDD/iobservable.h>
#include <TDD/ilistener.h>
#include <TDD/observable.h>




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

template <typename TaskId> struct
ischeduler : with_destructor<TaskId> {
    template <typename Callable>
    void schedule(TaskId task_id, Callable&& fun) {
        schedule_impl(std::move(task_id), std::forward<Callable>(fun));
    }

    void unschedule(TaskId task_id) {
        unschedule_impl(std::move(task_id));
    }
    
private:
    virtual void schedule_impl(TaskId task_id, std::function<void()> f) = 0;
    virtual void unschedule_impl(TaskId task_id) = 0;
};

template <typename T> class
observable_del_dispatch : public iobservable<T> {
    std::set<ilistener<T>*> listeners;
    ischeduler<ilistener<T>*>* const scheduler;

    void registerListener_impl(ilistener<T>& l) override {
        listeners.insert(&l);
        l.add_raii(this,[this,&l]{
            this->unregisterListener(l);
            scheduler->unschedule(&l);
        });
    }

    void unregisterListener_impl(ilistener<T>& l) override {
        listeners.erase(&l);
    }
    
public:
    observable_del_dispatch(ischeduler<ilistener<T>*>& s)
        : scheduler(&s)
    {
        INFO("observable_del_dispatch: constructor");
    }
    
    ~observable_del_dispatch() {
        INFO("observable_del_dispatch: destructor");
        for(auto& l : listeners) {
            l->remove_raii(this);
        }
    }
    
    
    observable_del_dispatch& operator = (const T& val) {
        for(auto& lstnr : listeners)
            scheduler->schedule(lstnr,[val,lstnr]{
                lstnr->handle(T(val));
            });
        return *this;
    }
};

TEST_CASE("observable with delegated dispatch","[observable_del_dispatch]") {
    struct test_scheduler : ischeduler<ilistener<int>*> {
        size_t called_schedule = 0u, called_unschedule = 0u;
        std::queue< std::tuple<ilistener<int>*,std::function<void()>>> tasks_scheduled;
        std::queue<ilistener<int>*> tasks_unscheduled;
        
    private:
        void schedule_impl(ilistener<int>* task_id, std::function<void()> f) override {
            ++called_schedule;
            tasks_scheduled.emplace(std::move(task_id),std::move(f));
        }
        void unschedule_impl(ilistener<int>* task_id) override {
            ++called_unschedule;
            tasks_unscheduled.emplace(std::move(task_id));
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
                REQUIRE(!scheduler.called_schedule);
            }
        }
        
        AND_WHEN("the observable is triggered after a listener is registered") {
            obs.registerListener(lstnr1);
            obs = 14;
            THEN("work is delegated to the scheduler") {
                REQUIRE(scheduler.called_schedule == 1);
            }
            AND_THEN("the listener is called with the value assigned if the scheduled task is executed") {
                REQUIRE(!lstnr1.triggered);
                auto task = scheduler.tasks_scheduled.front();
                REQUIRE(std::get<0>(task) == &lstnr1);
                std::get<1>(task)(); // execute the task
                REQUIRE(lstnr1.triggered);
                scheduler.tasks_scheduled.pop();
            }
        }
        
        AND_WHEN("the observable is triggered after multiple listeners are registered") {
            obs.registerListener(lstnr1);
            obs.registerListener(lstnr2);
            obs.registerListener(lstnr3);
            obs = 14;
            THEN("work is delegated to the scheduler in the form of several tasks") {
                REQUIRE(scheduler.called_schedule == 3);
            }
            AND_THEN("the listener is called with the value assigned if the scheduled task is executed") {
                REQUIRE(!lstnr1.triggered);
                REQUIRE(!lstnr2.triggered);
                REQUIRE(!lstnr3.triggered);
                while(scheduler.tasks_scheduled.size()) {
                    std::get<1>(scheduler.tasks_scheduled.front())();
                    scheduler.tasks_scheduled.pop();
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
                REQUIRE(scheduler.tasks_scheduled.size() == 0);
            }
        }
    }
}

TEST_CASE("observable_del_dispatch task lifetime","[observable_del_dispatch]") {
    struct test_scheduler : ischeduler<ilistener<int>*> {
        size_t called_schedule = 0u, called_unschedule = 0u;
        std::queue< std::tuple<ilistener<int>*,std::function<void()>>> tasks_scheduled;
        std::queue<ilistener<int>*> tasks_unscheduled;
        
    private:
        void schedule_impl(ilistener<int>* task_id, std::function<void()> f) override {
            ++called_schedule;
            tasks_scheduled.emplace(std::move(task_id),std::move(f));
        }
        void unschedule_impl(ilistener<int>* task_id) override {
            ++called_unschedule;
            tasks_unscheduled.emplace(std::move(task_id));
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
            THEN("a task is scheduled") {
                REQUIRE(scheduler.tasks_scheduled.size() == 1);
            }
            listener = nullptr;
            AND_THEN("the task is scheduled when the listener is destroyed") {
                REQUIRE(scheduler.tasks_unscheduled.size() == 1);
            }
        }
    }
}
