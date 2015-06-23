#pragma once
#include <HAL/MotorCtrl/Interfaces/IMotorCtrl.h>


struct CanOpenDS402MotorCtrl : MotorCtrlBase<CanOpenDS402MotorCtrl> {
    void writeDict(int,int) {
    }
};
