#include "CadSlab.h"
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


std::vector<DataSlabJson> GetDataSlabJson()
{
	std::vector<DataSlabJson> data;
	FILE* fp = fopen("C:/TobicCadArchi/slabdata.json", "rb");
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
		DataSlabJson dataBeam = DataSlabJson();
		dataBeam.name = itr->GetObject()["name"].GetString();
		dataBeam.zElevation = itr->GetObject()["zelevation"].GetDouble();
		dataBeam.thick = itr->GetObject()["thick"].GetDouble();
		dataBeam.listPoint = std::vector<API_Coord>();
		for (rapidjson::Value::ConstValueIterator itr2 = itr->GetObject()["point"].Begin();
			itr2 != itr->GetObject()["point"].End(); ++itr2)
		{
			API_Coord coor1;
			coor1.x = itr2->GetObject()["x"].GetDouble();
			coor1.y = itr2->GetObject()["y"].GetDouble();
			dataBeam.listPoint.push_back(coor1);
		}

		data.push_back(dataBeam);
	}
	return data;

}

GS::ErrCode CreateCadSlabs()
{
	GS::ErrCode err = NoError;

	API_Element element;
	API_ElementMemo memo = {};
	API_Element mask;

	auto data = GetDataSlabJson();
	if (data.size() == 0) return err;

	for (DataSlabJson dataSlab : data)
	{
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.type.typeID = API_SlabID;
		err = ACAPI_Element_GetDefaults(&element, nullptr);
		element.slab.poly.nCoords = dataSlab.listPoint.size()+1;
		element.slab.poly.nSubPolys = 1;
		auto name = dataSlab.name;
		element.slab.thickness = dataSlab.thick / 1000;
		element.slab.level = dataSlab.zElevation / 1000;

		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		memo.coords = (API_Coord**)BMhAllClear((element.slab.poly.nCoords + 1) * sizeof(API_Coord));
		memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));

		Int32 iCoord = 1;
		for (Int32 i = 0; i < dataSlab.listPoint.size(); i++) {
			(*memo.coords)[iCoord] = {dataSlab.listPoint[i].x / 1000,dataSlab.listPoint[i].y / 1000 };
			++iCoord;
		}
		(*memo.coords)[iCoord] = (*memo.coords)[1];
		(*memo.pends)[1] = element.slab.poly.nCoords;
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
