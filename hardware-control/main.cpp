#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>

#include <array>
#include <vector>

int main(int argc, const char * argv[])
{

    std::shared_ptr<IMotorCtrl> motor1 = std::make_shared<CanOpenDS402MotorCtrl>();
    std::shared_ptr<IMotorCtrl> motor2 = std::make_shared<ElmoWhistleMotorCtrl>();

    MoveToRel move(10);
    motor1->accept(move);
    
    ControllerId id;
    std::cout << "ControllerId="<<motor2->accept(id)<<std::endl;
//    std::cout << "ControllerId="<<motor1->accept(id).as<std::string>()<<std::endl;
    
    IsInitialized init;
//    std::cout << "IsInitialized1="<<motor1->accept(init).as<bool>()<<std::endl;
//    std::cout << "IsInitialized2="<<motor2->accept(init).as<bool>()<<std::endl;
    
    
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<char> ar{0,1,2,3,4};
    std::vector<char> a2{0,1,2,3,4};
    Any a1(ar);
    Any a3(a2);
    for(size_t i=0; i<10000000; i++) {
        a1 = std::move(a3);
        a3 = std::move(a1);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    ar = a2;
    std::cout << "duration="<< std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()<<std::endl;
    std::cout << "vector = "<<sizeof(std::vector<char>)<<", any="<<sizeof(Any)<<std::endl;
}
