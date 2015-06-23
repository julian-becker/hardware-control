#pragma once

template <typename VISITABLE, typename VISITOR> struct
visit;

template <typename VISITOR, typename...VISITORS> struct
IVisitable : IVisitable<VISITORS...>{
    using IVisitable<VISITORS...>::accept;
    virtual typename VISITOR::visit_return_type accept(VISITOR&) = 0;
};

template <typename VISITOR> struct
IVisitable<VISITOR> {
    virtual typename VISITOR::visit_return_type accept(VISITOR&) = 0;
};
