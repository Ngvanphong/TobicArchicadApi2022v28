#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct PointFa
{
	double X;
	double Y;
	double Z;
};


GS::ErrCode PutFamilyByPoint(void);

