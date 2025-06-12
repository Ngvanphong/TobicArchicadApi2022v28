#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct DataColumnJson
{
	API_Coord point;
	std::string name;
	double zElevation;
};

GS::ErrCode CreateCadColumns();

