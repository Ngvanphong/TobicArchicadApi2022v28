#include "SeperateWall.h"
#include "filereadstream.h"
#include <document.h>
#include <codecvt>
#include <string>
#include <locale>

std::vector<API_Element> SelectionWallLayer(GS::Array<API_Neig> selNeigs) 
{
	std::vector<API_Element> result;
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
		if (err == NoError && element.header.type.typeID == API_WallID) {
			result.push_back(element);
		}
	}
	delete selectionInfo;
	return result;
}

std::wstring stringToWstringWall(const char* utf8Bytes)
{
	//setup converter
	using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
	std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(utf8Bytes);
}

std::vector<WallTypeData> GetJsonWallLayer() 
{
	std::vector<WallTypeData> data;
	FILE* fp = fopen("C:/TobicCadArchi/LayerWall.json", "rb");
	if (!fp) return data;
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document doc;
	doc.ParseStream(is);
	if (doc.HasParseError()) return data;
	fclose(fp);

	for (rapidjson::Value::ConstValueIterator itr = doc.Begin(); itr != doc.End(); ++itr) 
	{
		WallTypeData wallLayer = WallTypeData();
		wallLayer.Name = itr->GetObject()["Name"].GetString();
		
		wallLayer.Top1Name= itr->GetObject()["Layer"].FindMember("Top1Name")->value.GetString();
		wallLayer.Top1 = itr->GetObject()["Layer"].FindMember("Top1")->value.GetDouble();

		wallLayer.Top2Name = itr->GetObject()["Layer"].FindMember("Top2Name")->value.GetString();
		wallLayer.Top2 = itr->GetObject()["Layer"].FindMember("Top2")->value.GetDouble();

		wallLayer.CoreName = itr->GetObject()["Layer"].FindMember("CoreName")->value.GetString();
		wallLayer.Core = 0;

		wallLayer.Bottom1Name = itr->GetObject()["Layer"].FindMember("Bottom1Name")->value.GetString();
		wallLayer.Bottom1= itr->GetObject()["Layer"].FindMember("Bottom1")->value.GetDouble();

		wallLayer.Bottom2Name = itr->GetObject()["Layer"].FindMember("Bottom2Name")->value.GetString();
		wallLayer.Bottom2 = itr->GetObject()["Layer"].FindMember("Bottom2")->value.GetDouble();
		data.push_back(wallLayer);
	}
	return data;

}

