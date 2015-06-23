#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

struct ControllerId : MotorCtrlVisitorBase<ControllerId,std::string> {};


template <> struct
visit<CanOpenDS402MotorCtrl,ControllerId> {
    static std::string call(CanOpenDS402MotorCtrl& mctrl, ControllerId& visitor) {
        return "CanOpenDS402MotorCtrl";
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,ControllerId> {
    static std::string call(ElmoWhistleMotorCtrl& mctrl, ControllerId& visitor) {
        return "ElmoWhistleMotorCtrl";
    }
};

