#include "WallToFloorBinding.h"
#include <codecvt>
#include <string>
#include <locale>
/// <summary>
/// Create material with name MaxMaterial;
/// </summary>
/// <param name="selNeigs"></param>
/// <returns></returns>

std::vector<std::vector<API_Element>> GetWallOutFloorBeamCeiling(GS::Array<API_Neig> selNeigs)
{
	std::vector<std::vector<API_Element>> result = std::vector<std::vector<API_Element>>();
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID == API_SelEmpty) {
		delete selectionInfo;
		return result;
	}
	std::vector<API_Element> allWall;
	std::vector<API_Element> allTarget;
	for (int i = 0; i < selectionInfo->sel_nElem; i++) 
	{
		API_Element element;
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			if (element.header.type.typeID == API_WallID) 
			{
				allWall.push_back(element);
			}
			else if (element.header.type.typeID == API_SlabID || element.header.type.typeID == API_BeamID)
			{
				allTarget.push_back(element);
			}
		}
	}
	result.push_back(allWall);
	result.push_back(allTarget);
	return result;
}
API_Coord	SubCoordWall(API_Coord c1, API_Coord c2)

{
	API_Coord c;
	c.x = c1.x - c2.x;
	c.y = c1.y - c2.y;
	return c;
}

bool CheckParellWall(const API_Coord& v1, const API_Coord& v2)
{
	if ((abs(v1.x) < 0.0001 && abs(v1.y) < 0.0001) || (abs(v2.x) < 0.0001 && abs(v2.y) < 0.0001)) return true;
	auto result = (v1.x * v2.x + v1.y * v2.y);
	return  abs(abs(result) - 1) < 0.001;
}

API_Coord	NormalizeCoord2DWall(const API_Coord& v)

{
	API_Coord ret = v;
	double len = v.x * v.x + v.y * v.y;
	if (len > 1e-10) {
		double lambda = 1.0 / sqrt(len);
		ret.x = ret.x * lambda;
		ret.y = ret.y * lambda;
	}
	return ret;
}

// create material "MaxMaterial" has priority is Max;
std::wstring wStringAttributeMaterial(const char* utf8Bytes)
{
	//setup converter
	using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
	std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(utf8Bytes);
}

std::wstring ToWStringWall(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

API_StoryType GetStoryElement(API_StoryInfo storyInfo,API_Element element)
{
	API_StoryType result;
	API_StoryType* pointerIterator = &(*storyInfo.data)[0];
	for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
		if (pointerIterator != nullptr) {
			if (id == element.header.floorInd) {
				result= *pointerIterator;
				break;
			}
		}
		++pointerIterator;
	}
	return result;
}

