#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct DataWallJson
{
	API_Coord startPoint;
	API_Coord endPoint;
	double zElevation;
	std::string name;
	double thick;
};

GS::ErrCode CreateCadWalls();

