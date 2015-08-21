#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

namespace Motor {
    struct MoveToAbs : MotorCtrlVisitorBase<MoveToAbs,int> {
        int steps;
        MoveToAbs(int steps) : steps(steps) {}
    };

}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::MoveToAbs> {
    static int call(CanOpenDS402MotorCtrl& mctrl, Motor::MoveToAbs& visitor) {
        std::cout << "CanOpenDS402MotorCtrl: MoveToAbs("<<visitor.steps<<")"<<std::endl;
        mctrl.writeDict(0x6041,0);
        return 0;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::MoveToAbs> {
    static int call(ElmoWhistleMotorCtrl& mctrl, Motor::MoveToAbs& visitor) {
        std::cout << "ElmoWhistleMotorCtrl: MoveToAbs("<<visitor.steps<<")"<<std::endl;
        return 0;
    }
};
