#pragma once

#include "ACAPinc.h"
#include "APIEnvir.h"
#include <list>
#include <vector>

struct WallTypeData
{
	std::string Name;
	std::string Top2Name;
	double Top2 = 0;
	std::string Top1Name;
	double Top1 = 0;
	std::string CoreName;
	double Core = 0;
	std::string Bottom1Name;
	double Bottom1 = 0;
	std::string Bottom2Name;
	double Bottom2 = 0;
};

GS::ErrCode SeperateWallData(void);
