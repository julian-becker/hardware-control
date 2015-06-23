#pragma once
#include "Visitable.h"
#include "IMotorCtrlVisitors.h"

using IMotorCtrl = IVisitable<
                        IMotorCtrlVisitor<bool>,         // list all accepted visitor return types here
                        IMotorCtrlVisitor<char>,         // ...
                        IMotorCtrlVisitor<int>,
                        IMotorCtrlVisitor<std::string>
                   >;


template <typename CLASS> using
MotorCtrlBase = Visitable<CLASS,IMotorCtrl>;