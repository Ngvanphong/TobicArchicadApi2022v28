#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct ZoneDataJson
{
	std::string name;
	API_Coord center;
	std::vector<API_Coord> listPoint;

};

GS::ErrCode CreateCadZones();

