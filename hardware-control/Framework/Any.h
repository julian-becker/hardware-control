#pragma once

class Any {
    struct holder {
        virtual holder* clone() const = 0;
        virtual ~holder() throw() {}
    };
    
    template <typename T> struct
    typed final : holder {
        T val;
        typed(T&& val) : val(std::move(val)) {}
        typed(const T& val) : val(val) {}
        holder* clone() const final override { T valCopy(val); return new typed(std::move(valCopy)); }
    };
    
    holder* val;
public:
    Any() : val(nullptr) {}
    
    template <typename T, typename=std::enable_if_t<!std::is_same<Any,std::decay_t<T>>::value>>
    Any(T&& val) : val(new typed<std::decay_t<T>>(std::forward<T>(val))) {}
    
    Any(const Any& o) : val(o.val? o.val->clone() : nullptr) {}
    
    Any(Any&& o) : val(std::move(o.val)) { o.val = nullptr; }
    
    Any& operator = (const Any& o) {
        if(&o != this) {
            Any copy(o);
            *this = std::move(copy);
        }
        return *this;
    }
    
    Any& operator = (Any&& o) {
        if(&o != this) {
            val = std::move(o.val);
            o.val = nullptr;
        }
        return *this;
    }
    
    ~Any() throw() { delete val; }
    
    template <typename T> T&
    as() { return static_cast<typed<T>*>(val)->val; }
    
};

