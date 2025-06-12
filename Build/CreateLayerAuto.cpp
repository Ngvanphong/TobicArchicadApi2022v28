#include "CreateLayerAuto.h"
#include "../Build/ReadExcel.h"

GS::ErrCode CreateLayerAuto()
{
	GS::ErrCode err = NoError;
	API_Attribute	attrib;
	BNZeroMemory(&attrib, sizeof(API_Attribute));
	attrib.header.typeID = API_LayerID;
	auto data = GetLayerNames();
	for (auto nameLayer : data)
	{
		strcpy(attrib.header.name, nameLayer.Name.c_str());
		err = ACAPI_Attribute_Create(&attrib, nullptr);

	}


	return NoError;
}
