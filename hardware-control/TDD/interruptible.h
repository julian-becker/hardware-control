#pragma once
#include <atomic>
#include <queue>

class interruptible {
    virtual void interrupt_impl() = 0;
    virtual void clear_interrupt_impl() = 0;
    
public:
    virtual ~interruptible() {}
    
    void interrupt() {
        interrupt_impl();
    }
    
    void clear_interrupt() {
        clear_interrupt_impl();
    }
};

struct interrupted_exception {};

class semaphore : public interruptible {
    std::atomic<size_t> counter;
    bool interrupted;
    std::condition_variable cond;
    std::mutex mutex;
 
    virtual void interrupt_impl() override {
        std::lock_guard<std::mutex> lock(mutex);
        interrupted = true;
        cond.notify_all();
    }
    
    virtual void clear_interrupt_impl() override {
        std::lock_guard<std::mutex> lock(mutex);
        interrupted = false;
    }
    
public:
    semaphore()
        : counter(0u)
        , interrupted(false) {}
    
    
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock,[&]{ return counter > 0u || interrupted; });
        if(interrupted)
            throw interrupted_exception{};
        --counter;
    }
    
    void post() {
        std::unique_lock<std::mutex> lock(mutex);
        ++counter;
        cond.notify_one();
    }
};

template <typename T>
class queue : public interruptible {
    std::queue<T> q;
    std::mutex mutex;
    semaphore sem;
    
    virtual void interrupt_impl() override {
        sem.interrupt();
    }

    virtual void clear_interrupt_impl() override {
        sem.clear_interrupt();
    }
    
public:
    void push(T i) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            q.push(std::move(i));
        }
        sem.post();
    }
    
    T wait_and_pop() {
        sem.wait();
        std::lock_guard<std::mutex> lock(mutex);
        auto val = std::move(q.front());
        q.pop();
        return val;
    }
};




