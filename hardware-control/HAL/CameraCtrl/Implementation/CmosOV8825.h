#pragma once
#include <HAL/CameraCtrl/Interfaces/ICameraCtrl.h>

struct CmosOV8825 : CameraCtrlBase<CmosOV8825> {
    void writeRegister(size_t addr, int value) {
        std::cout << "CmosOV8825: writing value " << value << " to register addr " << addr << std::endl;
    }
};
