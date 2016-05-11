#include <catch.h>
#include <cmath>
#include <iostream>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <sstream>
#include <utility>



template <typename T>
std::shared_ptr<std::decay_t<T>> forward_shared(T&& x) {
    return std::make_shared<std::decay_t<T>>(std::forward<T>(x));
}


template<typename T>
class Delegate;

template <typename T>
class Task;

template<typename Ret, typename ...Args> class
Delegate<Ret(Args...)> {
    struct Internals;
    
    using
    CallHandler = Ret(*)(std::shared_ptr<void>, Args...);
    
    std::shared_ptr<void> instance{ nullptr };
    CallHandler callback{ nullptr };
    
    Delegate(std::shared_ptr<void> obj, CallHandler funcPtr);
    
public:
    Ret operator()(Args... args) const;
    bool operator==(const Delegate& other) const;
    
    size_t hash() const {
        auto res = reinterpret_cast<size_t>(instance.get())^reinterpret_cast<size_t>(callback);
        std::cout << "hash="<<res << std::endl;
        return res;
    }
    
    template<Ret(*funcPtr)(Args...)> static Delegate
    create();
    
    template<typename T, Ret(T::*funcPtr)(Args...)> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) const> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) volatile> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) const volatile> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) &> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) const &> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) volatile &> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T, Ret(T::*funcPtr)(Args...) const volatile &> static Delegate
    create(std::shared_ptr<T> obj);
    
    template<typename T> static Delegate
    create(std::shared_ptr<T> t);
    
    template<typename T> static Delegate
    create(T&& t);
    
    
    template <typename T>
    operator Delegate<T(Args...)> () const {
        struct RemoveRetType {
            std::shared_ptr<void> instance;
            CallHandler callback;
            RemoveRetType(std::shared_ptr<void> instance, CallHandler callback) : instance(std::move(instance)), callback(std::move(callback)) {}
            void operator() (Args... args) {
                callback(instance, args...);
            }
        };
        
        return Delegate<void(Args...)>::create(std::make_shared<RemoveRetType>(instance, callback));
    }
};

template<typename Ret, typename ...Args>
Ret Delegate<Ret(Args...)>
::operator()(Args... args) const {
    return callback(instance, args...);
}

template<typename Ret, typename ...Args>
bool Delegate<Ret(Args...)>
::operator==(const Delegate& other) const {
    return instance == other.instance && callback == other.callback;
}



template<typename Ret, typename ...Args>
template<Ret(*funcPtr)(Args...)> auto Delegate<Ret(Args...)>
::create() -> Delegate {
    return Delegate{ nullptr, &Delegate::Internals::template toGlobalFn<funcPtr> }; // nullptr as first parameter because static/global functions can be called directly
}


template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...)> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toMemberFn<T, funcPtr> };
}



template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) const> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toConstMemberFn<T, funcPtr> };
}



template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) volatile> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toVolatileMemberFn<T, funcPtr> };
}


template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) const volatile> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toConstVolatileMemberFn<T, funcPtr> };
}

template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) &> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toLValueRefMemberFn<T, funcPtr> };
}

template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) const &> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toConstLValueRefMemberFn<T, funcPtr> };
}

template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) volatile &> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toVolatileLValueRefMemberFn<T, funcPtr> };
}

template<typename Ret, typename ...Args>
template<typename T, Ret(T::*funcPtr)(Args...) const volatile &> auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> obj) -> Delegate {
    return Delegate{ obj, &Delegate::Internals::template toConstVolatileLValueRefMemberFn<T, funcPtr> };
}

template<typename Ret, typename ...Args>
template<typename T>
auto Delegate<Ret(Args...)>
::create(std::shared_ptr<T> t) -> Delegate {
    return Delegate{ t, &Delegate::Internals::template toFunctor<T> };
}

template<typename Ret, typename ...Args>
template<typename T>
auto Delegate<Ret(Args...)>::create(T&& t) -> Delegate {
    return Delegate::create(forward_shared(std::move(t)));
}


template<typename Ret, typename ...Args>
Delegate<Ret(Args...)>::Delegate(std::shared_ptr<void> obj, CallHandler funcPtr) : instance{ obj }, callback{ funcPtr }
{
}

template<typename Ret, typename ...Args>
struct Delegate<Ret(Args...)>::Internals {
    
