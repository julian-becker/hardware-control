#include "../catch.h"
#include <Framework/Any.h>
#include <string>

TEST_CASE( "Anything be stored into an Any", "[any]" ) {
    REQUIRE( Any(1).as<int>() == 1 );
    REQUIRE( Any(1.0).as<double>() == Approx(1.0) );
    REQUIRE( Any(std::string("string")).as<std::string>() == std::string("string"));
    
    WHEN("Value of type int is stored") {
        Any any_int(42);
        THEN("Accessing it in as a different type through 'as<T>()' does not throw") {
            REQUIRE_NOTHROW(any_int.as<double>());
        }
        
        THEN("Accessing it in as a different type through 'as<T>()' does throw") {
            REQUIRE_THROWS_AS(any_int.as_s<double>(), std::bad_cast);
        }
    }
}