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
type_at_t = typename tuple_element<N, LIST>::type;


class LockableDevice {
    mutable std::mutex mutex;
    std::shared_ptr<void> const device;
    std::type_index id;
public:
    template <typename Device>
    LockableDevice(std::unique_ptr<Device> device)
        : device(std::move(device))
        , id(std::type_index(typeid(Device)))
    {
    }
    
    void lock() const {
        mutex.lock();
    }
    
    void unlock() const {
        mutex.unlock();
    }
    
    bool try_lock() const {
        return mutex.try_lock();
    }
    
    template <typename Device>
    Device& access () {
        return *device;
    }
    
    std::type_index getID() {
        return id;
    }
};


template <typename...Devices> struct requires {
    static constexpr size_t size = sizeof...(Devices);
};

template <typename> using MapToLockableDevice = LockableDevice;


class ResourceProvider {
    std::map<std::type_index, std::unique_ptr<LockableDevice>> deviceMap;
    
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
    
public:
    template <typename T, typename...Ts>
    auto acquire(requires<T,Ts...>) {
        auto lockScope = lock(*deviceMap.at(std::type_index(typeid(T))), *deviceMap.at(std::type_index(typeid(Ts)))...);
        return std::make_pair(std::move(lockScope), std::make_tuple(std::make_shared<T>(), std::make_shared<Ts>()...));
    }
    
    auto acquire(requires<>) {
        return std::make_pair(std::shared_ptr<void>(),std::tuple<>());
    }
    
    template <typename T>
    void registerDevice(std::unique_ptr<T> device) {
        auto lockDev = std::make_unique<LockableDevice>(std::move(device));
        deviceMap.emplace(std::make_pair(std::type_index(typeid(T)), std::move(lockDev)));
    }
};


template <typename ... ResourceInterfaces>
class Executor {
    std::unique_ptr<ResourceProvider> resourceProvider;
    
public:
    Executor(std::unique_ptr<ResourceProvider> resourceProvider)
        : resourceProvider(std::move(resourceProvider))
    {
    }

    template<typename Callable, typename Devices, int ...S>
    void executeWithDevices(Callable&& cmd, Devices&& devs, Seq<S...>) {
        cmd(*std::get<S>(devs)...);
    }
    
    template <typename Command>
    void execute(Command&& cmd) {
        using requirements = typename std::decay_t<Command>::requirements;
        auto devices = resourceProvider->acquire(requirements());
        executeWithDevices(std::forward<Command>(cmd), std::move(devices.second), typename Gens<requirements::size>::type());
    }
};


TEST_CASE("Executor", "[executor]") {
    GIVEN("An executor and a resource provider") {
        struct Device1{};
        struct Device2{};
        auto provider = std::make_unique<ResourceProvider>();
        provider->registerDevice(std::make_unique<Device1>());
        provider->registerDevice(std::make_unique<Device2>());
        Executor<Device1> executor(std::move(provider));
        THEN("It can execute a command") {
            struct command {
                using requirements = requires<>;
                bool executed = false;
                void operator() () {
                    executed = true;
                }
            } cmd;
            executor.execute(std::move(cmd));
            REQUIRE(cmd.executed);
        }
        
        THEN("It can execute a command with ressource requirements") {
            
            struct command {
                using requirements = requires<Device1, Device2>;
                
                bool executed = false;
                void operator() (Device1& d1, Device2&) {
                    executed = true;
                }
            } cmd;
            executor.execute(cmd);
            REQUIRE(cmd.executed);
        }
    }
}