    template<Ret(*funcPtr)(Args...)>
    static Ret toGlobalFn(std::shared_ptr<void>, Args... args)
    {
        return funcPtr(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...)>
    static Ret toMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) const>
    static Ret toConstMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T const*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) volatile>
    static Ret toVolatileMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T volatile*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) const volatile>
    static Ret toConstVolatileMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T const volatile*>(instance.get())->*funcPtr)(args...);
    }
    
    
    template<typename T, Ret(T::*funcPtr)(Args...) &>
    static Ret toLValueRefMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) const &>
    static Ret toConstLValueRefMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T const*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) volatile &>
    static Ret toVolatileLValueRefMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T volatile*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T, Ret(T::*funcPtr)(Args...) const volatile &>
    static Ret toConstVolatileLValueRefMemberFn(std::shared_ptr<void> instance, Args... args)
    {
        return (static_cast<T const volatile*>(instance.get())->*funcPtr)(args...);
    }
    
    template<typename T>
    static Ret toFunctor(std::shared_ptr<void> functor, Args... args)
    {
        return (*static_cast<T*>(functor.get()))(args...);
    }
    
};


class IExecutor {
    virtual void execute_impl(Delegate<void()> executable) = 0;
public:
    void execute(Delegate<void()> executable) { execute_impl(std::move(executable)); }
};

class ImmediateExecutor : public IExecutor {
    virtual void execute_impl(Delegate<void()> executable) {
        executable();
    }
};

class TestThreadExecutor : public IExecutor {
    virtual void execute_impl(Delegate<void()> executable) {
        std::thread t([=]{executable();});
        t.detach();
    }
};


template <typename...Args> class
Dispatcher;

template <typename...Args>
struct dispatch_dsl_temp {
    Dispatcher<Args...>& dispatcher;
    std::shared_ptr<IExecutor> executor;
    
    template <typename Ret, Ret(*GlobalFn)(Args...)>
    void to() {
        auto p = std::make_pair(Delegate<Ret(Args...)>::template create<GlobalFn>(), executor);
        dispatcher.observers.emplace(std::move(p));
    }

    template <typename Ret>
    void to(Delegate<Ret(Args...)> delegate) {
        auto p = std::make_pair(delegate, executor);
        dispatcher.observers.emplace(std::move(p));
    }

    template <typename T>
    void to(T&& fn) {
        using Ret = decltype(fn(std::declval<Args>()...));
        to(Delegate<Ret(Args...)>::create(forward_shared(std::forward<T>(fn))));
    }
    
};


template <typename...Args> class
Dispatcher {
    struct DelegateHash {
        template <typename Ret>
        size_t operator() (const Delegate<Ret(Args...)>& del) const {
            return del.hash();
        }
    };
    friend class dispatch_dsl_temp<Args...>;
    using delegate_type = Delegate<void(Args...)>;
    std::unordered_map<delegate_type,std::shared_ptr<IExecutor>, DelegateHash> observers;
public:
    Dispatcher()
        : sharedState(std::make_shared<SharedState>(this))
    {}
    
    ~Dispatcher() {
        std::lock_guard<std::mutex> lock(sharedState->mutex);
        sharedState->parent = nullptr;
    }

    auto via(std::shared_ptr<IExecutor> executor) {
        return dispatch_dsl_temp<Args...>{ *this, std::move(executor) };
    }
    
    
    template <typename Ret, Ret(*GlobalFn)(Args...)>
    void remove() {
        observers.erase(Delegate<Ret(Args...)>::template create<GlobalFn>());
    }
    
    void operator() (Args... args) {
        for(const auto& entry : observers) {
            auto& delegate = entry.first;
            auto& executor = entry.second;
            auto task = Delegate<void()>::create(forward_shared([=]{
                    delegate(args...);
            }));
            executor->execute(std::move(task));
        }
    }
    
    struct SharedState {
        Dispatcher* parent;
        std::mutex mutex;
        SharedState(Dispatcher* parent) : parent(std::move(parent)) {}
    };
    std::shared_ptr<SharedState> sharedState;
    
    Delegate<void(Args...)> input() {
        struct InputDelegate {

            std::shared_ptr<SharedState> sharedState;
            InputDelegate(std::shared_ptr<SharedState> sharedState) : sharedState(sharedState) {}
            void operator() (Args...args) {
                std::lock_guard<std::mutex> lock(sharedState->mutex);
                if(const auto parent = sharedState->parent)
                    (*parent)(args...);
                else
                    throw std::exception();
            }
        };
        
        auto inputDelegateInstance = std::make_shared<InputDelegate>(sharedState);
        return Delegate<void(Args...)>::create(inputDelegateInstance);
        
    }
};






