#pragma once
#include "IVisitor.h"
#include "Visitor.h"


struct CanOpenDS402MotorCtrl;
struct ElmoWhistleMotorCtrl;


struct IMotorCtrlVisitorBase : IVisitorBase<CanOpenDS402MotorCtrl,ElmoWhistleMotorCtrl> {};


template <typename CLASS, typename RETURN_TYPE> using
MotorCtrlVisitorBase = Visitor<CLASS,RETURN_TYPE,IMotorCtrlVisitorBase,CanOpenDS402MotorCtrl,ElmoWhistleMotorCtrl>;
