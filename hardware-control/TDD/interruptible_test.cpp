#include <catch.h>
#include <TDD/interruptible.h>
#include <type_traits>
#include <future>

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
    queue<int> q;
    
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
        auto thread1 = std::async(std::launch::async,[&]{ q.push(1); });
        auto thread2 = std::async(std::launch::async,[&]{ q.push(2); });
        auto thread3 = std::async(std::launch::async,[&]{ q.push(3); });
        REQUIRE_NOTHROW(q.wait_and_pop());
        REQUIRE_NOTHROW(q.wait_and_pop());
        REQUIRE_NOTHROW(q.wait_and_pop());
    }
}