int uiae(int,float) {
    std::cout << "uiae is called............" << std::endl;
    return 42;
}

    struct UserStruct
    {
        template <typename T, typename T1>
        T1 member(T a, float b) const volatile&
        {
            return a + static_cast<int>(b);
        }

        template <typename T, typename T1>
        T1 member1(T a, float b) const volatile&
        {
            return a + static_cast<int>(b);
        }
        
        
        int operator() (int,float) const { return 7; }
    };

struct teststruct {
    teststruct() { std::cout << "creating" << std::endl; }
    ~teststruct() { std::cout << "destroying" << std::endl; }
    operator int() { return 55; }
};

struct test {
    constexpr test(){}
    constexpr operator auto() const { return 42; }
};


#include <string>
#include <typeinfo>

std::string demangle(const char* name);

template <class T>
std::string type(const T& t) {

    return demangle(typeid(t).name());
}
#include <cxxabi.h>

std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

template <typename T>
class Task;

template <typename Ret, typename...Args>
class Task<Ret(Args...)> {
    Delegate<void(Args...)> delegateIn;
    mutable Dispatcher<Ret> dispatcherOut;
    Delegate<Ret(Args...)> delegate;
    
public:
    Task(Delegate<Ret(Args...)> delegateIn)
        : delegate(std::move(delegateIn))
        , delegateIn(Delegate<void(Args...)>::create([this](Args... args) {
                dispatcherOut(delegate(std::forward<Args>(args)...));
            }))
    {
    }
    
    Dispatcher<Ret>& output() const { return dispatcherOut; }
    Delegate<void(Args...)> input() const { return delegateIn; }
};

template <typename...Args>
class Task<void(Args...)> {
    Delegate<void(Args...)> delegateIn;
    Dispatcher<> dispatcherOut;
    Delegate<void(Args...)> delegate;
    
public:
    Task(Delegate<void(Args...)> delegateIn)
        : delegate(std::move(delegateIn))
        , delegateIn(Delegate<void(Args...)>::create([this](Args... args) {
                delegate(std::forward<Args>(args)...);
                dispatcherOut();
            }))
    {
    }
    
    Dispatcher<void>& output() const { return dispatcherOut; }
    Delegate<void(Args...)> input() const { return delegateIn; }
};

template <typename Ret1, typename Ret2, typename...Args1, typename...Args2>
Task<Ret1(Args1...)> operator >>= (const Task<Ret1(Args1...)>& a, const Task<Ret2(Args2...)>& b) {
    a.output().via(std::make_shared<ImmediateExecutor>()).to(b.input());
    return a;
}


std::string outHandler(int i) {
    std::stringstream str;
    str << "received output: "<<i << std::endl;
    return str.str();
}

void outHandler2(std::string i) {
    std::cout << "received output: "<<i << std::endl;
}

TEST_CASE("Delegetion","test1") {
    std::shared_ptr<IExecutor> executor1 = std::make_shared<TestThreadExecutor>();
    std::shared_ptr<IExecutor> executor2 = std::make_shared<ImmediateExecutor>();
    auto s = std::make_shared<UserStruct>();
    auto d2 = Delegate<int(int, float)>::create<UserStruct, &UserStruct::member>(s);
    auto d3 = Delegate<int(int, float)>::create<UserStruct, &UserStruct::member>(s);
    auto d4 = Delegate<int(int, float)>::create<UserStruct, &UserStruct::member1>(s);
    auto handler = Delegate<std::string(int)>::create<outHandler>();
    auto handler2 = Delegate<void(std::string)>::create<outHandler2>();
    d2.hash();
    d3.hash();
    d4.hash();
    
    Task<int(int, float)> task(d3);
    Task<std::string(int)> taskHandler(handler);
    Task<void(std::string)> taskHandler2(handler2);
    task >>= taskHandler >>= taskHandler2;
    auto taskinput(task.input());
    taskinput(4,5.0);
    
    auto dispatch = std::make_shared<Dispatcher<int,float>>();
    dispatch->via(executor1).to(Delegate<int(int, float)>(d2));
    {
        auto x = std::make_shared<teststruct>();
        dispatch->via(executor1).to([
            x](int, float){
            std::cout << "x=" << (*x) << std::endl;
            });
        
        std::cout << "leaving scope" << std::endl;
    }
    std::cout << "left scope" << std::endl;
    auto d5 = Delegate<void(int,float)>::create([](int, float){});
    
    auto input = dispatch->input();
    
    //dispatch = nullptr;
    input(113241234,4444.444);
    
}