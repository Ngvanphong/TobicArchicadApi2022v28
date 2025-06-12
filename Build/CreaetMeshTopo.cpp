#include "filereadstream.h"
#include <document.h>
#include "CreaetMeshTopo.h"



std::vector <std::vector<DataPolyMesh>> GetMeshJson()
{
	std::vector <std::vector<DataPolyMesh>> data = std::vector <std::vector<DataPolyMesh>>();
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
		std::vector<DataPolyMesh> arrayItem = std::vector<DataPolyMesh>();
		auto arrayValue = it->GetArray();
		for (rapidjson::Value::ConstValueIterator itItem = arrayValue.Begin(); itItem != arrayValue.End(); ++itItem)
		{
			DataPolyMesh point = DataPolyMesh();
			point.X = itItem->GetObject()["X"].GetDouble();
			point.Y = itItem->GetObject()["Y"].GetDouble();
			point.Z = itItem->GetObject()["Z"].GetDouble();
			arrayItem.push_back(point);
		}
		if (arrayItem.size() > 0) data.push_back(arrayItem);
	}
	return data;
}

GS::ErrCode ModifyTopo()
{
	//GS::Array<API_Neig> selNeigs;
	//API_Element* mesh = SelectMesh(selNeigs);
	//if (mesh == nullptr) return NoError;

	//API_ElementMemo memo = {};
	//ACAPI_Element_GetMemo(mesh->header.guid, &memo);

	//auto coords = &(*memo.coords)[1]; //

	//auto vertext = &(*memo.vertexIDs)[1];

	//auto countVert = BMGetHandleSize((GS::GSHandle)memo.vertexIDs) / sizeof(UInt32)-1;
	//std::vector<UInt32> arayVextex;
	//for (UInt32 in = 0; in < countVert; in++) {
	//	arayVextex.push_back((*memo.vertexIDs)[in+1]);
	//}

	//auto countPointLevel = BMGetHandleSize((GS::GSHandle)memo.meshLevelCoords) / sizeof(API_MeshLevelCoord);
	//auto countSub = BMGetHandleSize((GS::GSHandle)memo.meshLevelEnds) / sizeof(GS::Int32);

	//for (int i = 0; i < countPointLevel; i++) {
	//	auto point = (*memo.meshLevelCoords)[i];
	//	continue;
	//}
	//for (int i = 0; i < countSub; i++) {
	//	auto index = (*memo.meshLevelEnds)[i];
	//	continue;
	//}

	//auto countPoly = BMGetHandleSize((GS::GSHandle)memo.coords) / sizeof(API_Coord)-1;
	//auto countAllPoly = (*memo.meshPolyZ)[0];
	//auto test2 = (*memo.meshLevelCoords)[1].c;
	//auto countLevel3 = (*memo.meshLevelEnds)[2];
	//auto vetex1 = (*memo.meshLevelCoords)[1].vertexID;



	std::vector<std::vector<DataPolyMesh>> dataAll = GetMeshJson();
	int count = dataAll.size();
	int totalCount = 0;
	for (Int32 i = 0; i < count; i++)
	{
		totalCount += dataAll[i].size();
	}

	API_Coord3D centerPoint{ dataAll[0][0].X,dataAll[0][0].Y,dataAll[0][0].Z };

	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.type = API_MeshID;
	GS::ErrCode err = ACAPI_Element_GetDefaults(&element, nullptr);
	if (err != NoError) return NoError;
	element.mesh.poly.nCoords = 5;
	element.mesh.poly.nSubPolys = 1;
	element.mesh.poly.nArcs = 0;

	element.mesh.levelLines.nCoords= totalCount;
	element.mesh.levelLines.nSubLines = count;
	
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	memo.coords = reinterpret_cast<API_Coord**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
	memo.pends = reinterpret_cast<Int32**> (BMAllocateHandle((element.mesh.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	memo.parcs = reinterpret_cast<API_PolyArc**> (BMAllocateHandle(element.mesh.poly.nArcs * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));
	memo.meshPolyZ = reinterpret_cast<double**> (BMAllocateHandle((element.mesh.poly.nCoords + 1) * sizeof(double), ALLOCATE_CLEAR, 0));
	memo.meshLevelCoords= reinterpret_cast<API_MeshLevelCoord**> (BMAllocateHandle((totalCount) * sizeof(API_MeshLevelCoord), ALLOCATE_CLEAR, 0));
	memo.meshLevelEnds= reinterpret_cast<Int32**> (BMAllocateHandle((element.mesh.levelLines.nSubLines) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	
	Int32 index = 0;
	for (Int32 i = 0; i < count; i++)
	{
		for (Int32 j = 0; j < dataAll[i].size(); j++) 
		{
			(*memo.meshLevelCoords)[index].c.x = dataAll[i][j].X;
			(*memo.meshLevelCoords)[index].c.y = dataAll[i][j].Y;
			(*memo.meshLevelCoords)[index].c.z = dataAll[i][j].Z;
			(*memo.meshLevelCoords)[index].vertexID = index +1;
			index++;
		}
	}

	Int32 indexSet = 0;
	for (Int32 i = 0; i < count; i++) 
	{
		indexSet += dataAll[i].size();
		(*memo.meshLevelEnds)[i] = indexSet;
	}

	(*memo.coords)[1].x = centerPoint.x - 1000;
	(*memo.coords)[1].y = centerPoint.y - 1000;
	(*memo.coords)[2].x = centerPoint.x + 1000;
	(*memo.coords)[2].y = centerPoint.y - 1000;
	(*memo.coords)[3].x = centerPoint.x + 1000;
	(*memo.coords)[3].y = centerPoint.y + 1000;
	(*memo.coords)[4].x = centerPoint.x - 1000;
	(*memo.coords)[4].y = centerPoint.y + 1000;
	(*memo.coords)[element.mesh.poly.nCoords] = (*memo.coords)[1];

	(*memo.pends)[1] = element.mesh.poly.nCoords;

	(*memo.meshPolyZ)[1] = centerPoint.z;
	(*memo.meshPolyZ)[2] = centerPoint.z;
	(*memo.meshPolyZ)[3] = centerPoint.z;
	(*memo.meshPolyZ)[4] = centerPoint.z;
	(*memo.meshPolyZ)[5] = (*memo.meshPolyZ)[1];
	err = ACAPI_Element_Create(&element, &memo);

	return NoError;
}
