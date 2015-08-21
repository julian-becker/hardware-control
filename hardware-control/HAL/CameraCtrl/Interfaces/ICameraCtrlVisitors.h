#pragma once
#include "IVisitor.h"
#include "Visitor.h"


struct CmosOV3642;;
struct CmosOV8825;


struct ICameraCtrlVisitorBase : IVisitor<CmosOV3642,CmosOV8825> {};


template <typename CLASS, typename RETURN_TYPE> using
CameraCtrlVisitorBase = Visitor<CLASS,RETURN_TYPE,ICameraCtrlVisitorBase,CmosOV3642,CmosOV8825>;