// chu y ve mot cai default cho refence line ve center va tiet dien 1 lop
GS::ErrCode SeperateWallData(void)
{
	GS::ErrCode err = NoError;
	auto data = GetJsonWallLayer();
	if (data.size() == 0) return NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> selectedElement = SelectionWallLayer(selNeigs);

	API_ElementMemo memo = {};
	
	for (API_Element element : selectedElement) 
	{
		BNClear(memo);
		ACAPI_Element_GetMemo(element.header.guid, &memo);
		auto id = stringToWstringWall((*memo.elemInfoString).ToCStr());
		WallTypeData wallTypeLayer;
		bool isHasWall = false;
		for (auto item : data) {
			if (stringToWstringWall(item.Name.c_str()) == id) 
			{
				wallTypeLayer = item;
				isHasWall = true;
				break;
			}
		}
		if (isHasWall) 
		{
			double sideCenter = element.wall.offsetFromOutside;

			double totalThickness = element.wall.thickness;
			double core = totalThickness * 1000 - (wallTypeLayer.Bottom1 + wallTypeLayer.Bottom2 +
				wallTypeLayer.Top1 + wallTypeLayer.Top2 );

			wallTypeLayer.Core = core;

			API_Coord c1 = element.wall.begC;
			API_Coord c2 = element.wall.endC;
			double height = element.wall.height;
			double bottomOffset = element.wall.bottomOffset;
			short homeStory = element.header.floorInd;
			short topStory = element.wall.relativeTopStory;

			API_Vector directionWall = API_Vector();
			directionWall.x = c2.x - c1.x;
			directionWall.y = c2.y - c1.y;
			double len = directionWall.x * directionWall.x + directionWall.y * directionWall.y;
			double lamda = 1.0 / std::sqrt(len);
			directionWall.x = directionWall.x * lamda;
			directionWall.y = directionWall.y * lamda;

			API_Vector perpendicular = API_Vector();
			perpendicular.x = directionWall.y;
			perpendicular.y = -directionWall.x;

			API_Coord cOut1 = API_Coord();
			cOut1.x = c1.x - perpendicular.x * sideCenter;
			cOut1.y = c1.y - perpendicular.y * sideCenter;
			API_Coord cOut2 = API_Coord();
			cOut2.x = c2.x - perpendicular.x * sideCenter;
			cOut2.y = c2.y - perpendicular.y * sideCenter;
			
			double totalMove = 0;
			API_Element newWall = {};
			API_ElementMemo newMemo = {};
			API_Attribute attribute = {};

			if (wallTypeLayer.Top2 > 0) 
			{
				totalMove += wallTypeLayer.Top2 / 1000 / 2;
				API_Coord t1 = API_Coord();
				t1.x = cOut1.x + perpendicular.x * totalMove;
				t1.y = cOut1.y + perpendicular.y * totalMove;

				API_Coord t2 = API_Coord();
				t2.x = cOut2.x + perpendicular.x * totalMove;
				t2.y = cOut2.y + perpendicular.y * totalMove;
				// create wall for top 2

				BNClear(newWall);
				BNClear(newMemo);

				newWall.header.type.typeID = API_WallID;
				ACAPI_Element_GetDefaults(&newWall, &newMemo);
				newWall.wall.begC = t1;
				newWall.wall.endC = t2;
				if (topStory == 0) 
				{
					newWall.wall.relativeTopStory = 0;
					newWall.wall.height = height;
				}
				else newWall.wall.relativeTopStory = topStory;
				newWall.wall.bottomOffset = bottomOffset;
				*newMemo.elemInfoString = *memo.elemInfoString;
				newWall.header.floorInd = homeStory;

				BNClear(attribute);
				attribute.header.typeID = API_CompWallID;
				strcpy(attribute.header.name, wallTypeLayer.Top2Name.c_str());
				err = ACAPI_Attribute_Search(&attribute.header);
				if (err == NoError)
				{
					newWall.wall.modelElemStructureType = API_CompositeStructure;
					newWall.wall.composite = attribute.compWall.head.index;
					ACAPI_Element_Create(&newWall, &newMemo);
				}
				else 
				{
					newWall.wall.modelElemStructureType = API_BasicStructure;
					newWall.wall.thickness = wallTypeLayer.Top2 / 1000;
					ACAPI_Element_Create(&newWall, &newMemo);
				}


				totalMove += wallTypeLayer.Top2 / 1000 / 2;

			}

			if (wallTypeLayer.Top1 > 0)
			{
				totalMove += wallTypeLayer.Top1 / 1000 / 2;
				API_Coord t1 = API_Coord();
				t1.x = cOut1.x + perpendicular.x * totalMove;
				t1.y = cOut1.y + perpendicular.y * totalMove;

				API_Coord t2 = API_Coord();
				t2.x = cOut2.x + perpendicular.x * totalMove;
				t2.y = cOut2.y + perpendicular.y * totalMove;


				// create wall for top 1
				BNClear(newWall);
				BNClear(newMemo);
				newWall.header.type.typeID = API_WallID;
				ACAPI_Element_GetDefaults(&newWall, &newMemo);
				newWall.wall.begC = t1;
				newWall.wall.endC = t2;
				if (topStory == 0)
				{
					newWall.wall.relativeTopStory = 0;
					newWall.wall.height = height;
				}
				else newWall.wall.relativeTopStory = topStory;
				newWall.wall.bottomOffset = bottomOffset;
				newWall.wall.thickness = wallTypeLayer.Top1 / 1000;
				*newMemo.elemInfoString = *memo.elemInfoString;
				newWall.header.floorInd = homeStory;
				
				BNClear(attribute);
				attribute.header.typeID = API_CompWallID;
				strcpy(attribute.header.name, wallTypeLayer.Top1Name.c_str());
				err = ACAPI_Attribute_Search(&attribute.header);
				if (err == NoError)
				{
					newWall.wall.modelElemStructureType = API_CompositeStructure;
					newWall.wall.composite = attribute.compWall.head.index;
					ACAPI_Element_Create(&newWall, &newMemo);
				}
				else
				{
					newWall.wall.modelElemStructureType = API_BasicStructure;
					newWall.wall.thickness = wallTypeLayer.Top1 / 1000;
					ACAPI_Element_Create(&newWall, &newMemo);
				}

				totalMove += wallTypeLayer.Top1 / 1000 / 2;

			}

			if (wallTypeLayer.Core > 0)
			{
				// Modify
				totalMove += wallTypeLayer.Core / 1000 / 2;
				API_Coord t1 = API_Coord();
				t1.x = cOut1.x + perpendicular.x * totalMove;
				t1.y = cOut1.y + perpendicular.y * totalMove;

				API_Coord t2 = API_Coord();
				t2.x = cOut2.x + perpendicular.x * totalMove;
				t2.y = cOut2.y + perpendicular.y * totalMove;

				// create wall for core
				element.wall.begC = t1;
				element.wall.endC = t2;

				API_Element mask;
				ACAPI_ELEMENT_MASK_CLEAR(mask);
				ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
				ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);

				if (element.wall.modelElemStructureType== API_CompositeStructure)
				{
					element.wall.modelElemStructureType = API_BasicStructure;
					element.wall.thickness = wallTypeLayer.Core / 1000;
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, thickness);
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, modelElemStructureType);
					ACAPI_Element_Change(&element, &mask,&memo,0,true);
				}
				else
				{
					element.wall.thickness = wallTypeLayer.Core / 1000;
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, thickness);
					ACAPI_Element_Change(&element, &mask, &memo, 0, true);
				}
				totalMove += wallTypeLayer.Core / 1000 / 2;
			}

			if (wallTypeLayer.Bottom1 > 0)
			{
				totalMove += wallTypeLayer.Bottom1 / 1000 / 2;
				API_Coord t1 = API_Coord();
				t1.x = cOut1.x + perpendicular.x * totalMove;
				t1.y = cOut1.y + perpendicular.y * totalMove;

				API_Coord t2 = API_Coord();
				t2.x = cOut2.x + perpendicular.x * totalMove;
				t2.y = cOut2.y + perpendicular.y * totalMove;

				// create wall for core
				BNClear(newWall);
				BNClear(newMemo);
				newWall.header.type.typeID = API_WallID;
				ACAPI_Element_GetDefaults(&newWall, &newMemo);
				newWall.wall.begC = t1;
				newWall.wall.endC = t2;
				if (topStory == 0)
				{
					newWall.wall.relativeTopStory = 0;
					newWall.wall.height = height;
				}
				else newWall.wall.relativeTopStory = topStory;
				newWall.wall.bottomOffset = bottomOffset;
				newWall.wall.thickness = wallTypeLayer.Bottom1 / 1000;
				*newMemo.elemInfoString = *memo.elemInfoString;
				newWall.header.floorInd = homeStory;
				
				BNClear(attribute);
				attribute.header.typeID = API_CompWallID;
				strcpy(attribute.header.name, wallTypeLayer.Bottom1Name.c_str());
				err = ACAPI_Attribute_Search(&attribute.header);
				if (err == NoError)
				{
					newWall.wall.modelElemStructureType = API_CompositeStructure;
					newWall.wall.composite = attribute.compWall.head.index;
					ACAPI_Element_Create(&newWall, &newMemo);
				}
				else
				{
					newWall.wall.modelElemStructureType = API_BasicStructure;
					newWall.wall.thickness = wallTypeLayer.Bottom1 / 1000;
					ACAPI_Element_Create(&newWall, &newMemo);
				}


				totalMove += wallTypeLayer.Bottom1 / 1000 / 2;
			}

			if (wallTypeLayer.Bottom2 > 0)
			{
				totalMove += wallTypeLayer.Bottom2 / 1000 / 2;
				API_Coord t1 = API_Coord();
				t1.x = cOut1.x + perpendicular.x * totalMove;
				t1.y = cOut1.y + perpendicular.y * totalMove;

				API_Coord t2 = API_Coord();
				t2.x = cOut2.x + perpendicular.x * totalMove;
				t2.y = cOut2.y + perpendicular.y * totalMove;

				// create wall for core
				BNClear(newWall);
				BNClear(newMemo);
				newWall.header.type.typeID = API_WallID;
				ACAPI_Element_GetDefaults(&newWall, &newMemo);
				newWall.wall.begC = t1;
				newWall.wall.endC = t2;
				if (topStory == 0)
				{
					newWall.wall.relativeTopStory = 0;
					newWall.wall.height = height;
				}
				else newWall.wall.relativeTopStory = topStory;
				newWall.wall.bottomOffset = bottomOffset;
				newWall.wall.thickness = wallTypeLayer.Bottom2 / 1000;
				*newMemo.elemInfoString = *memo.elemInfoString;
				newWall.header.floorInd = homeStory;
				
				BNClear(attribute);
				attribute.header.typeID = API_CompWallID;
				strcpy(attribute.header.name, wallTypeLayer.Bottom2Name.c_str());
				err = ACAPI_Attribute_Search(&attribute.header);
				if (err == NoError)
				{
					newWall.wall.modelElemStructureType = API_CompositeStructure;
					newWall.wall.composite = attribute.compWall.head.index;
					ACAPI_Element_Create(&newWall, &newMemo);
				}
				else
				{
					newWall.wall.modelElemStructureType = API_BasicStructure;
					newWall.wall.thickness = wallTypeLayer.Bottom2 / 1000;
					ACAPI_Element_Create(&newWall, &newMemo);
				}

				totalMove += wallTypeLayer.Bottom2 / 1000 / 2;
			}


		}
		

	}



	return NoError;
}
