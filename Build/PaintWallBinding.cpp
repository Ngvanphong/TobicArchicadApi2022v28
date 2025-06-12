#include "PaintWallBinding.h"
#include <codecvt>
#include <string>
#include <locale>

std::vector<API_Element> SelectHabaki(GS::Array<API_Neig> selNeigs) {
    std::vector<API_Element> result;
    API_SelectionInfo* selectionInfo = new API_SelectionInfo();
    GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
    if (selectionInfo->typeID == API_SelEmpty) {
        delete selectionInfo;
        return result;
    }
    for (int i = 0; i < selectionInfo->sel_nElem; i++)
    {
        API_Element element;
        element.header.guid = selNeigs[i].guid;
        err = ACAPI_Element_Get(&element);
        if (err == NoError && element.header.type.typeID == API_WallID)
        {
            if(element.wall.thickness==0.01)
            {
                result.push_back(element);
            }
            
        }
    }
    delete selectionInfo;
    return result;
}

// create material "MaxMaterial" has priority is Max;
std::wstring wStringAttributeMaterialPaint(const char* utf8Bytes)
{
    //setup converter
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(utf8Bytes);
}

std::wstring ToWStringPaint(const GS::UniString& str)
{
    static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
    return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}


GS::ErrCode PaintWallByHabaki(void)
{
    const std::wstring nameMaxMaterial = L"MaxMaterial";
    std::wstring volumnName = L"Volume (Net)";
    API_Attribute attrib;
    bool hasAttribute = false;
    GS::UInt32 count, i;
    API_AttributeIndex indexMaxMaxMaterial;
    ACAPI_Attribute_GetNum(API_BuildingMaterialID, count);
    for (i = 1; i <= count; i++) {
        BNZeroMemory(&attrib, sizeof(API_Attribute));
        attrib.header.typeID = API_BuildingMaterialID;
        attrib.header.index = ACAPI_CreateAttributeIndex(i);
        ACAPI_Attribute_Get(&attrib);
        if (wStringAttributeMaterialPaint(attrib.header.name) == nameMaxMaterial)
        {
            hasAttribute = true;
            indexMaxMaxMaterial = ACAPI_CreateAttributeIndex(i);
            break;
        }
    }
    if (!hasAttribute) return NoError;

    API_StoryInfo storyInfo;
    BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
    ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
    //ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
    
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> wallHabaki = SelectHabaki(selNeigs);
    if (wallHabaki.size() == 0) return NoError;
    GS::Array<API_Guid> guidAllFloor = GS::Array<API_Guid>();
    GS::Array<API_Guid> guidOnLayer = GS::Array< API_Guid>();
    auto currentHabakiLevel = wallHabaki[0].header.floorInd;
    ACAPI_Element_GetElemList(API_SlabID, &guidOnLayer, APIFilt_OnVisLayer);
    API_Element refElement = {};
    for (GS::Array<API_Guid>::ConstIterator it = guidOnLayer.Enumerate(); it != nullptr; ++it) {
        refElement.header.guid = *it;
        auto errGet=ACAPI_Element_Get(&refElement);
        if (errGet == NoError) 
        {
            if (currentHabakiLevel <= refElement.header.floorInd && refElement.header.floorInd <= currentHabakiLevel + 1) 
            {
                guidAllFloor.Push(*it);
            }
        }
    }
    if (guidAllFloor.GetSize() == 0) return NoError;
    GS::Array<GuidVolumePaint> arrayGuidVolume = GS::Array<GuidVolumePaint>();
    API_Property property;
    API_PropertyDefinition propertyDefinitionSlab;
    bool hasParameter = false;
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    for (API_Guid id : guidAllFloor)
    {
        if (!hasParameter) {
            ACAPI_Element_GetPropertyDefinitions(id, API_PropertyDefinitionFilter_All, propertyDefinitions);
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPaint(propertyDefinition.name);
                if (name == volumnName)
                {
                    ACAPI_Element_GetPropertyValue(id, propertyDefinition.guid, property);
                    propertyDefinitionSlab = propertyDefinition;
                    break;
                }
            }
        }
        ACAPI_Element_GetPropertyValue(id, propertyDefinitionSlab.guid, property);
        double originVolumne = property.value.singleVariant.variant.doubleValue;
        GuidVolumePaint guidVolume = GuidVolumePaint();
        guidVolume.guid = id;
        guidVolume.volumne = originVolumne;
        arrayGuidVolume.Push(guidVolume);
    }

    API_Element markWall;
    ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, height);

    API_Element wallElement = {};
    API_ElementMemo wallMemo = {};
    GS::Array<API_Neig> neigCreated = GS::Array <API_Neig>();
    for (auto wall : wallHabaki)
    {
        API_Coord start = wall.wall.begC;
        API_Coord end = wall.wall.endC;
        auto refIndex = wall.wall.referenceLineLocation;
        auto levelIndex = wall.header.floorInd;
        double bottomOffset = wall.wall.bottomOffset + wall.wall.height;

        API_StoryType storyBabaki;
        API_StoryType* pointerIteratorHabaki = &(*storyInfo.data)[0];
        for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
            if (pointerIteratorHabaki != nullptr) {
                if (id == wall.header.floorInd) {
                    storyBabaki = *pointerIteratorHabaki;
                    break;
                }
            }
            ++pointerIteratorHabaki;
        }
        
        wallElement.header.type.typeID = API_WallID;
        ACAPI_Element_GetDefaults(&wallElement, &wallMemo);
        wallElement.wall.thickness = 0.001;
        wallElement.wall.begC = start;
        wallElement.wall.endC = end;
        wallElement.wall.referenceLineLocation = refIndex;
        wallElement.header.floorInd = levelIndex;
        wallElement.wall.bottomOffset = bottomOffset;
        wallElement.wall.height = 5;
        wallElement.wall.buildingMaterial = indexMaxMaxMaterial;
        ACAPI_Element_Create(&wallElement, &wallMemo);

        API_Neig neigElement;
        BNZeroMemory(&neigElement, sizeof(API_Neig));
        neigElement.guid = wallElement.header.guid;
        neigElement.neigID = APINeig_Wall;
        neigElement.inIndex = 1;
        neigCreated.Push(neigElement);

        std::vector<double> listHeightPaint = std::vector<double>();
        double heightPaint = 0;
        for (auto idSlab : guidAllFloor)
        {
            API_Element slab = {};
            slab.header.guid = idSlab;
            ACAPI_Element_Get(&slab);
            ACAPI_Element_GetPropertyValue(idSlab, propertyDefinitionSlab.guid, property);
            double volumeCheck = property.value.singleVariant.variant.doubleValue;
            double volumeOrigin = 0;
            for (auto item : arrayGuidVolume) {
                if (item.guid == idSlab) { volumeOrigin = item.volumne; break; }
            }
            if (abs(volumeOrigin - volumeCheck) > 0.0000001)
            {
                API_StoryType currentStory;
                API_StoryType* pointerIterator = &(*storyInfo.data)[0];
                for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
                    if (pointerIterator != nullptr) {
                        if (id == slab.header.floorInd) {
                            currentStory = *pointerIterator;
                            break;
                        }
                    }
                    ++pointerIterator;
                }
                heightPaint = currentStory.level + slab.slab.level - slab.slab.thickness
                    - (storyBabaki.level + wall.wall.bottomOffset + wall.wall.height);
                if (heightPaint > 1) {
                    listHeightPaint.push_back(heightPaint);
                }
            }
        }
        double heightMin = heightPaint;
        for (auto item : listHeightPaint) {
            if (item < heightMin) {
                heightMin = item;
            }
        }
        if (heightMin > 0) 
        {
            wallElement.wall.height = heightMin;
            ACAPI_Element_Change(&wallElement, &markWall, nullptr, 0UL, true);
        }
    }
    ACAPI_DisposeElemMemoHdls(&wallMemo);
    if (neigCreated.GetSize() > 0)
    {
        ACAPI_Selection_DeselectAll();
        ACAPI_Selection_Select(neigCreated, true);
    }

    return NoError;
}
