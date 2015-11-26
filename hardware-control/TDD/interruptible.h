#pragma once
#include <atomic>
#include <exception>

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


template <typename T> class
deque {
    struct node {
        T value;
        node* next;
        node* prev;
    };
    
    node* front = nullptr;
    node* back = nullptr;
    
public:
    deque() {};
    ~deque() {
        for(auto nod = front; nod != nullptr; nod = nod->next) {
            delete nod;
        }
            
    }
    
    void push_front(T&& val) {
        auto nod = new node{std::move(val), nullptr, nullptr};
        if(!front) {
            front = nod;
            back  = nod;
            return;
        }
        front->prev = nod;
        nod->next = front;
        front = nod;
    }
    
    T pop_front() {
        if(!front)
            throw std::out_of_range("Cannot pop without pushing first");
        T val = std::move(front->value);
        auto old_node = front;
        front = front->next;
        if(front)
            front->prev = nullptr;
        delete old_node;
        return val;
    }
    
    T pop_back() {
        if(!front)
            throw std::out_of_range("Cannot pop without pushing first");
        T val = std::move(back->value);
        auto old_node = back;
        back = back->prev;
        if(back)
            back->next = nullptr;
        else
            front = nullptr;
        delete old_node;
        return val;
    }
};

template <typename T>
class queue : public interruptible {
    deque<T> q;
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
            q.push_front(std::move(i));
        }
        sem.post();
    }
    
    T wait_and_pop() {
        sem.wait();
        std::lock_guard<std::mutex> lock(mutex);
        auto val = std::move(q.pop_back());
        return val;
    }
};






