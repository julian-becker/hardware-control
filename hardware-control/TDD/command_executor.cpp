//
//  command_executor.cpp
//  hardware-control
//
//  Created by Julian Becker on 11.05.16.
//  Copyright © 2016 Julian Becker. All rights reserved.
//

#include "command_executor.hpp"
#include <catch.h>
#include <typeindex>
#include <mutex>
#include <map>
#include <future>
#include <iostream>
#include <cxxabi.h>

namespace {

std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

template <class T>
std::string demangle() {

    return demangle(typeid(T).name());
}
}

template <int...>
struct Seq {};

template <int n, int... s>
struct Gens : Gens<n-1, n-1, s...> {};

template <int... s>
struct Gens<0, s...> {
  typedef Seq<s...> type;
};


/// We want a metafunction to accept an index N into our type list LIST
template <unsigned N, typename LIST> struct
tuple_element;

/// Specialization for the case where N==0
template <template <typename ...> class LIST_T,typename T,typename...ELMS> struct
tuple_element<0,LIST_T<T,ELMS...>> {
    using type = T; // just take the first type from the list
};

template <unsigned N, template <typename ...> class LIST_T,typename T,typename...ELMS> struct
tuple_element<N,LIST_T<T,ELMS...>> {
    /// fallback for N>0: delegate the result type recursively until N->0
    using type = typename tuple_element<N-1,LIST_T<ELMS...>>::type;
};

// create a convenience wrapper for the template
template <unsigned N, typename LIST> using
type_at = typename tuple_element<N, LIST>::type;


template <typename T1, typename T2, typename...Ts>
auto lock(T1& t1, T2& t2, Ts&... ts) {
    std::lock(t1, t2, ts...);
    return std::shared_ptr<void>(nullptr,[&](void*){
        t1.unlock(), t2.unlock();
        int unlockRest[]{(ts.unlock(),1)...};
    });
}

template <typename T1>
auto lock(T1& t1) {
    t1.lock();
    return std::shared_ptr<void>(nullptr,[&](void*){ t1.unlock(); });
}



class LockableDevice {
    mutable std::mutex mutex;
    std::shared_ptr<void> const device;
    std::string name;
public:
    template <typename Device>
    LockableDevice(std::unique_ptr<Device> device)
        : device(std::move(device))
        , name(demangle<Device>())
    {
    }
    
    void lock() const {
        mutex.lock();
        std::cout << "LOCKED:   " << name << std::endl;
    }
    
    void unlock() const {
        mutex.unlock();
        std::cout << "UNLOCKED: " << name << std::endl;
    }
    
    bool try_lock() const {
        auto res = mutex.try_lock();
        std::cout << "TRYLOCK:  " << name << " -> " << (res? "success" : "failure") << std::endl;
        return res;
    }
    
    template <typename Device>
    Device* access () {
        return static_cast<Device*>(device.get());
    }
};


template <typename...Devices> struct requires {
    static constexpr size_t size = sizeof...(Devices);
};

template <typename> using MapToLockableDevice = LockableDevice;

template<class F>
auto shared_function( F&& f ) {
  auto pf = std::make_shared<std::decay_t<F>>(std::forward<F>(f));
  return [pf](auto&&... args){
    return (*pf)(decltype(args)(args)...);
  };
}



class ResourceRegistry {
    std::mutex mutex;
    std::map<std::type_index, std::unique_ptr<LockableDevice>> deviceMap;
    
public:
    template <typename...Ts>
    auto acquire(requires<Ts...>) {
        std::lock_guard<std::mutex> selfProtect(mutex);
        auto lockScope = lock(*deviceMap.at(std::type_index(typeid(Ts)))...);
        return std::make_pair(
            std::move(lockScope),
            std::make_tuple(
                deviceMap.at(std::type_index(typeid(Ts)))->access<Ts>()...
            )
        );
    }
    
    auto acquire(requires<>) {
        return std::make_pair(std::shared_ptr<void>(),std::tuple<>());
    }
    
    template <typename T>
    void registerDevice(std::unique_ptr<T> device) {
        std::lock_guard<std::mutex> selfProtect(mutex);
        auto lockDev = std::make_unique<LockableDevice>(std::move(device));
        deviceMap.emplace(std::make_pair(std::type_index(typeid(T)), std::move(lockDev)));
    }
};

struct Executor {
    void execute(std::function<void()> task) {
        task();
    }
};


class ResourceOrchestrator {
    std::unique_ptr<ResourceRegistry> resourceRegistry;
    std::shared_ptr<Executor> executor;
    
