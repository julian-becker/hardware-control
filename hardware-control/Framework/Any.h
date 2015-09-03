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
    as() {
        return static_cast<typed<T>*>(val)->val;
    }

    template <typename T> T&
    as_s() {
        if(!dynamic_cast<typed<T>*>(val))
            throw std::bad_cast();
        return static_cast<typed<T>*>(val)->val;
    }
    
};
/*

struct abstract_storage {
    virtual ~abstract_storage() throw() {}
    virtual void* storage() = 0;
    virtual void cleanup() = 0;
    virtual void clone(void*) const = 0;
    virtual void move(void*) = 0;
    template <typename T> T& as() { return *static_cast<T*>(storage()); }
    
};

template <typename T, typename = void> struct
optimized_storage;

template <> struct
optimized_storage<void> final : abstract_storage {
    optimized_storage() {}
    virtual void cleanup() override {};
    virtual void* storage() override { return nullptr; }
    virtual void clone(void* storage) const override {
        new (storage) optimized_storage();
    }
    virtual void move(void* storage) override {
        new (storage) optimized_storage();
    }
};

template <typename T> struct
optimized_storage<T,std::enable_if_t<(sizeof(T)<=sizeof(std::max_align_t))>> final : abstract_storage {
    T val;
    optimized_storage(T&& val) : val(std::move(val)) {}
    optimized_storage(const T& val) : val(val) {}
    virtual void cleanup() override {};
    virtual void* storage() override { return &val; }
    virtual void clone(void* storage) const override {
        new (storage) optimized_storage(val);
    }
    virtual void move(void* storage) override {
        new (storage) optimized_storage(std::move(val));
    }
};

template <typename T> struct
optimized_storage<T,std::enable_if_t<(sizeof(T)>sizeof(std::max_align_t))>> final : abstract_storage {
    T* val;
    optimized_storage(T&& val) : val(new T(std::move(val))) {}
    optimized_storage(const T& val) : val(new T(val)) {}
    virtual void cleanup() override { delete val; };
    virtual void* storage() override { return val; }
    virtual void clone(void* storage) const override {
        new (storage) optimized_storage(*val);
    }
    virtual void move(void* storage) override {
        new (storage) optimized_storage(std::move(*val));
        val = nullptr;
    }
};

class Any {
    std::aligned_storage<24> store;
    
    void* storage() { return &store; }
    const void* storage() const { return &store; }
    
    
public:
    Any() {
        new (storage()) optimized_storage<void>();
    }

    template <typename T, typename=std::enable_if_t<!std::is_same<Any,std::decay_t<T>>::value>>
    Any(T&& val) {
        new (storage()) optimized_storage<std::decay_t<T>>(std::forward<T>(val));
    }
    
    Any(const Any& o) {
        static_cast<const abstract_storage*>(o.storage())->clone(storage());
    }
    
    Any(Any&& o) {
        static_cast<abstract_storage*>(o.storage())->move(storage());
    }
    
    Any& operator = (const Any& o) {
        if(&o != this) {
            Any copy(o);
            *this = std::move(copy);
        }
        return *this;
    }
    
    Any& operator = (Any&& o) {
        if(&o != this) {
            static_cast<abstract_storage*>(storage())->cleanup();
            static_cast<abstract_storage*>(o.storage())->move(storage());
        }
        return *this;
    }
    
    ~Any() throw() { static_cast<abstract_storage*>(storage())->cleanup(); }
    
    template <typename T> T&
    as() { return static_cast<abstract_storage*>(storage())->as<T>(); }
};
*/