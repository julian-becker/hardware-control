//
//  observable.cpp
//  hardware-control
//
//  Created by Julian Becker on 09.09.15.
//  Copyright (c) 2015 Julian Becker. All rights reserved.
//

#include <TDD/observable.h>
#include <catch.h>


struct test_listener final : ilistener<int> {
    int value = 0;
    size_t triggered = 0u;
private:
    void handle_impl(int&& i) override {
        value = i;
        ++triggered;
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


TEST_CASE("Observable is copyable", "[observable]") {
    GIVEN("an observable") {
        observable<int> obs(37);
        WHEN("a copy is made") {
            observable<int> obsCopy(obs);

            THEN("the copy has the same value as the original") {
                REQUIRE(obsCopy == obs);
            }
        }
    }
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
        test_listener lstnr1, lstnr2, lstnr3;
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
        test_listener lstnr;
        WHEN("listener registers and unregisteres at the observable and a value is put into the observable") {
            obs.registerListener(lstnr);
            obs.unregisterListener(lstnr);
            obs = 42;
            THEN("the listener is not triggered") {
                REQUIRE(lstnr.triggered == 0);
            }
        }
    }
}


TEST_CASE("destroying listener before observable must be safe", "[observable]") {
    GIVEN("an observable<int> and a listener<int>") {
    
        auto obs = std::make_shared<observable<int>>(0);
        auto lstnr = std::make_shared<test_listener>();
        
        WHEN("the listener is destroyed and the observable triggered") {
            obs->registerListener(*lstnr);
            lstnr=0;
            *obs = 37;
            THEN("the program does not crash");
        }
    }
}


TEST_CASE("destroying observable before listener must be safe", "[observable][listener]") {
    GIVEN("an observable<int> and a listener<int>") {
        auto obs = new observable<int>(0);
        auto lstnr = new test_listener();
        
        WHEN("the observable is destroyed") {
            obs->registerListener(*lstnr);
            delete obs;
            delete lstnr;
            
            THEN("the registered listener can be safely destructed and the program does not crash");
        }
    }
}

TEST_CASE("moving an observable into another observable keeps the listeners intact","[observable]") {
    GIVEN("an observable and a listener") {
        observable<int> obs(0);
        bool triggered = false;
        auto lstnr = std::make_shared<test_listener>();
        WHEN("the listener is registered it the observable") {
            obs.registerListener(*lstnr);
            AND_WHEN("the observable is move-constructed into a different observable") {
                observable<int> obs2(std::move(obs));
                THEN("modifying the moved-to observable will trigger the listener") {
                    obs2 = 42;
                    REQUIRE(lstnr->triggered == 1);
                }
            }
            AND_WHEN("the observable is moved into a different observable") {
                observable<int> obs2(0);
                obs2 = std::move(obs);
                THEN("modifying the moved-to observable will trigger the listener") {
                    obs2 = 42;
                    REQUIRE(lstnr->triggered == 1);
                }
            }
        }
    }
}




