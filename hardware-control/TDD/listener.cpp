//
//  listener.cpp
//  hardware-control
//
//  Created by Julian Becker on 09.09.15.
//  Copyright (c) 2015 Julian Becker. All rights reserved.
//

#include <TDD/listener.h>
#include <catch.h>



TEST_CASE( "Create a listener", "[listener]" ) {
    WHEN("constructing an listener<int>") {
        THEN("no exception is thrown") {
            REQUIRE_NOTHROW(listener<int>([](int&&){}));
        }
    }
}

