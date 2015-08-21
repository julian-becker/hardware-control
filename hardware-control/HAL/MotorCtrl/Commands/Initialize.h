#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrlVisitors.h>


namespace Motor {

    struct IsInitialized : MotorCtrlVisitorBase<IsInitialized,bool> {};
    struct Initialize : MotorCtrlVisitorBase<Initialize,bool> {};

}

template <> struct
visit<CanOpenDS402MotorCtrl,Motor::IsInitialized> {
    static bool call(CanOpenDS402MotorCtrl& mctrl, Motor::IsInitialized& visitor) {
        return true;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::IsInitialized> {
    static bool call(ElmoWhistleMotorCtrl& mctrl, Motor::IsInitialized& visitor) {
        return false;
    }
};





template <> struct
visit<CanOpenDS402MotorCtrl,Motor::Initialize> {
    static bool call(CanOpenDS402MotorCtrl& mctrl, Motor::Initialize& visitor) {
        std::cout << "CanOpenDS402MotorCtrl: Initialize()"<<std::endl;
        return true;
    }
};

template <> struct
visit<ElmoWhistleMotorCtrl,Motor::Initialize> {
    static bool call(ElmoWhistleMotorCtrl& mctrl, Motor::Initialize& visitor) {
        std::cout << "ElmoWhistleMotorCtrl: Initialize()"<<std::endl;
        return false;
    }
};

