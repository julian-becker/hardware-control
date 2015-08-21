#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

namespace Motor {

    struct MoveToRel : MotorCtrlVisitorBase<MoveToRel,int> {
        int steps;
        MoveToRel(int steps) : steps(steps) {}
    };
}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::MoveToRel> {
    static int call(CanOpenDS402MotorCtrl& mctrl, Motor::MoveToRel& visitor) {
        std::cout << "CanOpenDS402MotorCtrl: MoveToRel("<<visitor.steps<<")"<<std::endl;
        mctrl.writeDict(0x6041,0);
        return 0;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::MoveToRel> {
    static int call(ElmoWhistleMotorCtrl& mctrl, Motor::MoveToRel& visitor) {
        std::cout << "ElmoWhistleMotorCtrl: MoveToRel("<<visitor.steps<<")"<<std::endl;
        return 0;
    }
};
