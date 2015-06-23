#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>

struct IsInitialized : MotorCtrlVisitorBase<IsInitialized,bool> {};


template <> struct
visit<CanOpenDS402MotorCtrl,IsInitialized> {
    static bool call(CanOpenDS402MotorCtrl& mctrl, IsInitialized& visitor) {
        return true;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,IsInitialized> {
    static bool call(ElmoWhistleMotorCtrl& mctrl, IsInitialized& visitor) {
        return false;
    }
};

