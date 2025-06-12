#include "filereadstream.h"
#include <document.h>
#include "EditMeshTopo.h"

API_Element SelectMeshTopo(GS::Array<API_Neig> selNeigs)
{
	API_Element result;
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID == API_SelEmpty) {
		delete selectionInfo;
		return result;
	}
	for (int i = 0; i < selectionInfo->sel_nElem; i++) {
		API_Element element;
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError && element.header.type.typeID == API_MeshID)
		{
			return element;
		}
	}
	delete selectionInfo;
	return result;
}

std::vector <std::vector<DataPolyMeshEdit>> GetMeshEditJson()
{
	std::vector <std::vector<DataPolyMeshEdit>> data = std::vector <std::vector<DataPolyMeshEdit>>();
	FILE* fp = fopen("C:/Tobic/polyline.json", "rb");
	if (!fp) return data;
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document doc;
	doc.ParseStream(is);
	if (doc.HasParseError()) return data;
	fclose(fp);
	for (rapidjson::Value::ConstValueIterator it = doc.Begin(); it != doc.End(); ++it)
	{
		std::vector<DataPolyMeshEdit> arrayItem = std::vector<DataPolyMeshEdit>();
		auto arrayValue = it->GetArray();
		for (rapidjson::Value::ConstValueIterator itItem = arrayValue.Begin(); itItem != arrayValue.End(); ++itItem)
		{
			DataPolyMeshEdit point = DataPolyMeshEdit();
			point.X = itItem->GetObject()["X"].GetDouble();
			point.Y = itItem->GetObject()["Y"].GetDouble();
			point.Z = itItem->GetObject()["Z"].GetDouble();
			arrayItem.push_back(point);
		}
		if (arrayItem.size() > 0) data.push_back(arrayItem);
	}
	return data;
}

GS::ErrCode EditTopoByPolyline(void)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	API_Element mesh = SelectMeshTopo(selNeigs);

	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR(mask);
	ACAPI_ELEMENT_MASK_SET(mask, API_MeshType, levelLines);

	API_ElementMemo memo;
	err= ACAPI_Element_GetMemo(mesh.header.guid, &memo, APIMemoMask_MeshLevel);
	if (err != NoError) return NoError;
	std::vector<std::vector<DataPolyMeshEdit>> dataAll = GetMeshEditJson();
	int count = dataAll.size();
	int totalCount = 0;
	for (Int32 i = 0; i < count; i++)
	{
		totalCount += dataAll[i].size();
	}

	std::vector<API_MeshLevelCoord> oldMeshCoords = std::vector<API_MeshLevelCoord>();
	std::vector<Int32> oldMeshEnds = std::vector<Int32>();
	auto oldMeshCoordsCount = BMGetHandleSize((GSHandle)(memo.meshLevelCoords)) / sizeof(API_MeshLevelCoord);
	auto oldMeshEndCount = BMGetHandleSize((GSHandle)(memo.meshLevelEnds)) / sizeof(Int32);
	for (int i = 0; i < oldMeshCoordsCount; i++) {
		oldMeshCoords.push_back((*memo.meshLevelCoords)[i]);
	}
	for (int i = 0; i < oldMeshEndCount;i++) 
	{
		oldMeshEnds.push_back((*memo.meshLevelEnds)[i]);
	}

	int counMeshEndOldAndNew = oldMeshEndCount + count;
	int countCoordOldAndNew = totalCount + oldMeshCoordsCount;

	mesh.mesh.levelLines.nCoords = countCoordOldAndNew;
	mesh.mesh.levelLines.nSubLines = counMeshEndOldAndNew;

	memo.meshLevelCoords= reinterpret_cast<API_MeshLevelCoord**> (BMAllocateHandle((countCoordOldAndNew) * sizeof(API_MeshLevelCoord), ALLOCATE_CLEAR, 0));
	memo.meshLevelEnds = reinterpret_cast<Int32**> (BMAllocateHandle((counMeshEndOldAndNew) * sizeof(Int32), ALLOCATE_CLEAR, 0));

	Int32 indexCoord = 0;
	for (Int32 i = 0; i < oldMeshCoordsCount; i++)
	{
		(*memo.meshLevelCoords)[indexCoord] = oldMeshCoords[i];
		indexCoord++;
	}

	Int32 indexEnd = 0;
	Int32 lastIndexEnd = 0;
	for (Int32 i = 0; i < oldMeshEndCount; i++) 
	{
		(*memo.meshLevelEnds)[indexEnd] = oldMeshEnds[i];
		lastIndexEnd = oldMeshEnds[i];
		indexEnd++;
	}

	for (Int32 i = 0; i < count; i++) {
		for (Int32 j = 0; j < dataAll[i].size(); j++) {
			(*memo.meshLevelCoords)[indexCoord].c.x = dataAll[i][j].X;
			(*memo.meshLevelCoords)[indexCoord].c.y = dataAll[i][j].Y;
			(*memo.meshLevelCoords)[indexCoord].c.z = dataAll[i][j].Z;
			(*memo.meshLevelCoords)[indexCoord].vertexID = indexCoord + 1;
			indexCoord++;
		}
	}

	for (Int32 i = 0; i < count; i++) {
		lastIndexEnd += dataAll[i].size();
		(*memo.meshLevelEnds)[indexEnd] = lastIndexEnd;
		indexEnd++;
	}


	err = ACAPI_Element_Change(&mesh, &mask, &memo, APIMemoMask_MeshLevel, true);
	ACAPI_DisposeElemMemoHdls(&memo);
    return NoError;
}
