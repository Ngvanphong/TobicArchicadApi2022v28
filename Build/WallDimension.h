#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "ResourceIds.hpp"
#include "File.hpp"
#include "uchar_t.hpp"
#include <OnExit.hpp>
#include <cstdlib>
#include <list>
#include "PickPoint.h"
#include "DimensionUtilities.h"

GSErrCode CreateDimensionTotalWall(void);

GSErrCode CreateDimensionCenterWall(double);

GSErrCode CreateDimensionLayerWall(double);




