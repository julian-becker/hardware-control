#include <TDD/with_destructor.h>
#include <catch.h>


TEST_CASE("with_destructor::add_raii","[with_destructor]") {
    GIVEN("an instance of with_destructor") {
        auto wd = new with_destructor<int>();
        
        WHEN("A functor is added with add_raii(functor)") {
            bool called = false;
            std::function<void()> fun = [&called]{ called = true; };
            wd->add_raii(0, fun);
            THEN("the functor is called when the with_destructor-instance is destroyed") {
                REQUIRE(!called);
                delete wd;
                REQUIRE(called);
            }
        }
        
        WHEN("A functor is added that will call remove_raii()") {
            bool called1 = false, called2 = false;
            std::function<void()> fun1 = [&called1,wd]{
                called1 = true;
                wd->remove_raii(2);
            };
            std::function<void()> fun2 = [&called2]{
                called2 = true;
            };
            wd->add_raii(1, fun1);
            wd->add_raii(2, fun2);
            THEN("the second functors that has been removed by the first is not called when the with_destructor-instance is destroyed") {
                REQUIRE(!called1);
                REQUIRE(!called2);
                delete wd;
                REQUIRE(called1);
                REQUIRE(!called2);
            }
        }

        
        WHEN("A functor is added that will call add_raii()") {
            bool called1 = false, called2 = false;
            std::function<void()> fun1 = [&called1,&called2,wd]{
                std::function<void()> fun2 = [&called2]{
                    called2 = true;
                };
                called1 = true;
                wd->add_raii(2,fun2);
            };
            wd->add_raii(1, fun1);
            THEN("the second functors that has been removed by the first is not called when the with_destructor-instance is destroyed") {
                REQUIRE(!called1);
                REQUIRE(!called2);
                delete wd;
                REQUIRE(called1);
                REQUIRE(called2);
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
