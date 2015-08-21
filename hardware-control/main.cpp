#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>

#include <array>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sstream>




int main(int argc, const char * argv[]) {

    std::shared_ptr<IMotorCtrl> motor1 = std::make_shared<CanOpenDS402MotorCtrl>();
    std::shared_ptr<IMotorCtrl> motor2 = std::make_shared<ElmoWhistleMotorCtrl>();
    
    ControllerId id;
    std::vector<std::shared_ptr<IMotorCtrlVisitorBase>> cmds;
    cmds.push_back(std::make_shared<ControllerId>());
    cmds.push_back(std::make_shared<MoveToRel>(10));
    
    
    for(auto& cmd : cmds)
        motor2->accept(*cmd);
}
