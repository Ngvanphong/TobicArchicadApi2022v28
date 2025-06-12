#include "PoinsToMeshBinding.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

API_Element SelectMeshTopoAdd(GS::Array<API_Neig> selNeigs)
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
std::vector<std::string> adv_tokenizer2(std::string s, char del)
{
	std::vector<std::string> result = std::vector<std::string>();
	std::stringstream ss(s);
	std::string word;
	while (!ss.eof()) {
		getline(ss, word, del);
		result.push_back(word);
	}
	return result;
}

std::vector<PointAddMesh> GetPointByTxtFileMesh()
{
	std::vector<PointAddMesh> result = std::vector<PointAddMesh>();
	std::fstream datafile;
	datafile.open("C:/Tobic/add_point_xyz.txt", std::ios::in);
	if (datafile.is_open())
	{
		std::string linedata = "";
		while (std::getline(datafile, linedata))
		{
			if (linedata != "" && !linedata.empty())
			{
				auto listString = adv_tokenizer2(linedata, ';');
				double x = std::stod(listString[0]);
				double y = std::stod(listString[1]);
				double z = std::stod(listString[2]);
				PointAddMesh point = PointAddMesh();
				point.X = x;
				point.Y = y;
				point.Z = z;
				result.push_back(point);
			}
		}
	}
	datafile.close();
	return result;
}

GS::ErrCode AddPointToMesh(void)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	API_Element mesh = SelectMeshTopoAdd(selNeigs);
	std::vector<PointAddMesh> listPointAndMesh = GetPointByTxtFileMesh();
	API_ElementMemo memo;
	err=ACAPI_Element_GetMemo(mesh.header.guid, &memo, APIMemoMask_MeshLevel);
	if (err != NoError) return NoError;
	
	std::vector<PointAddMesh> listPointNotSame = std::vector<PointAddMesh>();
	auto oldMeshCoordsCount = BMGetHandleSize((GSHandle)(memo.meshLevelCoords)) / sizeof(API_MeshLevelCoord);
	std::vector<API_MeshLevelCoord> oldMeshCoords = std::vector<API_MeshLevelCoord>();
	for (int i = 0; i < oldMeshCoordsCount; i++)
	{
		oldMeshCoords.push_back((*memo.meshLevelCoords)[i]);
	}

	auto oldMeshEndCount = BMGetHandleSize((GSHandle)(memo.meshLevelEnds)) / sizeof(Int32);
	std::vector<Int32> oldMeshEnds = std::vector<Int32>();
	for (int i = 0; i < oldMeshEndCount; i++)
	{
		oldMeshEnds.push_back((*memo.meshLevelEnds)[i]);
	}

	Int32 nMeshLevels = BMGetHandleSize((GSHandle)memo.meshLevelCoords) / sizeof(API_MeshLevelCoord);
	for (auto item : listPointAndMesh)
	{
		bool isSame = false;
		for (Int32 i = 0; i < nMeshLevels; i++)
		{
			double x = (*memo.meshLevelCoords)[i].c.x;
			double y = (*memo.meshLevelCoords)[i].c.y;
			if (std::abs(item.X - x) < 0.0001 && std::abs(item.Y - y) < 0.0001)
			{
				isSame = true;
				break;
			}
		}
		if (!isSame) listPointNotSame.push_back(item);
	}


	Int32 totalCount = nMeshLevels + listPointNotSame.size();
	mesh.mesh.levelLines.nCoords = totalCount;
	mesh.mesh.levelLines.nSubLines = oldMeshEndCount + listPointNotSame.size();

	memo.meshLevelCoords = reinterpret_cast<API_MeshLevelCoord**> (BMAllocateHandle((totalCount) * sizeof(API_MeshLevelCoord), ALLOCATE_CLEAR, 0));
	memo.meshLevelEnds = reinterpret_cast<Int32**> (BMAllocateHandle((oldMeshEndCount+ listPointNotSame.size()) * sizeof(Int32), ALLOCATE_CLEAR, 0));
	
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

	for (Int32 i = 0; i < listPointNotSame.size(); i++)
	{
		(*memo.meshLevelCoords)[indexCoord].c.x = listPointNotSame[i].X;
		(*memo.meshLevelCoords)[indexCoord].c.y = listPointNotSame[i].Y;
		(*memo.meshLevelCoords)[indexCoord].c.z = listPointNotSame[i].Z;
		(*memo.meshLevelCoords)[indexCoord].vertexID = indexCoord + 1;
		indexCoord++;
	}

	for (Int32 i = 0; i < listPointNotSame.size(); i++) 
	{
		lastIndexEnd++;
		(*memo.meshLevelEnds)[indexEnd] = lastIndexEnd;
		indexEnd++;
	}
	API_Element mask;
	ACAPI_ELEMENT_MASK_CLEAR(mask);
	ACAPI_ELEMENT_MASK_SET(mask, API_MeshType, levelLines);
	err = ACAPI_Element_Change(&mesh, &mask, &memo, APIMemoMask_MeshLevel, true);
	ACAPI_DisposeElemMemoHdls(&memo);
	return NoError;
}
