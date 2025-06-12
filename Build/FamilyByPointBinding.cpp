#include "FamilyByPointBinding.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

std::vector<std::string> adv_tokenizer(std::string s, char del)
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


std::vector<PointFa> GetPointByTxtFile()
{
	std::vector<PointFa> result = std::vector<PointFa>();
	std::fstream datafile;
	datafile.open("C:/Tobic/toado_xyz.txt", std::ios::in);
	if (datafile.is_open()) 
	{
		std::string linedata="";
		while (std::getline(datafile,linedata))
		{
			if (linedata != "" && !linedata.empty()) 
			{
				auto listString = adv_tokenizer(linedata, ';');
				double x = std::stod(listString[0]);
				double y = std::stod(listString[1]);
				double z = std::stod(listString[2]);
				PointFa point = PointFa();
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


API_Element GetPartElement(GS::Array<API_Neig> selNeigs) 
{
	API_Element result;
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return result;
	}
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == GS::NoError && element.header.type.typeID== API_ObjectID)
		{
			result = element;
			break;
		}
	}
	return result;
}



GS::ErrCode PutFamilyByPoint(void)
{

	std::vector<PointFa> listDataPoint = GetPointByTxtFile();
	GS::Array<API_Neig> selNeigs;
	API_Element selectedElement = GetPartElement(selNeigs);
	GS::ErrCode err = NoError;
	err = ACAPI_Element_Get(&selectedElement);
	if (err != NoError) return NoError;
	API_Element newElement;
	API_ElementMemo newMemo;

	API_StoryInfo storyInfo = {};
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	//ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	API_StoryType currentStory;
	API_StoryType* pointerIterator = &(*storyInfo.data)[0];
	for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
		if (pointerIterator != nullptr) {
			if (id == selectedElement.header.floorInd) {
				currentStory = *pointerIterator;
				break;
			}
		}
		++pointerIterator;
	}
	for (auto point : listDataPoint) 
	{
		BNZeroMemory(&newElement, sizeof(API_Element));
		BNZeroMemory(&newMemo, sizeof(API_ElementMemo));
		newElement.header.type.typeID = selectedElement.header.type.typeID;
		newElement.header.guid = selectedElement.header.guid;
		err = ACAPI_Element_Get(&newElement);
		ACAPI_Element_GetMemo(newElement.header.guid, &newMemo);
		newElement.object.pos.x = point.X;
		newElement.object.pos.y = point.Y;
		newElement.header.floorInd = selectedElement.header.floorInd;
		double zOffset = point.Z - currentStory.level;
		newElement.object.level = zOffset;
		err = ACAPI_Element_Create(&newElement, &newMemo);

		#pragma region LibPart
				//auto libraryIndex = newElement.object.libInd;

				/*API_LibPart           libPart;
				libPart.index = libraryIndex;
				err = ACAPI_LibPart_Search(&libPart, true);*/

				/*API_AddParType** addPars = nullptr;
				double               a, b;
				Int32                addParNum, i, ind, i1, i2;
				API_LibPartDetails   details;*/

				//err= ACAPI_LibPart_GetParams(libraryIndex, &a, &b, &addParNum, &addPars);

				//for (i = 0; i < addParNum; i++) 
				//{
				//	if ((*addPars)[i].typeMod == API_ParSimple)
				//	{
				//		auto type = (*addPars)[i].typeID;
				//		auto name = (*addPars)[i].name;
				//		auto value = (*addPars)[i].value.real;
				//		auto valueStr = (*addPars)[i].value.uStr;

				//	}
				//}
				//ACAPI_LibPart_GetDetails(libPart.index, &details);
				/*API_Elem_Head elemHead;
				API_PropertyObjectRefType** propRefs;
				API_LibPart                libPart;
				Int32                      nProp, i;
				char                       s[256];
				BNZeroMemory(&elemHead, sizeof(API_Elem_Head));
				elemHead.guid = newElement.header.guid;
				err = ACAPI_Element_GetPropertyObjects(&elemHead, &propRefs, &nProp);
				for (int i = 0; i < nProp; i++) {
					BNZeroMemory(&libPart, sizeof(API_LibPart));
					libPart.index = (*propRefs)[i].libIndex;
					err = ACAPI_LibPart_Get(&libPart);
					auto name = libPart.docu_UName;
					if ((*propRefs)[i].assign)
					{

					}
					else
					{

					}


				}*/
		#pragma endregion

	}

	ACAPI_DisposeElemMemoHdls(&newMemo);

	return NoError;
}
