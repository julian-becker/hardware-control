#pragma once
#include <HAL/CameraCtrl/Interfaces/ICameraCtrl.h>

struct CmosOV3642 : CameraCtrlBase<CmosOV3642> {
    void writeRegister(size_t addr, int value) {
        std::cout << "CmosOV3642: writing value " << value << " to register addr " << addr << std::endl;
    }
};
