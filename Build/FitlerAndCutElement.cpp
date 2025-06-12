#include "FitlerAndCutElement.h"
#include <algorithm>    // std::sort



std::vector<API_Element> SelectionElement3D(GS::Array<API_Neig> selNeigs) {
	std::vector<API_Element> vectorElements;
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return vectorElements;
	}
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == GS::NoError) {
			vectorElements.push_back(element);
		}
	}
	return vectorElements;
}

struct myclass {
	bool operator() (API_StoryType i, API_StoryType j) { return (i.level < j.level); }
} myobject;

GS::ErrCode ShowFilterAndCut3d()
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> vectorElements = SelectionElement3D(selNeigs);
	if (vectorElements.size() == 0) return NoError;
	double xMin = 10000;
	double yMin = 10000;
	double xMax = -10000;
	double yMax = -10000;
	double zMin = 10000;
	double zMax = -10000;
	bool isBox = false;
	for (API_Element element : vectorElements) {
		API_Box3D* boxItem = new API_Box3D();
		err= ACAPI_Element_CalcBounds(&element.header, boxItem);
		if (err == NoError) {
			if (xMin > boxItem->xMin) xMin = boxItem->xMin;
			if (yMin > boxItem->yMin) yMin = boxItem->yMin;
			if (zMin > boxItem->zMin) zMin = boxItem->zMin;

			if (xMax < boxItem->xMax) xMax = boxItem->xMax;
			if (yMax < boxItem->yMax) yMax = boxItem->yMax;
			if (zMax < boxItem->zMax) zMax = boxItem->zMax;
			isBox = true;
		}
		delete boxItem;
	}
	if (!isBox) return NoError;

	/*API_StoryInfo storyInfo;
	err = ACAPI_Environment(APIEnv_GetStorySettingsID,&storyInfo, nullptr);
	std::vector<API_StoryType> allStory;
	if (err == NoError) {

		auto countStory = storyInfo.lastStory - storyInfo.firstStory + 1;
		for (auto i = 0; i < countStory; i++)
		{
			auto story = (*storyInfo.data)[i];
			allStory.push_back(story);
		}
	}
	if (allStory.size() == 0) return NoError;

	std::sort(allStory.begin(), allStory.end(), myobject);

	API_StoryType* firstStory= nullptr;
	API_StoryType* endStory= nullptr;
	for (std::vector<API_StoryType>::iterator it = allStory.begin(); it != allStory.end(); ++it)
	{
		auto elevation = (*it).level;
		if (elevation > zMax&&endStory==nullptr)
		{
			endStory = &(*it);
			break;
		}
	}

	for (auto it = allStory.rbegin(); it != allStory.rend(); ++it)
	{
		auto elevation = (*it).level;
		if (elevation < zMin && firstStory == nullptr)
		{
			firstStory = &(*it);
			break;
		}
	}

	if (firstStory == nullptr) firstStory = &allStory[0];
	if (endStory == nullptr) endStory = &allStory[allStory.size()-1];*/



	//API_DatabaseInfo	myDB;
	//ACAPI_Database(APIDb_GetCurrentDatabaseID, &myDB);

	//// reset content
	//API_DatabaseInfo	planDB;
	//BNZeroMemory(&planDB, sizeof(API_DatabaseInfo));
	//planDB.typeID = APIWind_FloorPlanID;
	//ACAPI_Database(APIDb_ResetCurrentDatabaseID);
	//err = ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &planDB);

	//API_SelectionInfo 	selectionInfo;
	//BNClear(selectionInfo);
	//selectionInfo.typeID = API_MarqueeHorBox;
	//selectionInfo.marquee.box.xMin = xMin - 1;
	//selectionInfo.marquee.box.yMin = yMin - 1;
	//selectionInfo.marquee.box.xMax = xMax + 1;
	//selectionInfo.marquee.box.yMax = yMax + 1;
	//selectionInfo.multiStory = true;
	//err = ACAPI_Selection_SetMarquee(&selectionInfo);



	//filterAndCutSetting.firstStory3D = firstStory->index;
	//filterAndCutSetting.lastStory3D = endStory->index;
	//filterAndCutSetting.allStories = false;
	//filterAndCutSetting.trimToStoryRange = true;


	API_3DCutPlanesInfo cutInfo;
	BNZeroMemory(&cutInfo, sizeof(API_3DCutPlanesInfo));
	err = ACAPI_View_Get3DCuttingPlanes(&cutInfo);
	if (err == NoError)
	{
		if (cutInfo.shapes != nullptr) {
			BMKillHandle((GSHandle*)&(cutInfo.shapes));
		}
		cutInfo.isCutPlanes = true;
		cutInfo.nShapes = 6;
		cutInfo.shapes = reinterpret_cast<API_3DCutShapeType**>(BMAllocateHandle(cutInfo.nShapes * sizeof(API_3DCutShapeType), ALLOCATE_CLEAR, 0));
		if (cutInfo.shapes != nullptr)
		{

			(*cutInfo.shapes)[0].cutStatus = 2;
			(*cutInfo.shapes)[0].pa = xMax + 1;
			(*cutInfo.shapes)[0].pb = zMin - 1;
			(*cutInfo.shapes)[0].pc = xMin - 1;
			(*cutInfo.shapes)[0].pd = zMin - 1;

			(*cutInfo.shapes)[1].cutStatus = 2;
			(*cutInfo.shapes)[1].pa = xMin - 1;
			(*cutInfo.shapes)[1].pb = zMax + 1;
			(*cutInfo.shapes)[1].pc = xMax + 1;
			(*cutInfo.shapes)[1].pd = zMax + 1;

			(*cutInfo.shapes)[2].cutStatus = 3;
			(*cutInfo.shapes)[2].pa = xMin - 1;
			(*cutInfo.shapes)[2].pb = yMin - 1;
			(*cutInfo.shapes)[2].pc = xMin - 1;
			(*cutInfo.shapes)[2].pd = yMax + 1;

			(*cutInfo.shapes)[3].cutStatus = 3;
			(*cutInfo.shapes)[3].pa = xMax + 1;
			(*cutInfo.shapes)[3].pb = yMax + 1;
			(*cutInfo.shapes)[3].pc = xMax + 1;
			(*cutInfo.shapes)[3].pd = yMin - 1;

			(*cutInfo.shapes)[4].cutStatus = 1;
			(*cutInfo.shapes)[4].pa = yMin - 1;
			(*cutInfo.shapes)[4].pb = xMin - 1;
			(*cutInfo.shapes)[4].pc = yMin - 1;
			(*cutInfo.shapes)[4].pd = xMax + 1;

			(*cutInfo.shapes)[5].cutStatus = 1;
			(*cutInfo.shapes)[5].pa = yMax + 1;
			(*cutInfo.shapes)[5].pb = xMax + 1;
			(*cutInfo.shapes)[5].pc = yMax + 1;
			(*cutInfo.shapes)[5].pd = xMin - 1;

		}

		err = ACAPI_View_Get3DCuttingPlanes(&cutInfo);
		if (err == NoError) {
			API_WindowInfo windowInfo;
			BNZeroMemory(&windowInfo, sizeof(API_WindowInfo));
			windowInfo.typeID = APIWind_3DModelID;
			err = ACAPI_Window_ChangeWindow(&windowInfo);
		}
		BMKillHandle((GSHandle*)&(cutInfo.shapes));
	}

	return GS::ErrCode();
}


