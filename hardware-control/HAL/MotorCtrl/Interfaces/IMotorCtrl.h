#pragma once
#include "Visitable.h"
#include "IMotorCtrlVisitors.h"

using IMotorCtrl = IVisitable<
                        IMotorCtrlVisitorBase         // list all accepted visitor return types here
                   >;


template <typename CLASS> using
MotorCtrlBase = Visitable<CLASS,IMotorCtrl>;