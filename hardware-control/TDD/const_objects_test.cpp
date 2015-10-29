#include <catch.h>
#include "const_objects.h"

#include <iostream>

#define STATIC_ASSERT(...)              \
    do {                                \
        static_assert(__VA_ARGS__,"");  \
    } while(false)


TEST_CASE("constexpr integer literals") {
    using namespace cst;
    
    constexpr auto i42 = 42_cst;
    STATIC_ASSERT(i42.value == 42);
    
    constexpr auto i44 = 44_cst;
    STATIC_ASSERT(i44.value == 44);
}


TEST_CASE("adding constexpr ints") {
    using namespace cst;
    constexpr auto result = 42_cst + 3_cst;
    STATIC_ASSERT(result + 1_cst == 46_cst);
    STATIC_ASSERT(0_cst != 1_cst);
    STATIC_ASSERT(7_cst - 5_cst == 2_cst);
    STATIC_ASSERT(5_cst - 7_cst == -2_cst);
}

TEST_CASE("comparing int_t<N> and val<int>") {
    using namespace cst;
    STATIC_ASSERT(val<int>(5) == 5_cst);
    STATIC_ASSERT(5_cst == val<int>(5));
    STATIC_ASSERT(5_cst != val<int>(1));
    STATIC_ASSERT(val<int>(5) != 1_cst);
}

TEST_CASE("constant value") {
    constexpr cst::val<const char*> str = "I'm a constexpr string";
    
    constexpr auto int_44 = cst::int_t<44>{};
    constexpr cst::val<int> ff(int_44);
    constexpr cst::val<int> i = 42;
    STATIC_ASSERT(i == 42);
}

TEST_CASE("constexpr pair") {
    using namespace cst;
    constexpr pair<int,double> p(42,57.);
    STATIC_ASSERT(p[0_cst] == 42);
    STATIC_ASSERT(p[1_cst] == 57.);
}

TEST_CASE("constexpr tuple") {
    using namespace cst;
    constexpr tuple<char,int,double,long long> t('A',42,3.14,0xAAAAAAAAAAAAAAAALL);
    STATIC_ASSERT(t[0_cst] == 'A');
    STATIC_ASSERT(t[1_cst] == 42);
    STATIC_ASSERT(t[2_cst] == 3.14);
    STATIC_ASSERT(t[3_cst] == 0xAAAAAAAAAAAAAAAALL);
}


TEST_CASE("constexpr array") {
    using namespace cst;
    constexpr array<5,char> t{'A','B','C','D','E'};
    
    STATIC_ASSERT(t[0] == 'A');
    STATIC_ASSERT(t[0_cst] == 'A');
    STATIC_ASSERT(t[val<int>(3)] == 'D');

    constexpr array<5,char> t1 = "uiae";
    STATIC_ASSERT(t1[0] == 'u');
    STATIC_ASSERT(t1[1] == 'i');
    STATIC_ASSERT(t1[2] == 'a');
    STATIC_ASSERT(t1[3] == 'e');
    STATIC_ASSERT(t1[4] == '\0');
}



TEST_CASE("mapping constexpr values") {
    using namespace cst;
    constexpr map<2,int,char> m{
        pair<int,char>(0,'0'),
        pair<int,char>(10,'A')
    };
    
    STATIC_ASSERT(m[0]  == '0');
    STATIC_ASSERT(m[10] == 'A');
    
    constexpr auto m3 = m.insert(11,'B')
                         .insert(12,'C')
                         .insert(13,'D');
                         
    STATIC_ASSERT(m3[11] == 'B');
}

