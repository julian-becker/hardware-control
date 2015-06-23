#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>











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






int main(int argc, const char * argv[])
{

    std::shared_ptr<IMotorCtrl> motor1 = std::make_shared<CanOpenDS402MotorCtrl>();
    std::shared_ptr<IMotorCtrl> motor2 = std::make_shared<ElmoWhistleMotorCtrl>();

    MoveToRel move(10);
    motor1->accept(move);
    
    ControllerId id;
    std::cout << "ControllerId="<<motor1->accept(id)<<std::endl;
    
    IsInitialized init;
    std::cout << "IsInitialized1="<<motor1->accept(init)<<std::endl;
    std::cout << "IsInitialized2="<<motor2->accept(init)<<std::endl;
}
