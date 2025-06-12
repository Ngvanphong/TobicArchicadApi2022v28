#include "ColumnCad.h"
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


std::vector<DataColumnJson> GetDataColumnJson()
{
	std::vector<DataColumnJson> data;
	FILE* fp = fopen("C:/TobicCadArchi/columndata.json", "rb");
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
		DataColumnJson dataColumn = DataColumnJson();
		dataColumn.name = itr->GetObject()["name"].GetString();
		dataColumn.zElevation = itr->GetObject()["zelevation"].GetDouble();
		dataColumn.point.x = itr->GetObject()["point"].FindMember("x")->value.GetDouble();
		dataColumn.point.y = itr->GetObject()["point"].FindMember("y")->value.GetDouble();

		data.push_back(dataColumn);
	}
	return data;

}

GS::ErrCode CreateCadColumns()
{
	GSErrCode err = NoError;
	API_Element element;
	API_Element mask;
	API_ElementMemo memo;


	API_Element elementDrawing;
	GS::Array<API_Neig> selNeigs;
	API_SelectionInfo selectionInfo;
	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (err != NoError) return NoError;
	if (selectionInfo.typeID == API_SelEmpty) return NoError;
	for (Int32 j = 0; j < selectionInfo.sel_nElem; j++) {
		API_Element item;
		BNClear(item);
		item.header.guid = selNeigs[j].guid;
		err = ACAPI_Element_Get(&item);
		if (err == NoError) {
			if (item.header.type.typeID == API_DrawingID) {
				elementDrawing = item;
				break;
			}
		}

	}
	
	double angleDrawing = elementDrawing.drawing.angle;

	auto data = GetDataColumnJson();
	if (data.size() == 0) return err;

	for (DataColumnJson dataColumn : data) {
		BNClear(element);
		BNClear(memo);
		element.header.type.typeID = API_ColumnID;
		err = ACAPI_Element_GetDefaults(&element, &memo);
		element.column.origoPos = { dataColumn.point.x / 1000,dataColumn.point.y / 1000 };
		element.column.bottomOffset = dataColumn.zElevation/1000;
		element.column.height = 3.0;
		element.column.axisRotationAngle = angleDrawing;
		auto name = dataColumn.name;

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
