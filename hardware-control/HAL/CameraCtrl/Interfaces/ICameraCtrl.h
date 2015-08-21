#pragma once
#include "Visitable.h"
#include "ICameraCtrlVisitors.h"

using ICameraCtrl = IVisitable<
                        ICameraCtrlVisitorBase
                    >;


template <typename CLASS> using
CameraCtrlBase = Visitable<CLASS,ICameraCtrl>;