#include "ReadDataCad.h"
#include "DataReadCad.h"
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

#include <windows.h>



std::wstring ToWStringCad(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}


API_Element SelectionFileCad(GS::Array<API_Neig> selNeigs)
{


	API_Element dwgFile = {};

	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) return dwgFile;
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
	{
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			if (element.header.type.typeID == API_DrawingID) {
				dwgFile = element;
				return dwgFile;
			}
		}
	}


	
	return dwgFile;
}

static IO::Location GetSpecialFolderLocation(API_SpecFolderID id)
{
	IO::Location folder;
	ACAPI_ProjectSettings_GetSpecFolder(&id, &folder);
	return folder;
}



GS::ErrCode GetInfromationCad(bool isBeam, bool isColumn,bool isWall,bool isFoundataion, bool isRoom)
{
	API_Element element;
	GS::Array<API_Neig> selNeigs;
	element = SelectionFileCad(selNeigs);
	API_DrawingLinkInfo drawingLinkInfo;
	BNZeroMemory(&drawingLinkInfo, sizeof(API_DrawingLinkInfo));
	GSErrCode err = ACAPI_Drawing_GetDrawingLink(&element.header.guid,&drawingLinkInfo);
	if (err != NoError) return err;
	if (drawingLinkInfo.linkTypeID == API_DrawingLink_ExternalViewID) return err;
	GS::UniString path;
	err = (*drawingLinkInfo.linkPath).ToPath(&path);
	auto pathName = ToWStringCad(path);

	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	err = ACAPI_Element_GetMemo(element.header.guid, &memo);
	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return err;
	}

	API_DrawingType drawingType = element.drawing;
	API_Coord location = { drawingType.offset.x * 1000,drawingType.offset.y * 1000 };
	auto rotaion = drawingType.angle;

	//GS::Array<API_Guid> allGuidGrids;
	//ACAPI_Element_GetElemList(API_ObjectID, &allGuidGrids);
	//GS::Array<API_Element> allGrids;
	//for (API_Guid guid : allGuidGrids) {
	//	API_Element elItem = {};
	//	elItem.header.guid = guid;
	//	err = ACAPI_Element_Get(&elItem);
	//	if (err == NoError) {
	//		if (elItem.header.type.variationID == APIVarId_GridElement)
	//		{
	//			ACAPI_Element_GetMemo(elItem.header.guid, &memo);
	//			auto lableGrid = memo.params[0]->name;
	//			allGrids.Push(elItem);
	//		}
	//	}
	//}


	//DataCad dataCad = DataCad();
	//dataCad.path = pathName;
	//dataCad.location = location;
	//dataCad.angle = rotaion;

	//rapidjson::Document d;
	//d.SetObject();

	//std::string s(pathName.begin(), pathName.end());

	//rapidjson::Value pathname;
	//pathname.SetString(s.c_str(), d.GetAllocator());
	//d.AddMember("pathname", pathname, d.GetAllocator());
	//d.AddMember("x", location.x, d.GetAllocator());
	//d.AddMember("y", location.y, d.GetAllocator());
	//d.AddMember("angle", rotaion, d.GetAllocator());
	//FILE* fp = fopen("C:/TobicCadArchi/data.json", "wb");
	//char writeBuffer[65536];
	//rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	//rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	//d.Accept(writer);

	//fclose(fp);


	std::wofstream fw(L"C:/TobicCadArchi/data.txt", std::ofstream::out);
	const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
	if (fw.is_open())
	{
		//store array contents to text file
		fw.imbue(utf8_locale);
		fw << pathName << std::endl;
		fw << std::to_wstring(location.x) << std::endl;
		fw << std::to_wstring(location.y) << std::endl;
		fw << std::to_wstring(rotaion) << std::endl;
		fw.close();
	}
	if (isBeam) {

		system("start C:/TobicCadArchi/ArchicadBeamApp.exe");
	}
	else if (isColumn) {
		system("start C:/TobicCadArchi/ArchicadColumnApp.exe");
	}
	else if (isWall) {
		system("start C:/TobicCadArchi/ArchicadWallApp.exe");
	}
	else if (isFoundataion) {
		system("start C:/TobicCadArchi/ArchicadFloorApp.exe");
	}
	else if (isRoom) {
		system("start C:/TobicCadArchi/ArchicadRoomApp.exe");
	}



	return NoError;
}









