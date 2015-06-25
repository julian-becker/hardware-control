#pragma once
#include <Framework/Any.h>

template <typename VISITABLE, typename...VISITABLES> struct
IVisitor : IVisitor<VISITABLES...> {
    using visitor_interface = IVisitor;
    using IVisitor<VISITABLES...>::visit;
    virtual Any visit(VISITABLE&) = 0;
};

template <typename VISITABLE> struct
IVisitor<VISITABLE> {
    using visitor_interface = IVisitor;
    virtual Any visit(VISITABLE&) = 0;
};

