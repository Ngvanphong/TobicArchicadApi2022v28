#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct EleIndex
{
	short Index;
	double Elevation;
	GS::UniString Name;
};

GS::ErrCode  ImportLevelFile(void);