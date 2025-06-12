#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct PointAddMesh 
{
	double X;
	double Y;
	double Z;
};

GS::ErrCode AddPointToMesh(void);

