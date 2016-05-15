//
//  streams.cpp
//  hardware-control
//
//  Created by Julian Becker on 14.05.16.
//  Copyright © 2016 Julian Becker. All rights reserved.
//

#include "streams.hpp"
#include <catch.h>
#include <iostream>
#include <type_traits>
#include <cxxabi.h>
#include <string>
namespace {

std::string demangle(const char* name);

template <class T>
std::string printtype() {
    return demangle(typeid(T).name());
}


std::string demangle(const char* name) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
}

}





template <typename Element> class
stream {
    using element_type = Element;
};

template <
    typename Element,
    typename Impl> class
input_stream
    : public stream <Element>
{
    using input_iterator = typename Impl::input_iterator;
    input_iterator begin() { return begin_impl(); }
private:
    virtual input_iterator begin_impl() = 0;
};



template <
    typename T,
    typename IteratorState> class
input_iterator {
    using
    iterator_state = IteratorState;
    
    using
    value_type = T;
    
    virtual void
    moveNext_impl() { return impl->moveNext_impl(); }
    
    virtual value_type&
    access_impl  () { return impl->access_impl  (); }
    
    virtual input_iterator*
    clone_impl   () { return impl->clone_impl   (); }
    
    virtual bool
    compare_impl (const input_iterator& other) { return impl->compare_impl (other); }
    
    input_iterator* impl;
    
protected:
    input_iterator()
        : impl(this)
    {}
    
public:
    ~input_iterator()
    {}
    
    input_iterator(const input_iterator& other)
        : impl(other.impl->clone_impl())
    {}
    
    input_iterator(input_iterator&& other)
        : impl(std::move(other.impl))
    {}
    
    input_iterator& operator= (const input_iterator& other) {
        auto copy(other);
        *this = std::move(copy);
        return *this;
    }
    
    input_iterator& operator= (input_iterator&& other) {
        std::swap(impl, other.impl );
        return *this;
    }
    
    value_type& operator* () {
        return impl->access_impl();
    }
    
    value_type& operator* () const {
        return impl->access_impl();
    }
    
    value_type* operator-> () {
        return &impl->access_impl();
    }
    
    input_iterator& operator++() {
        impl->moveNext_impl();
        return *this;
    }
    
    input_iterator operator++(int) {
        auto copy(*this);
        operator++();
        return copy;
    }
    
    friend bool operator == (const input_iterator& a, const input_iterator& b) {
        return a.impl->compare_impl(b);
    }
    
    friend bool operator != (const input_iterator& a, const input_iterator& b) {
        return !a.impl->compare_impl(b);
    }
};


class
running_integer final
    : public input_iterator<int, int*>
{
    using base_type = input_iterator<int, int*>;
    using value_type = int;
    using iterator_state = int*;
    
    void
    moveNext_impl() override {
        value++;
    }
    
    value_type&
    access_impl  () override {
        return value;
    }
    
    input_iterator*
    clone_impl   () override {
        return new running_integer(value);
    }
    
    bool
    compare_impl(const input_iterator& other) override {
        return value == static_cast<running_integer const&>(other).value;
    }
    
    int value;
    
public:
    running_integer(int startVal)
        : base_type()
        , value(std::move(startVal))
    {}
    
    ~running_integer() {
    }
};




TEST_CASE("ByteStreamFromFile","Stream") {
   
}


template <typename Element> struct
field {};



template <typename E, typename...Es> struct
container : container<E>, container<Es...> {
    using container<E>::operator[];
    using container<Es...>::operator[];
    
    void writeTo (std::ostream& ostr, size_t indent=0u) const {
        static_cast<container<E> const*>(this)->writeTo(ostr,indent);
        static_cast<container<Es...> const*>(this)->writeTo(ostr,indent);
    }
    
    friend std::ostream& operator<< (std::ostream& ostr, const container& c) {
        c.writeTo(ostr);
        return ostr;
    }
};

template <typename T> struct DataOutFormat {
    static void writeTo(std::ostream& ostr, const T& val) {
        ostr << val;
    }
};

template <> struct
DataOutFormat<std::string> {
    static void writeTo(std::ostream& ostr, const std::string& str) {
        ostr << "\"" << str << "\"";
    }
};

template <typename Element> struct
container<field<Element>>  {
    auto&
    operator[](field<Element>) {
        return value;
    }
    
    void writeTo (std::ostream& ostr, size_t indent=0u) const {
        std::string s(indent,' ');
        ostr << s << "SubContainer {\"" << Element{} << "\" \"" << Element{} << "\" {\n"
             << s << "AttrContainer {\n";
        value.writeTo(ostr,indent+4u);
        ostr << s << "}\n";
    }
private:
    typename Element::value_type value;
};




