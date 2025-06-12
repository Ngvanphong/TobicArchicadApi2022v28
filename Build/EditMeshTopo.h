#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct DataPolyMeshEdit
{
	double X;
	double Y;
	double Z;
};

GS::ErrCode EditTopoByPolyline(void);