#pragma once
#include "IVisitor.h"
#include "IVisitable.h"

template <typename CLASS, typename INTERFACE, typename T, typename...TS> struct
VisitorImpl : VisitorImpl<CLASS, INTERFACE, TS...> {
    using VisitorImpl<CLASS, INTERFACE, TS...>::visit;
    typename INTERFACE::visit_return_type visit(T& visitable) final override {
        return ::visit<T,CLASS>::call(visitable,static_cast<CLASS&>(*this));
    }
};

template <typename CLASS, typename INTERFACE, typename T> struct
VisitorImpl<CLASS, INTERFACE, T> : INTERFACE {
    using INTERFACE::visit;
    typename INTERFACE::visit_return_type visit(T& visitable) final override {
        return ::visit<T,CLASS>::call(visitable,static_cast<CLASS&>(*this));
    }
};

template <typename CLASS, typename RETURN_TYPE, typename T, typename...TS> using
Visitor = VisitorImpl<CLASS, IVisitor<RETURN_TYPE,T,TS...>,T,TS...>;



