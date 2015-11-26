#include <catch.h>
#include <TDD/interruptible.h>
#include <type_traits>
#include <future>
#include <list>
#include <iostream>

TEST_CASE("interruptible interface") {

    GIVEN("an implementation of interruptible") {
        struct test_interrupt : interruptible {
            size_t counter = 0u;
            size_t clear_counter = 0u;
            void interrupt_impl() override {
                counter++;
            }
            void clear_interrupt_impl() override {
                clear_counter++;
            }
        };
        
        test_interrupt interrupt;
        
        WHEN("calling interrupt() on the instance") {
            interrupt.interrupt();
            THEN("the private implementation method interrupt_impl() is invoked in the implementation") {
                REQUIRE(interrupt.counter == 1u);
                REQUIRE(interrupt.clear_counter == 0u);
                
                AND_WHEN("clearing the interrupt invokes clear_interrupt_impl()") {
                    interrupt.clear_interrupt();
                    REQUIRE(interrupt.counter == 1u);
                    REQUIRE(interrupt.clear_counter == 1u);
                }
            }
        }
    }
}

TEST_CASE("Interruptible semaphore") {
    SECTION("A semaphore is interruptible") {
        using t = std::is_base_of<interruptible,semaphore>;
        REQUIRE(t::value);
    }
    
    GIVEN("an instance of a sempaphore") {
        semaphore sem;
        std::atomic<bool> called(false);
        THEN("the thread calling wait() throws an interrupted_exception if interrupt() is called from the same thread before") {
            sem.interrupt();
            REQUIRE_THROWS_AS(sem.wait(), interrupted_exception);
        }
        
        THEN("the thread calling wait() throws an interrupted_exception if interrupt() is called from some other thread") {
            auto thread = std::async(std::launch::async,[&]{ sem.interrupt(); });
            REQUIRE_THROWS_AS(sem.wait(), interrupted_exception);
        }

        THEN("the thread calling wait() unblocks when someone calls post()") {
            auto thread1 = std::async(std::launch::async,[&]{ sem.post(); });
            auto thread2 = std::async(std::launch::async,[&]{ sem.post(); });
            auto thread3 = std::async(std::launch::async,[&]{ sem.post(); });
            auto thread4 = std::async(std::launch::async,[&]{ sem.post(); });
            REQUIRE_NOTHROW(sem.wait());
            REQUIRE_NOTHROW(sem.wait());
            REQUIRE_NOTHROW(sem.wait());
            REQUIRE_NOTHROW(sem.wait());
        }
    };
};

TEST_CASE("queue") {
    queue<size_t> q;
    
    SECTION("pushing one element") {
        q.push(1);
        REQUIRE(q.wait_and_pop() == 1);
    }

    SECTION("pushing two elements") {
        q.push(1);
        q.push(2);
        REQUIRE(q.wait_and_pop() == 1);
        REQUIRE(q.wait_and_pop() == 2);
    }

    SECTION("interrupting a wait") {
        auto thread = std::async(std::launch::async,[&]{ q.interrupt(); });
        REQUIRE_THROWS_AS(q.wait_and_pop(),interrupted_exception);
        q.push(43);
        REQUIRE_THROWS_AS(q.wait_and_pop(),interrupted_exception);
        q.clear_interrupt();
        REQUIRE(q.wait_and_pop() == 43);
    }
    
    SECTION("pushing from multiple threads") {
        std::list<std::thread> threads;
        std::atomic<size_t> count(0u);
        constexpr size_t N = 500;
        
        for(size_t i=0; i<N; i++) {
            threads.emplace_back(std::thread([&q,&count,i]{
                q.push(i);
                ++count;
            }));
            threads.back().detach();
        }

        for(size_t i=0; i<N; i++) {
            size_t v;
            REQUIRE_NOTHROW(v = q.wait_and_pop());
            --count;
        }
        
        REQUIRE(count == 0u);
    }
}


TEST_CASE("stack") {
    GIVEN("a stack") {
        deque<int> s;
        WHEN("pushing 1, 2, 3, 4, 5") {
            s.push_front(1);
            s.push_front(2);
            s.push_front(3);
            s.push_front(4);
            s.push_front(5);
            THEN("pop will return the last value pushed") {
                REQUIRE(5 == s.pop_front());
                REQUIRE(4 == s.pop_front());
                REQUIRE(3 == s.pop_front());
                REQUIRE(2 == s.pop_front());
                REQUIRE(1 == s.pop_front());
                REQUIRE_THROWS_AS(s.pop_front(), std::out_of_range);
            }
        }
    }
}

TEST_CASE("stack destruction") {
    GIVEN("a deque") {
        struct test_val {
            std::function<void(void)> fun;
            ~test_val() { fun(); }
        };
        deque<test_val>* s = new deque<test_val>();
        WHEN("pushing several instances") {
            bool inst1_destroyed = false, inst2_destroyed = false, inst3_destroyed = false, inst4_destroyed = false;
            s->push_front({[&]{ inst1_destroyed = true; }});
            s->push_front({[&]{ inst2_destroyed = true; }});
            s->push_front({[&]{ inst3_destroyed = true; }});
            s->push_front({[&]{ inst4_destroyed = true; }});
            THEN("pop will return the last value pushed") {
                delete s;
                REQUIRE(inst1_destroyed);
                REQUIRE(inst2_destroyed);
                REQUIRE(inst3_destroyed);
                REQUIRE(inst4_destroyed);
            }
        }
    }
}

TEST_CASE("deque: push_front -> pop_back") {
    deque<int> d;
    d.push_front(1);
    d.push_front(2);
    REQUIRE(1 == d.pop_back());    
    REQUIRE(2 == d.pop_back());
}

