#pragma once
#include <Framework/Any.h>

template <typename VISITABLE, typename VISITOR> struct
visit;

template <typename VISITOR, typename...VISITORS> struct
IVisitable : IVisitable<VISITORS...>{
    using IVisitable<VISITORS...>::accept;
    virtual Any accept(VISITOR&) = 0;
    
    template <typename VISITOR_T>
    typename VISITOR_T::visit_return_type accept_typed(VISITOR_T& visitor) {
        return accept(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
};

template <typename VISITOR> struct
IVisitable<VISITOR> {
    virtual Any accept(VISITOR&) = 0;

    template <typename VISITOR_T>
    typename VISITOR_T::visit_return_type accept_typed(VISITOR_T& visitor) {
        return accept(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
};
