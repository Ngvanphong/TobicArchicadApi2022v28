#pragma once
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct LineWallData
{
	API_Coord start;
	API_Coord end;

	double thickFloorBottom;
	double thickFloorTop;

	API_StoryType storyBottom;
	API_StoryType storyTop;

	double offsetBottom;
	double offsetTop;

	API_Vector directionMove;

};

GS::ErrCode CreateWallFromToCeiling();

