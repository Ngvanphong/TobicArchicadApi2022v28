#include "WallCad.h"
#include "rapidjson.h"
#include <iostream>
#include <fstream>
#include <filewritestream.h>
#include <writer.h>
#include <document.h>
#include <locale>
#include <codecvt>
#include <string>
#include <filereadstream.h>

std::vector<DataWallJson> GetDataWallJson()
{
	std::vector<DataWallJson> data;
	FILE* fp = fopen("C:/TobicCadArchi/walldata.json", "rb");
	if (!fp) return data;
	// Read the file into a buffer
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document doc;
	doc.ParseStream(is);
	if (doc.HasParseError()) return data;
	fclose(fp);

	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr)
	{
		DataWallJson dataWall = DataWallJson();
		dataWall.name = itr->GetObject()["name"].GetString();
		dataWall.zElevation = itr->GetObject()["zelevation"].GetDouble();
		dataWall.thick= itr->GetObject()["thick"].GetDouble();
		dataWall.startPoint.x = itr->GetObject()["startpoint"].FindMember("x")->value.GetDouble();
		dataWall.startPoint.y = itr->GetObject()["startpoint"].FindMember("y")->value.GetDouble();
		dataWall.endPoint.x = itr->GetObject()["endpoint"].FindMember("x")->value.GetDouble();
		dataWall.endPoint.y = itr->GetObject()["endpoint"].FindMember("y")->value.GetDouble();
		data.push_back(dataWall);
	}
	return data;

}

GS::ErrCode CreateCadWalls()
{
	GS::ErrCode err = NoError;

	API_Coord startPos, endPos;
	API_Element element = {};
	API_ElementMemo memo = {};
	API_Element mask;

	auto data = GetDataWallJson();
	if (data.size() == 0) return err;

	// for
	for (DataWallJson dataWall : data) 
	{
		startPos = { dataWall.startPoint.x / 1000,dataWall.startPoint.y / 1000 };
		endPos = { dataWall.endPoint.x / 1000,dataWall.endPoint.y / 1000};
		auto name = dataWall.name;
		BNClear(element);
		BNClear(memo);
		element.header.type.typeID = API_WallID;
		ACAPI_Element_GetDefaults(&element, &memo);
		element.wall.begC = startPos;
		element.wall.endC = endPos;
		element.wall.height = 3.0;
		element.wall.bottomOffset = dataWall.zElevation/1000;
		element.wall.thickness = dataWall.thick/1000;

		err = ACAPI_Element_Create(&element, &memo);
		if (err == NoError) {
			
			ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_ElemInfoString);
			if (memo.elemInfoString != nullptr) 
			{
					delete memo.elemInfoString;
			}
			memo.elemInfoString = new GS::UniString(name.c_str());

			ACAPI_ELEMENT_MASK_CLEAR(mask);
			err = ACAPI_Element_ChangeParameters({ element.header.guid }, &element, &memo, &mask);
		}
	}
	
	ACAPI_DisposeElemMemoHdls(&memo);

	return NoError;
}
