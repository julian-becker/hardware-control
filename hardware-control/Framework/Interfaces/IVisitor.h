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


template <typename R, typename INTERFACE, typename VISITABLE, typename...VISITABLES> struct
IVisitorTypedImpl : IVisitorTypedImpl<R,INTERFACE,VISITABLES...> {
    using visit_return_type = R;
    using IVisitorTypedImpl<R,INTERFACE,VISITABLES...>::visit;
    using IVisitorTypedImpl<R,INTERFACE,VISITABLES...>::visit_typed;
    
    virtual R visit_typed(VISITABLE& visitable) {
        return visit(visitable).template as<R>();
    }
};

template <typename R, typename INTERFACE, typename VISITABLE> struct
IVisitorTypedImpl<R,INTERFACE,VISITABLE> : INTERFACE {
    using INTERFACE::visit;
    using visit_return_type = R;
    
    virtual R visit_typed(VISITABLE& visitable) {
        return visit(visitable).Any::template as<R>();
    }
};

template <typename R, typename...VISITABLES> using
IVisitorTyped = IVisitorTypedImpl<R, IVisitor<VISITABLES...>, VISITABLES...>;