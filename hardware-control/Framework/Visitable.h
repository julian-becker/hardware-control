#pragma once
#include "IVisitable.h"

template <typename CLASS, typename INTERFACE, typename ITER = INTERFACE> struct
VisitableImpl;


template <typename CLASS, typename INTERFACE, typename VISITOR, typename...VISITORS> struct
VisitableImpl<CLASS,INTERFACE,IVisitable<VISITOR,VISITORS...>> : VisitableImpl<CLASS,INTERFACE,IVisitable<VISITORS...>> {
    using VisitableImpl<CLASS,INTERFACE,IVisitable<VISITORS...>>::accept;
    typename VISITOR::visit_return_type accept(VISITOR& visitor) final override {
        return visitor.visit(static_cast<CLASS&>(*this));
    };
};


template <typename CLASS, typename INTERFACE, typename VISITOR> struct
VisitableImpl<CLASS,INTERFACE,IVisitable<VISITOR>> : INTERFACE {
    using INTERFACE::accept;
    typename VISITOR::visit_return_type accept(VISITOR& visitor) final override {
        return visitor.visit(static_cast<CLASS&>(*this));
    };
};

template <typename CLASS, typename INTERFACE> using
Visitable = VisitableImpl<CLASS, INTERFACE>;