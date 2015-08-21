#pragma once
#include <HAL/CameraCtrl/Interfaces/ICameraCtrlVisitors.h>

namespace Camera {

    struct Initialize : CameraCtrlVisitorBase<Initialize,bool> {};

}

template <> struct
visit<CmosOV8825,Camera::Initialize> {
    static bool call(CmosOV8825& cctrl, Camera::Initialize& visitor) {
        std::cout << "CmosOV8825: Initialize()"<<std::endl;
        return true;
    }
};

template <> struct
visit<CmosOV3642,Camera::Initialize> {
    static bool call(CmosOV3642& cctrl, Camera::Initialize& visitor) {
        std::cout << "CmosOV3642: Initialize()"<<std::endl;
        return false;
    }
};

