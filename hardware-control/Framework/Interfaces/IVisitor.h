#pragma once
#include <Framework/Any.h>


/// @brief: Interface for visitors.
/// @tparam VISITABLE: a target class that this visitor can visit.
template <typename VISITABLE, typename...VISITABLES> struct
IVisitor : IVisitor<VISITABLES...> {
    using visitor_interface = IVisitor;
    using IVisitor<VISITABLES...>::visit;
    virtual Any visit(VISITABLE&) = 0;    
};

/// specialization for single visitable target
template <typename VISITABLE> struct
IVisitor<VISITABLE> {
    using visitor_interface = IVisitor;
    virtual Any visit(VISITABLE&) = 0;
};

