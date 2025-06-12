#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct GuidVolume
{
	API_Guid guid;
	double volumne;
};

GS::ErrCode CreateWallBase(void);

GS::ErrCode CreateWallBaseByZone(void);

