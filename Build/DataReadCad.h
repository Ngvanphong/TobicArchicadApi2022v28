#pragma once
#include<string>
#include "ACAPinc.h"
#include <vector>


struct GridData
{
	API_Coord3D start;

	API_Coord3D end;

	std::string lable;
};

struct DataCad
{
	std::wstring  path;

	API_Coord location;

	double angle;


};


