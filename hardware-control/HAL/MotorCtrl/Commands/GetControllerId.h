#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

namespace Motor {

    struct ControllerId : MotorCtrlVisitorBase<ControllerId,std::string> {
        ~ControllerId() throw() {}
    };

}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::ControllerId> {
    static std::string call(CanOpenDS402MotorCtrl& mctrl, Motor::ControllerId& visitor) {
        return "CanOpenDS402MotorCtrl";
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::ControllerId> {
    static std::string call(ElmoWhistleMotorCtrl& mctrl, Motor::ControllerId& visitor) {
        return "ElmoWhistleMotorCtrl";
    }
};