template <typename Element> struct
container<Element> {
    template <typename ElementIn>
    std::enable_if_t<
        std::is_same<ElementIn,Element>::value && Element{}.index() == ElementIn{}.index(),
        typename Element::value_type&>
    operator[](ElementIn) {
        return value;
    }
    
    template <typename ElementIn>
    std::enable_if_t<
        std::is_same<ElementIn,Element>::value && Element{}.index() == ElementIn{}.index(),
        typename Element::value_type&>
    operator[](ElementIn) const {
        return value;
    }
    
    void writeTo (std::ostream& ostr, size_t indent=0u) const {
//        std::cout << "AAAAAAAAA" << printtype<attr>() << std::endl;
        std::string s(indent,' ');
        ostr << s << "Attr<" << Element{}.type_name() << "> { \n"
             << s << "\tName \"" << Element{} << "\"\n"
             << s << "\tValue ";
        DataOutFormat<decltype(value)>::writeTo(ostr,value);
        ostr << s << "\n"
             << s << "}\n";
    }

    typename Element::value_type value;
};


#define DEFER_(...) __VA_ARGS__
#define DEFER(...) DEFER_(__VA_ARGS__)
#define CONCATENATE(X,Y,Z)  X ## Y ## Z
#define STRUCT(NAME)  CONCATENATE( S_,NAME, )
#define FIELD(NAME)   CONCATENATE( F_,NAME, )
#define TYPE(NAME)    CONCATENATE( T_,NAME, )

#define SYMBOLIC(NAME,TYPE) \
    static constexpr struct STRUCT(NAME) { \
        constexpr operator const char*() const { return #NAME; } \
        constexpr const char* type_name() const { return #TYPE; } \
        constexpr int index() const { return __COUNTER__; } \
        using value_type = TYPE; \
    } NAME {};


#define SYMBOLIC_F(NAME,T) \
    struct STRUCT(T) { \
        constexpr operator const char*() const { return #NAME; } \
        constexpr const char* type_name() const { return #T; } \
        constexpr int index() const { return __COUNTER__; } \
        using value_type = T; \
    }; \
    constexpr field<STRUCT(T)> NAME {};

#define CONTAINER_BEGIN(NAME) \
    using TYPE(NAME) = container<
#define ATTR_DECL(ATTR) \
    std::decay_t<decltype(FIELD(ATTR))>
#define CONTAINER_END() \
    >;

#define CONTAINER_DECL_BEGIN(NAME) \
    namespace FIELD(NAME) {
#define CONTAINER_DECL_END() \
    }



#define ATTR(NAME,TYPE) NAME, TYPE
#define APPLY_TO(F, ...) F(__VA_ARGS__)


CONTAINER_DECL_BEGIN(Sensor)
    SYMBOLIC(Name,       std::string)
    SYMBOLIC(SensorType, int)
    SYMBOLIC(DistOffset, double)
    SYMBOLIC(ApdHv, double)
CONTAINER_DECL_END()

CONTAINER_BEGIN(Sensor)
    ATTR_DECL(Sensor::Name),
    ATTR_DECL(Sensor::SensorType),
    ATTR_DECL(Sensor::DistOffset),
    ATTR_DECL(Sensor::ApdHv)
CONTAINER_END()

CONTAINER_DECL_BEGIN(Mirror)
    SYMBOLIC(Name,       std::string)
    SYMBOLIC(TriggerOffset, int)
CONTAINER_DECL_END()

CONTAINER_BEGIN(Mirror)
    ATTR_DECL(Mirror::Name),
    ATTR_DECL(Mirror::TriggerOffset)
CONTAINER_END()

CONTAINER_DECL_BEGIN(ScannerParams)
    SYMBOLIC_F(SensorField, T_Sensor)
    SYMBOLIC_F(MirrorField, T_Mirror)
CONTAINER_DECL_END()

CONTAINER_BEGIN(ScannerParams)
    ATTR_DECL(ScannerParams::SensorField),
    ATTR_DECL(ScannerParams::MirrorField)
CONTAINER_END()

#define STR(X) X
#define STRN(...) STR(#__VA_ARGS__)
#define DROP(F, HEAD, ...) APPLY_TO(F, __VA_ARGS__)
#define DUP(F, HEAD, ...) APPLY_TO(F, HEAD, HEAD, __VA_ARGS__)
#define PUSH(F, X) APPLY_TO(F, X,
#define OVER(F, HEAD, NEXT, ...) APPLY_TO(F,NEXT,HEAD,NEXT,__VA_ARGS__)
#define DUMP(...) APPLY_TO(STRN,__VA_ARGS__)


#define TESTSTR DUMP(OVER(CONT, 1,2,3,4,5))


TEST_CASE("tree","mpl") {
    using namespace F_ScannerParams;
    using namespace F_Sensor;
    
    T_ScannerParams params;
    params[SensorField];
    params[SensorField][SensorType] = 14.4;
    params[SensorField][ApdHv] = 57.3;
    params[SensorField][DistOffset] = false;
    
    std::cout << "params[SensorField][Name]="<<params<<std::endl;
    std::cout << ":::::::::::";
    params.writeTo(std::cout);
//    std::cout << ":::::::::" << printtype<decltype(params[SensorField])>() << std::endl;
}




























