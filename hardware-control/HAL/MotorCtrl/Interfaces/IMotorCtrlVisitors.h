#pragma once
#include "IVisitor.h"
#include "Visitor.h"


struct CanOpenDS402MotorCtrl;
struct ElmoWhistleMotorCtrl;


template <typename RETURN_TYPE> using
IMotorCtrlVisitor = IVisitor<RETURN_TYPE,CanOpenDS402MotorCtrl,ElmoWhistleMotorCtrl>;


template <typename CLASS, typename RETURN_TYPE> using
MotorCtrlVisitorBase = Visitor<CLASS,RETURN_TYPE,CanOpenDS402MotorCtrl,ElmoWhistleMotorCtrl>;
