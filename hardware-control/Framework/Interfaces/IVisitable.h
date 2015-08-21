#pragma once
#include <Framework/Any.h>

/// declaration of the template that has to be specialized for each visitable/visitor pair
template <typename VISITABLE, typename VISITOR> struct
visit;



template <typename T> struct
has_visit_return_type
{
    typedef char yes[1];
    typedef char no[2];

    template <typename C> static yes& test(typename C::visit_return_type*);
    template <typename> static no& test(...);

    static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes);
};


template <typename VISITOR, typename...VISITORS> struct
IVisitable : IVisitable<VISITORS...>{

    template <typename VISITOR_T>
    std::enable_if_t<has_visit_return_type<VISITOR_T>::value,typename VISITOR_T::visit_return_type> accept(VISITOR_T& visitor) {
        return accept_untyped(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
    
    template <typename VISITOR_T>
    std::enable_if_t<!has_visit_return_type<VISITOR_T>::value,void> accept(VISITOR_T& visitor) {
        accept_untyped(visitor);
    }
    
private:
    using IVisitable<VISITORS...>::accept;
    friend struct IVisitable;
    virtual Any accept_untyped(VISITOR&) = 0;
};





template <typename VISITOR> struct
IVisitable<VISITOR> {

    template <typename VISITOR_T>
    std::enable_if_t<has_visit_return_type<VISITOR_T>::value,typename VISITOR_T::visit_return_type> accept(VISITOR_T& visitor) {
        return accept_untyped(visitor).template as<typename VISITOR_T::visit_return_type>();
    }
    
    template <typename VISITOR_T>
    std::enable_if_t<!has_visit_return_type<VISITOR_T>::value,void> accept(VISITOR_T& visitor) {
        accept_untyped(visitor);
    }
    
private:
    friend struct IVisitable;
    virtual Any accept_untyped(VISITOR&) = 0;
};
