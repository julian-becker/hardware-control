#pragma once


template <typename R, typename T, typename...TS> struct
IVisitor : IVisitor<R,TS...> {
    using visit_return_type = R;
    using IVisitor<R,TS...>::visit;
    virtual R visit(T&) = 0;
};

template <typename R, typename T> struct
IVisitor<R,T> {
    using visit_return_type = R;
    virtual R visit(T&) = 0;
};
