#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h""
#include <vector>
#include "PickPoint.h"
#include "DimensionUtilities.h"
#include <cstdlib>

GSErrCode CreateTotalDimBeam();

GSErrCode CreateDimBeamToLine(double);

GSErrCode CreateDimLayerBeam(double);
