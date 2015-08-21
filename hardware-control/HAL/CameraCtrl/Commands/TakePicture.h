#pragma once
#include <HAL/CameraCtrl/Interfaces/ICameraCtrlVisitors.h>

struct picture {};

namespace Camera {

    struct TakePicture : CameraCtrlVisitorBase<TakePicture,picture> {};

}

template <> struct
visit<CmosOV8825,Camera::TakePicture> {
    static picture call(CmosOV8825& cctrl, Camera::TakePicture& visitor) {
        std::cout << "CmosOV8825: TakePicture()"<<std::endl;
        return picture{};
    }
};


template <> struct
visit<CmosOV3642,Camera::TakePicture> {
    static picture call(CmosOV3642& cctrl, Camera::TakePicture& visitor) {
        std::cout << "CmosOV3642: TakePicture()"<<std::endl;
        return picture{};
    }
};

