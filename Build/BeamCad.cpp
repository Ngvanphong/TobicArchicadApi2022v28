#include "BeamCad.h"
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


std::vector<DataBeamJson> GetDataBeamJson()
{
	std::vector<DataBeamJson> data;
	FILE* fp = fopen("C:/TobicCadArchi/beamdata.json", "rb");
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
		DataBeamJson dataBeam = DataBeamJson();
		dataBeam.name = itr->GetObject()["name"].GetString();
		dataBeam.zElevation = itr->GetObject()["zelevation"].GetDouble();
		dataBeam.startPoint.x = itr->GetObject()["startpoint"].FindMember("x")->value.GetDouble();
		dataBeam.startPoint.y = itr->GetObject()["startpoint"].FindMember("y")->value.GetDouble();
		dataBeam.endPoint.x = itr->GetObject()["endpoint"].FindMember("x")->value.GetDouble();
		dataBeam.endPoint.y = itr->GetObject()["endpoint"].FindMember("y")->value.GetDouble();
		data.push_back(dataBeam);
	}
	return data;

}


GS::ErrCode CreateBeams()
{
	GS::ErrCode err = NoError;

	auto data = GetDataBeamJson();
	if (data.size() == 0) return err;

	API_Coord startPos, endPos;
	API_Element element = {};
	API_ElementMemo memo = {};
	API_Element	mask;

	// for each start point and end point from json
	for (DataBeamJson dataBeam : data) 
	{
		startPos = { dataBeam.startPoint.x / 1000,dataBeam.startPoint.y / 1000 };
		endPos = { dataBeam.endPoint.x / 1000,dataBeam.endPoint.y / 1000 };
		auto name = dataBeam.name;

		BNClear(element);
		BNClear(memo);
		element.header.type.typeID = API_BeamID;
		ACAPI_Element_GetDefaults(&element, &memo);
		element.beam.begC = startPos;
		element.beam.endC = endPos;
		element.beam.level = dataBeam.zElevation/1000;

		err = ACAPI_Element_Create(&element, &memo);
		if (err == NoError) {
			ACAPI_ELEMENT_MASK_CLEAR(mask);
			ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_ElemInfoString);
			if (memo.elemInfoString != nullptr)
			{
				delete memo.elemInfoString;
			}
			memo.elemInfoString = new GS::UniString(name.c_str());
			err = ACAPI_Element_ChangeParameters({ element.header.guid }, &element, &memo, &mask);
		}
	}

	ACAPI_DisposeElemMemoHdls(&memo);

	return NoError;
}
