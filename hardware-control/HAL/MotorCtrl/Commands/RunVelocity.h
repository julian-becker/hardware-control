#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

namespace Motor {

    struct RunVelocity : MotorCtrlVisitorBase<RunVelocity,void> {
        const double velocity;
        RunVelocity(double velocity) : velocity(velocity) {}
    };

}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::RunVelocity> {
    static int call(CanOpenDS402MotorCtrl& mctrl, Motor::RunVelocity& visitor) {
        std::cout << "CanOpenDS402MotorCtrl: RunVelocity("<<visitor.velocity<<")"<<std::endl;
        mctrl.writeDict(0x6041,0);
        return 0;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::RunVelocity> {
    static int call(ElmoWhistleMotorCtrl& mctrl, Motor::RunVelocity& visitor) {
        std::cout << "ElmoWhistleMotorCtrl: RunVelocity("<<visitor.velocity<<")"<<std::endl;
        return 0;
    }
};