double GetHeightBeam(API_Element beam) 
{
	std::wstring heightBeam = L"Height";
	GS::Array<API_PropertyDefinition> propertyDefinitions;
	 ACAPI_Element_GetPropertyDefinitions(beam.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
	 for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
	 {
		 if (propertyDefinition.measureType != API_PropertyLengthMeasureType) continue;
		 auto name = ToWStringWall(propertyDefinition.name);
		 if ( name == heightBeam)
		 {
			 API_Property property;
			 ACAPI_Element_GetPropertyValue(beam.header.guid, propertyDefinition.guid, property);
			 return property.value.singleVariant.variant.doubleValue;
		 }
	 }
	 return 0;

}

GS::ErrCode AlignWallToFloorBeam(void)
{
	const std::wstring nameMaterialWall = L"MaxMaterial";
	API_Attribute attrib;
	bool hasAttribute = false;
	UInt32 count, i;
	API_AttributeIndex indexMaxMaxMaterial;
	ACAPI_Attribute_GetNum(API_BuildingMaterialID, count);
	for (i = 1; i <= count; i++) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_BuildingMaterialID;
		attrib.header.index = ACAPI_CreateAttributeIndex(i);
		ACAPI_Attribute_Get(&attrib);
		if (wStringAttributeMaterial(attrib.header.name) == nameMaterialWall) 
		{
			hasAttribute = true;
			indexMaxMaxMaterial = ACAPI_CreateAttributeIndex(i);
			break;
		}
	}
	if (!hasAttribute) return NoError;

	GS::Array<API_Neig> selNeigs;
	auto allData = GetWallOutFloorBeamCeiling(selNeigs);
	std::vector<API_Element> allWall = allData[0];
	std::vector<API_Element> allTarget = allData[1];
	API_Element markWall;
	ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, buildingMaterial);
	ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, relativeTopStory);
	ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, height);

	std::wstring volumnName = L"Volume (Net)";
	GS::Array<API_PropertyDefinition> propertyDefinitions;
	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
	for (API_Element wall : allWall)
	{
		API_StoryType wallStory = GetStoryElement(storyInfo,wall);
		double bottomLevelCanAttach = wallStory.level + wall.wall.bottomOffset+ 1.5;

		auto oldMaterial = wall.wall.buildingMaterial;
		double originHeight = wall.wall.height;
		std::vector<double> targetEleChangeVolumne;
		for (API_Element target : allTarget) 
		{
			API_Property property;
			ACAPI_Element_GetPropertyDefinitions(target.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
			API_PropertyDefinition propertyDefinitionTarget;
			for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
			{
				auto name = ToWStringWall(propertyDefinition.name);
				if (name == volumnName)
				{
					ACAPI_Element_GetPropertyValue(target.header.guid, propertyDefinition.guid, property);
					propertyDefinitionTarget = propertyDefinition;
					break;
				}
			}
			double originTargetVolume = property.value.singleVariant.variant.doubleValue;

			wall.wall.relativeTopStory = 0;
			wall.wall.height = originHeight + 1.5;
			wall.wall.buildingMaterial = indexMaxMaxMaterial;
			ACAPI_Element_Change(&wall, &markWall, nullptr, 0UL, true);

			ACAPI_Element_GetPropertyValue(target.header.guid, propertyDefinitionTarget.guid, property);
			double volumTargetCheck = property.value.singleVariant.variant.doubleValue;
			if (abs(originTargetVolume - volumTargetCheck) > 0.00001)
			{
				API_StoryType storyLevel = GetStoryElement(storyInfo, target);
				double offsetTarget = 0;
				if (target.header.type.typeID == API_BeamID) {
					// tru chieu cao dam
					API_Coord directionWall = NormalizeCoord2DWall(SubCoordWall(wall.wall.endC, wall.wall.begC));
					API_Coord directionBeam = NormalizeCoord2DWall(SubCoordWall(target.beam.endC, target.beam.begC));
					if (CheckParellWall(directionWall, directionBeam))
					{
						double heightBeam = GetHeightBeam(target);
						offsetTarget = storyLevel.level + target.beam.level - heightBeam;
						if (offsetTarget > bottomLevelCanAttach)
						{
							targetEleChangeVolumne.push_back(offsetTarget);
						}
					}
				}
				else 
				{
					auto vectorWall= SubCoordWall(wall.wall.endC, wall.wall.begC);
					double lengthWall = sqrt(vectorWall.x * vectorWall.x + vectorWall.y * vectorWall.y);
					if (abs(originTargetVolume - volumTargetCheck) > wall.wall.thickness * target.slab.thickness * lengthWall * 0.2)
					{
						offsetTarget = storyLevel.level + target.slab.level - target.slab.thickness;
						if (offsetTarget > bottomLevelCanAttach)
						{
							targetEleChangeVolumne.push_back(offsetTarget);
						}
					}
				}
			}
			else
			{
				wall.wall.height = originHeight -1.5;
				ACAPI_Element_Change(&wall, &markWall, nullptr, 0UL, true);

				ACAPI_Element_GetPropertyValue(target.header.guid, propertyDefinitionTarget.guid, property);
				double volumTargetCheck = property.value.singleVariant.variant.doubleValue;
				if (abs(originTargetVolume - volumTargetCheck) > 0.00001)
				{
					API_StoryType storyLevel = GetStoryElement(storyInfo, target);
					double offsetTarget = 0;
					if (target.header.type.typeID == API_BeamID) {
						// tru chieu cao dam
						API_Coord directionWall = NormalizeCoord2DWall(SubCoordWall(wall.wall.endC, wall.wall.begC));
						API_Coord directionBeam= NormalizeCoord2DWall(SubCoordWall(target.beam.endC, target.beam.begC));
						if (CheckParellWall(directionWall, directionBeam))
						{
							double heightBeam = GetHeightBeam(target);
							offsetTarget = storyLevel.level + target.beam.level - heightBeam;
							if (offsetTarget > bottomLevelCanAttach)
							{
								targetEleChangeVolumne.push_back(offsetTarget);
							}
						}
					}
					else
					{
						auto vectorWall = SubCoordWall(wall.wall.endC, wall.wall.begC);
						double lengthWall = sqrt(vectorWall.x * vectorWall.x + vectorWall.y * vectorWall.y);
						if (abs(originTargetVolume - volumTargetCheck) > wall.wall.thickness * target.slab.thickness * lengthWall * 0.2)
						{
							offsetTarget = storyLevel.level + target.slab.level - target.slab.thickness;
							if (offsetTarget > bottomLevelCanAttach)
							{
								targetEleChangeVolumne.push_back(offsetTarget);
							}
						}
						
					}
				}
			}
			wall.wall.height = originHeight;
			ACAPI_Element_Change(&wall, &markWall, nullptr, 0UL, true);
		}

		double levelMin = 1111111111;
		for (auto item : targetEleChangeVolumne) 
		{
			if (levelMin > item) 
			{
				levelMin = item;
			}
		}
		if (levelMin != 1111111111) 
		{
			ACAPI_Element_Get(&wall);
			double elementEle = wallStory.level + wall.wall.bottomOffset + wall.wall.height;
			double heightAfter = levelMin - elementEle;
			wall.wall.height = wall.wall.height+ heightAfter;
			wall.wall.buildingMaterial = oldMaterial;
			ACAPI_Element_Change(&wall, &markWall, nullptr, 0UL, true);
		}
		else 
		{
			ACAPI_Element_Get(&wall);
			wall.wall.height = originHeight;
			wall.wall.buildingMaterial = oldMaterial;
			ACAPI_Element_Change(&wall, &markWall, nullptr, 0UL, true);
		}
	}
	return NoError;
}
