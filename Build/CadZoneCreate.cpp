#include "CadZoneCreate.h"
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

std::vector<ZoneDataJson> GetDataZoneJson()
{
	std::vector<ZoneDataJson> data;
	FILE* fp = fopen("C:/TobicCadArchi/roomdata.json", "rb");
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
		ZoneDataJson dataZone = ZoneDataJson();
		dataZone.name = itr->GetObject()["name"].GetString();
		dataZone.listPoint = std::vector<API_Coord>();
		dataZone.center.x = itr->GetObject()["MiddlePoint"].FindMember("X")->value.GetDouble();
		dataZone.center.x = itr->GetObject()["MiddlePoint"].FindMember("Y")->value.GetDouble();

		for (rapidjson::Value::ConstValueIterator itr2 = itr->GetObject()["point"].Begin();
			itr2 != itr->GetObject()["point"].End(); ++itr2)
		{
			API_Coord coor1;
			coor1.x = itr2->GetObject()["x"].GetDouble();
			coor1.y = itr2->GetObject()["y"].GetDouble();
			dataZone.listPoint.push_back(coor1);
		}

		data.push_back(dataZone);
	}
	return data;

}

GS::ErrCode CreateCadZones()
{
	GS::ErrCode err = NoError;
	API_Element element;
	API_ElementMemo memo = {};
	auto data = GetDataZoneJson();
	if (data.size() == 0) return err;

	API_StoryInfo storyInfo;
	short actStory = 0;
	err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	if (err != NoError) return NoError;
	actStory = storyInfo.actStory;
	BMKillHandle((GSHandle*)&storyInfo.data);

	for (ZoneDataJson dataZone : data) 
	{
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		BNZeroMemory(&element, sizeof(API_Element));

		element.header.type.typeID = API_ZoneID;
		err = ACAPI_Element_GetDefaults(&element, &memo);
		if (err != NoError) return NoError;
		element.header.type.typeID = API_ZoneID;

		element.header.floorInd = actStory;
		element.zone.manual = true;

		API_Polygon poly;
		API_Coord** coords = nullptr;
		Int32** pends = nullptr;
		API_PolyArc** parcs = nullptr;
		double height= 1.5;
		GS::UniString roomName = dataZone.name.c_str();
		GS::Array<API_Guid> zoneListId;
		ACAPI_Element_GetElemList(API_ZoneID, &zoneListId);
		GS::UniString roomNoStr = GS::UniString::Printf("%lu", (GS::ULongForStdio)zoneListId.GetSize() + 1);
		API_Coord posText = { dataZone.center.x / 1000,dataZone.center.y / 1000 };

		API_Guid newZoneGuid = APINULLGuid;
		poly.nCoords = dataZone.listPoint.size() +1;
		poly.nSubPolys = 1;
		poly.nArcs = 0;

		coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
		pends= reinterpret_cast<Int32**> (BMAllocateHandle((poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
		parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
		if (coords && pends && parcs) 
		{
			Int32 iCoord= 1;
			for (Int32 i = 0; i < dataZone.listPoint.size(); i++) {
				(*coords)[iCoord] = {dataZone.listPoint[i].x/1000,dataZone.listPoint[i].y/1000};
				++iCoord;
			}
			(*coords)[iCoord] = (*coords)[1];
			(*pends)[1] = poly.nCoords;

			element.zone.poly = poly;

			memo.coords= reinterpret_cast<API_Coord**>		(BMAllocateHandle((poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
			memo.pends = reinterpret_cast<Int32**>			(BMAllocateHandle((poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
			memo.parcs = reinterpret_cast<API_PolyArc**>	(BMAllocateHandle(poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

			for (Int32 i = 1; i <= poly.nCoords; i++) {
				(*memo.coords)[i].x = (*coords)[i].x;
				(*memo.coords)[i].y = (*coords)[i].y;
			}
			(*memo.pends)[1] = (*pends)[1];

			GS::snuprintf(element.zone.roomName, sizeof(element.zone.roomName), roomName.ToUStr());
			GS::snuprintf(element.zone.roomNoStr, sizeof(element.zone.roomNoStr), roomNoStr.ToUStr());

			element.zone.pos= { dataZone.center.x / 1000,dataZone.center.y / 1000 };
			element.zone.roomHeight = height;
			err = ACAPI_Element_Create(&element, &memo);
			ACAPI_DisposeElemMemoHdls(&memo);

		}

	}

	return GS::NoError;
}
