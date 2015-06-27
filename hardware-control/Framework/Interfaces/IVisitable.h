#pragma once
#include <Framework/Any.h>

template <typename VISITABLE, typename VISITOR> struct
visit;

template <typename VISITOR, typename...VISITORS> struct
IVisitable : IVisitable<VISITORS...>{
    template <typename VISITOR_T>
    typename VISITOR_T::visit_return_type accept(VISITOR_T& visitor) {
        return accept_untyped(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
private:
    using IVisitable<VISITORS...>::accept;
    friend struct IVisitable;
    virtual Any accept_untyped(VISITOR&) = 0;
};

template <typename VISITOR> struct
IVisitable<VISITOR> {

    template <typename VISITOR_T>
    typename VISITOR_T::visit_return_type accept(VISITOR_T& visitor) {
        return accept_untyped(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
private:
    friend struct IVisitable;
    virtual Any accept_untyped(VISITOR&) = 0;
};
