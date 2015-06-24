#pragma once
#include <Framework/Any.h>

template <typename VISITABLE, typename...VISITABLES> struct
IVisitorBase : IVisitorBase<VISITABLES...> {
    using visitor_interface = IVisitorBase;
    using IVisitorBase<VISITABLES...>::visit;
    virtual Any visit(VISITABLE&) = 0;
};

template <typename VISITABLE> struct
IVisitorBase<VISITABLE> {
    using visitor_interface = IVisitorBase;
    virtual Any visit(VISITABLE&) = 0;
};


template <typename R, typename INTERFACE, typename VISITABLE, typename...VISITABLES> struct
IVisitorImpl : IVisitorImpl<R,INTERFACE,VISITABLES...> {
    using visit_return_type = R;
    using IVisitorImpl<R,INTERFACE,VISITABLES...>::visit;
    using IVisitorImpl<R,INTERFACE,VISITABLES...>::visit_typed;
    
    virtual R visit_typed(VISITABLE& visitable) {
        return visit(visitable).template as<R>();
    }
};

template <typename R, typename INTERFACE, typename VISITABLE> struct
IVisitorImpl<R,INTERFACE,VISITABLE> : INTERFACE {
    using INTERFACE::visit;
    using visit_return_type = R;
    
    virtual R visit_typed(VISITABLE& visitable) {
        return visit(visitable).Any::template as<R>();
    }
};

template <typename R, typename...VISITABLES> using
IVisitor = IVisitorImpl<R, IVisitorBase<VISITABLES...>, VISITABLES...>;