#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/MoveToAbs.h>
#include <HAL/MotorCtrl/Commands/FindHome.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>
#include <HAL/MotorCtrl/Commands/RunVelocity.h>


#include <HAL/CameraCtrl/Implementation/CmosOV3642.h>
#include <HAL/CameraCtrl/Implementation/CmosOV8825.h>
#include <HAL/CameraCtrl/Commands/Initialize.h>
#include <HAL/CameraCtrl/Commands/TakePicture.h>

#include <array>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sstream>



int main(int argc, const char * argv[]) {

    /// motor tests
    std::shared_ptr<IMotorCtrl> motor1 = std::make_shared<CanOpenDS402MotorCtrl>();
    std::shared_ptr<IMotorCtrl> motor2 = std::make_shared<ElmoWhistleMotorCtrl>();
    
    std::vector<std::shared_ptr<IMotorCtrlVisitorBase>> motorCmds;
    motorCmds.push_back(std::make_shared<Motor::Initialize>());
    motorCmds.push_back(std::make_shared<Motor::FindHome>());
    motorCmds.push_back(std::make_shared<Motor::MoveToRel>(10));
    motorCmds.push_back(std::make_shared<Motor::MoveToAbs>(10));
    motorCmds.push_back(std::make_shared<Motor::RunVelocity>(1000.0));
    
    
    for(auto& cmd : motorCmds)
        motor1->accept(*cmd);

    for(auto& cmd : motorCmds)
        motor2->accept(*cmd);


    /// camera tests
    std::shared_ptr<ICameraCtrl> cam1 = std::make_shared<CmosOV3642>();
    std::shared_ptr<ICameraCtrl> cam2 = std::make_shared<CmosOV8825>();
    std::vector<std::shared_ptr<ICameraCtrlVisitorBase>> cameraCmds;
    cameraCmds.push_back(std::make_shared<Camera::Initialize>());
    cameraCmds.push_back(std::make_shared<Camera::TakePicture>());

    for(auto& cmd : cameraCmds)
        cam1->accept(*cmd);

    for(auto& cmd : cameraCmds)
        cam2->accept(*cmd);
    
}
