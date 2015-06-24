#pragma once


template <typename R, typename VISITABLE, typename...VISITABLES> struct
IVisitor : IVisitor<R,VISITABLES...> {
    using visit_return_type = R;
    using IVisitor<R,VISITABLES...>::visit;
    virtual R visit(VISITABLE&) = 0;
};

template <typename R, typename VISITABLE> struct
IVisitor<R,VISITABLE> {
    using visit_return_type = R;
    virtual R visit(VISITABLE&) = 0;
};