    template <typename Cmd, typename...Ts>
    static auto result_type_helper(Cmd&& cmd,requires<Ts...>) -> decltype(cmd(std::declval<std::add_lvalue_reference_t<Ts>>()...));
    
    template <typename Command> struct
    result_of {
        using type = decltype(result_type_helper(std::declval<Command>(), typename std::decay_t<Command>::requirements()));
    };
    template <typename T> using result_of_t = typename result_of<T>::type;

    template <typename...T>
    struct task_wrapper {
        
    };
    
public:
    ResourceOrchestrator(std::unique_ptr<ResourceRegistry> resourceRegistry, std::shared_ptr<Executor> executor)
        : resourceRegistry(std::move(resourceRegistry))
        , executor(std::move(executor))
    {
    }
    

    template<typename Result, typename Command, typename Devices, int ...S>
    std::enable_if_t<std::is_same<Result,void>::value, void>
    executeWithDevices(std::promise<Result> promise, Command&& cmd, std::shared_ptr<void> lockScope, Devices&& devs, Seq<S...>) {
        std::function<void(std::promise<Result>& prom, Command& cmd, type_at<S,Devices>&...devices)> unboundTask =
            [](std::promise<Result>& prom, Command& cmd, type_at<S,Devices>&...devices) {
                try {
                    cmd(*devices...);
                    prom.set_value();
                } catch ( ... ) {
                    prom.set_exception(std::current_exception());
                }
            };
        std::function<void()> task = shared_function(std::bind(unboundTask, std::move(promise), std::forward<Command>(cmd), std::get<S>(devs)...));
        executor->execute(task);
    }
    
    template<typename Result, typename Command, typename Devices, int ...S>
    std::enable_if_t<!std::is_same<Result,void>::value, void>
    executeWithDevices(std::promise<Result> promise, Command&& cmd, std::shared_ptr<void> lockScope, Devices&& devs, Seq<S...>) {
        std::function<void(std::promise<Result>& prom, Command& cmd, type_at<S,Devices>&...devices)> unboundTask =
            [](std::promise<Result>& prom, Command& cmd, type_at<S,Devices>&...devices) {
                try {
                    auto result = cmd(*devices...);
                    prom.set_value(std::move(result));
                } catch ( ... ) {
                    prom.set_exception(std::current_exception());
                }
            };
        std::function<void()> task = shared_function(std::bind(unboundTask, std::move(promise), std::forward<Command>(cmd), std::get<S>(devs)...));
        executor->execute(task);
    }
    
    template <typename Command, typename Result = result_of_t<Command>>
    auto execute(Command&& cmd) -> Result {
        using requiredResources = typename std::decay_t<Command>::requirements;
        auto devices = resourceRegistry->acquire(requiredResources());
        std::promise<Result> promise;
        auto future = promise.get_future();
        executeWithDevices(std::move(promise), std::forward<Command>(cmd), std::move(devices.first), std::move(devices.second), typename Gens<requiredResources::size>::type());
        return future.get();
    }
};


TEST_CASE("ResourceOrchestrator", "[executor]") {
    GIVEN("A ResourceOrchestrator and a ResourceRegistry") {
        struct Device1{
            void operation1() { }
        };
        struct Device2{
            void operation2() { }
        };
        auto registry = std::make_unique<ResourceRegistry>();
        auto executor = std::make_shared<Executor>();
        auto dev1 = std::make_unique<Device1>();
        auto dev2 = std::make_unique<Device2>();
        
        registry->registerDevice(std::move(dev1));
        registry->registerDevice(std::move(dev2));
        ResourceOrchestrator orchestrator(std::move(registry), executor);
        THEN("It can execute a command") {
            bool executed = false;
            struct command {
                using requirements = requires<>;
                std::reference_wrapper<bool> executed;
                void operator() () {
                    bool& b(executed);
                    b = true;
                }
            } cmd { std::ref(executed) };
            orchestrator.execute(cmd);
            REQUIRE(executed);
        }
        
        THEN("It can execute a command with ressource requirements") {
            bool executed = false;
            struct command {
                using requirements = requires<Device1, Device2>;
                
                std::reference_wrapper<bool> executedRef;
                void operator() (Device1& d1, Device2& d2) {
                    bool& executed(executedRef);
                    std::cout << "executing............." << std::endl;
                    d1.operation1();
                    d2.operation2();
                    executed = true;
                }
            } cmd { std::ref(executed) };
            orchestrator.execute(cmd);
            REQUIRE(executed);
        }
    }
}
