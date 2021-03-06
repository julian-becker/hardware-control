#pragma once
#include "IVisitor.h"
#include "IVisitable.h"

template <typename CLASS, typename INTERFACE, typename T, typename...TS> struct
VisitorImpl : VisitorImpl<CLASS, INTERFACE, TS...> {
    using VisitorImpl<CLASS, INTERFACE, TS...>::visit;
    Any visit(T& visitable) final override {
        return ::visit<T,CLASS>::call(visitable,static_cast<CLASS&>(*this));
    }
};

template <typename CLASS, typename INTERFACE, typename T> struct
VisitorImpl<CLASS, INTERFACE, T> : INTERFACE {
    Any visit(T& visitable) final override {
        return ::visit<T,CLASS>::call(visitable,static_cast<CLASS&>(*this));
    }
};

template <typename CLASS, typename RETURN_TYPE, typename INTERFACE, typename T, typename...TS> struct
Visitor : VisitorImpl<CLASS,INTERFACE,T,TS...> {
    using visit_return_type = RETURN_TYPE;
};



