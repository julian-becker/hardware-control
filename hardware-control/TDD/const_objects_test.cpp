#include <catch.h>
#include "const_objects.h"

#include <iostream>

#define STATIC_ASSERT(...)              \
    do {                                \
        static_assert(__VA_ARGS__,"");  \
        REQUIRE(__VA_ARGS__);           \
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


constexpr cst::array<5,int> create_array() {
    cst::array<5,int> a{0,0,0,0};
    a[0] = 7;
    a[3] = 1;
    return a;
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

TEST_CASE("printing an array") {
    using namespace cst;
    constexpr array<5,char> t1 = "uiae";
    for(const auto& v : t1)
        std::cout << "v = " << v << std::endl;
    
    std::cout << "t1 = " << t1 << std::endl;
    constexpr array<4,int> t2 = {1,2,3,4};
    std::cout << "t2 = " << t2 << std::endl;
}


TEST_CASE("printing a map") {
    using namespace cst;
    constexpr auto m = map<0,int,double>()
                        .insert(0,3.141)
                        .insert(2,2.717)
                        .insert(2,27)
                        .insert(7,12345.6789);
    
    std::cout << m << std::endl;
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


TEST_CASE("constexpr uuid class") {
    using namespace cst;
    constexpr auto uuid1 = "718EB018-A00A-11E5-A42E-4F4C358CC448"_uuid;
    constexpr auto uuid2 = "00000000-A00A-11E5-A42E-4F4C358CC448"_uuid;
    
    STATIC_ASSERT(uuid1 == "718EB018-A00A-11E5-A42E-4F4C358CC448"_uuid);
    STATIC_ASSERT(uuid1 != uuid2);
    
    std::cout << std::hex << uuid1.data1 << std::endl;
    std::cout << std::hex << uuid1.data2 << std::endl;
    std::cout << std::hex << uuid1.data3 << std::endl;
    std::cout << std::hex << uuid1.data4[0] << std::endl;
    
    STATIC_ASSERT(uuid1.data1 == 0x718EB018u);
    STATIC_ASSERT(uuid1.data2 == 0xA00A);
    STATIC_ASSERT(uuid1.data3 == 0x11E5);
    STATIC_ASSERT(uuid1.data4[0u] == 0xA4);
    STATIC_ASSERT(uuid1.data4[1u] == 0x2E);
    STATIC_ASSERT(uuid1.data4[2u] == 0x4F);
    STATIC_ASSERT(uuid1.data4[3u] == 0x4C);
    STATIC_ASSERT(uuid1.data4[4u] == 0x35);
    STATIC_ASSERT(uuid1.data4[5u] == 0x8C);
    STATIC_ASSERT(uuid1.data4[6u] == 0xC4);
    STATIC_ASSERT(uuid1.data4[7u] == 0x48);
}
