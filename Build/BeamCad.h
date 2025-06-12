#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct DataBeamJson
{
	API_Coord startPoint;
	API_Coord endPoint;
	double zElevation;
	std::string name;
};

GS::ErrCode CreateBeams();

