#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>



struct  DataSlabJson
{
	std::vector<API_Coord> listPoint;
	std::string name;
	double zElevation;
	double thick;
};


GS::ErrCode CreateCadSlabs();

