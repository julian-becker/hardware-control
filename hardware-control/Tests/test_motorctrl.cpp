#include "../catch.h"

#include <iostream>
#include <HAL/MotorCtrl/Implementation/CanOpenDS402MotorCtrl.h>
#include <HAL/MotorCtrl/Implementation/ElmoWhistleMotorCtrl.h>
#include <HAL/MotorCtrl/Commands/GetControllerId.h>
#include <HAL/MotorCtrl/Commands/MoveToRel.h>
#include <HAL/MotorCtrl/Commands/MoveToAbs.h>
#include <HAL/MotorCtrl/Commands/FindHome.h>
#include <HAL/MotorCtrl/Commands/Initialize.h>
#include <HAL/MotorCtrl/Commands/RunVelocity.h>


#include <HAL/CameraCtrl/Implementation/CmosOV3642.h>
#include <HAL/CameraCtrl/Implementation/CmosOV8825.h>
#include <HAL/CameraCtrl/Commands/Initialize.h>
#include <HAL/CameraCtrl/Commands/TakePicture.h>

#include <array>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sstream>


enum class COMPONENT {
    Camera,
    MirrorMotor,
    PanMotor,
    FiberAmplifier,
};

template <COMPONENT comp, typename VAL> struct
configuration {
    using type = VAL;
};

template <typename...> struct
list;

template <size_t I, typename LIST> struct
type_at;

template <size_t I, template <typename...> class LIST_T, typename T, typename...TS> struct
type_at<I,LIST_T<T,TS...>>
{
    using type = typename type_at<I-1,LIST_T<TS...>>::type;
};

template <template <typename...> class LIST_T, typename T, typename...TS> struct
type_at<0,LIST_T<T,TS...>>
{
    using type = T;
};

template <size_t I, typename T> using
type_at_t = typename type_at<I,T>::type;




template <COMPONENT attr, typename CONTAINER> struct
index_of;

template <COMPONENT attr, template <typename...> class LIST_T, typename T, typename...TS> struct
index_of<attr,LIST_T<configuration<attr,T>,TS...>> {
    static constexpr size_t value = 0;
};

template <COMPONENT attr, template <typename...> class LIST_T, typename T, typename...TS> struct
index_of<attr,LIST_T<T,TS...>> {
    static constexpr size_t value = 1 + index_of<attr,LIST_T<TS...>>::value;
};


template <COMPONENT attr, typename CONTAINER> using
get_component_t = typename type_at_t<index_of<attr, CONTAINER>::value,CONTAINER>::type;


using hardware =
    list<
        configuration<COMPONENT::Camera, CmosOV8825>,
        configuration<COMPONENT::PanMotor, CanOpenDS402MotorCtrl>,
        configuration<COMPONENT::MirrorMotor, ElmoWhistleMotorCtrl>
    >;



void muiaeain(int argc, const char * argv[]) {

    /// motor tests
    using MirrorType = get_component_t<COMPONENT::MirrorMotor,hardware>;
    using PanType = get_component_t<COMPONENT::PanMotor,hardware>;

    std::shared_ptr<IMotorCtrl> mirrorMCtrl = std::make_shared<MirrorType>();
    std::shared_ptr<IMotorCtrl> panMCtrl = std::make_shared<PanType>();
    
    std::vector<std::shared_ptr<IMotorCtrlVisitorBase>> motorCmds;
    motorCmds.push_back(std::make_shared<Motor::Initialize>());
    motorCmds.push_back(std::make_shared<Motor::FindHome>());
    motorCmds.push_back(std::make_shared<Motor::MoveToRel>(10));
    motorCmds.push_back(std::make_shared<Motor::MoveToAbs>(10));
    motorCmds.push_back(std::make_shared<Motor::RunVelocity>(1000.0));
    
    
    for(auto& cmd : motorCmds)
        mirrorMCtrl->accept(*cmd);

    for(auto& cmd : motorCmds)
        panMCtrl->accept(*cmd);


    /// camera tests
    std::shared_ptr<ICameraCtrl> cam1 = std::make_shared<CmosOV3642>();
    std::shared_ptr<ICameraCtrl> cam2 = std::make_shared<CmosOV8825>();
    std::vector<std::shared_ptr<ICameraCtrlVisitorBase>> cameraCmds;
    cameraCmds.push_back(std::make_shared<Camera::Initialize>());
    cameraCmds.push_back(std::make_shared<Camera::TakePicture>());

    for(auto& cmd : cameraCmds)
        cam1->accept(*cmd);

    for(auto& cmd : cameraCmds)
        cam2->accept(*cmd);
}
