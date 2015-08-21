#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

namespace Motor {
    struct FindHome : MotorCtrlVisitorBase<FindHome,int> {};
}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::FindHome> {
    static int call(CanOpenDS402MotorCtrl& mctrl, Motor::FindHome& visitor) {
        std::cout << "CanOpenDS402MotorCtrl: FindHome()"<<std::endl;
        return 0;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::FindHome> {
    static int call(ElmoWhistleMotorCtrl& mctrl, Motor::FindHome& visitor) {
        std::cout << "ElmoWhistleMotorCtrl: FindHome()"<<std::endl;
        return 0;
    }
};
