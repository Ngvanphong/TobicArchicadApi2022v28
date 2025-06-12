#pragma once
#include "ACAPinc.h"
struct DataProfile
{
	std::string Name;
	double Width;
	double Height;
	bool IsColumn;
};

struct  DataProfileH
{
	std::string Name;
	double Width;
	double Height;
	double tw;
	double tf;
	double IsColumn;
};

struct DataProfileRecSteel {
	std::string Name;
	double Width;
	double Height;
	double Thickness;
};

struct DataProfileMashi {
	std::string Name;
	double Top;
	double Bottom;
	double Left;
	double Right;
};


struct LayerName {
	std::string Name;
	
};
