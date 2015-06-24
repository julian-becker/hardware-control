#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>





int main(int argc, const char * argv[])
{

    std::shared_ptr<IMotorCtrl> motor1 = std::make_shared<CanOpenDS402MotorCtrl>();
    std::shared_ptr<IMotorCtrl> motor2 = std::make_shared<ElmoWhistleMotorCtrl>();

    MoveToRel move(10);
    motor1->accept_typed(move);
    
    ControllerId id;
    std::cout << "ControllerId="<<motor1->accept(id).as<std::string>()<<std::endl;
    std::cout << "ControllerId="<<motor1->accept_typed(id)<<std::endl;
    
    IsInitialized init;
    std::cout << "IsInitialized1="<<motor1->accept(init).as<bool>()<<std::endl;
    std::cout << "IsInitialized2="<<motor2->accept(init).as<bool>()<<std::endl;
}
