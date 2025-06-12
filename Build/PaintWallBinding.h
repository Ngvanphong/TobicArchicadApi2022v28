#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct GuidVolumePaint
{
	API_Guid guid;
	double volumne;
};

GS::ErrCode PaintWallByHabaki(void);

