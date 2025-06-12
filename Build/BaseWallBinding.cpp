#include "BaseWallBinding.h"
/// <summary>
/// Nho cho vat lieu cua wall len cao de wall cat san
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
bool IsSamePointWall(const API_Coord& v1, const API_Coord& v2)
{
    return  abs(v1.x - v2.x) < 0.0001 && abs(v1.y - v2.y) < 0.0001;
}
std::vector<API_Element> GetZoneSelected(GS::Array<API_Neig> selNeigs) 
{
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
        if (err == NoError && element.header.type.typeID == API_ZoneID)
        {
            result.push_back(element);
        }
    }
    delete selectionInfo;
    return result;
}

std::vector<API_Element> GetSlabSelected(GS::Array<API_Neig> selNeigs)
{
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
        if (err == NoError && element.header.type.typeID == API_SlabID)
        {
            result.push_back(element);
        }
    }
    delete selectionInfo;
    return result;
}

std::wstring ToWStringWallSlab(const GS::UniString& str)
{
    static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
    return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode CreateWallBase(void)
{
    API_Element markWall;
    ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, bottomOffset);
    ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, height);
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> allSlab = GetSlabSelected(selNeigs);
    if (allSlab.size() == 0) return NoError;
    API_ElementMemo memoSlab = {};
    GS::Array<API_Neig> neigCreated = GS::Array < API_Neig>();
    for (auto slab : allSlab)
    {
        BNClear(memoSlab);
        ACAPI_Element_GetMemo(slab.header.guid, &memoSlab);
        auto coords = memoSlab.coords;
        auto pends = memoSlab.pends;
        std::vector<std::vector<API_Coord>> arrayArrayPointBoundary = std::vector<std::vector<API_Coord>>();
        auto pointerArray = &(*pends)[1];
        GS::Int32 previousIndex = 0;
        auto interator = &(*coords)[0];
        int countPoly = slab.slab.poly.nSubPolys;
        for (auto poly = 1; poly <= countPoly; poly++)
        {
            auto indexEnd = *pointerArray;
            std::vector<API_Coord> arrayPointBoundary = std::vector<API_Coord>();
            for (int i = previousIndex; i < indexEnd; i++)
            {
                interator++;
                if (i >= previousIndex + 1)
                {
                    arrayPointBoundary.push_back(*interator);
                }
               
            }
            arrayArrayPointBoundary.push_back(arrayPointBoundary);
            previousIndex = indexEnd;
            ++pointerArray;
        }


        API_Element wallElement = {};
        API_ElementMemo wallMemo = {};
        for (const auto& arrayPointBoundary : arrayArrayPointBoundary)
        {
            for (int i = 0; i < arrayPointBoundary.size(); i++)
            {
                BNClear(wallElement);
                BNClear(wallMemo);
                API_Coord first = arrayPointBoundary[i];
                API_Coord second;
                if (i == arrayPointBoundary.size() - 1) {
                    second = arrayPointBoundary[0];
                }
                else {
                    second = arrayPointBoundary[i + 1];
                }

                API_Neig neigElement;
                if (!IsSamePointWall(first, second))
                {
                    wallElement.header.type.typeID = API_WallID;
                    ACAPI_Element_GetDefaults(&wallElement, &wallMemo);
                    wallElement.wall.thickness = 0.01;
                    wallElement.wall.begC = first;
                    wallElement.wall.endC = second;
                    wallElement.header.floorInd = slab.header.floorInd;
                    wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
                    wallElement.wall.relativeTopStory = 0;
                    ACAPI_Element_Create(&wallElement, &wallMemo);
                    BNZeroMemory(&neigElement, sizeof(API_Neig));
                    neigElement.guid = wallElement.header.guid;
                    neigElement.neigID = APINeig_Wall;
                    neigElement.inIndex = 1;
                    neigCreated.Push(neigElement);
                    wallElement.wall.bottomOffset = slab.slab.level;
                    wallElement.wall.height = 0.06;
                    ACAPI_Element_Change(&wallElement, &markWall, nullptr, 0UL, true);
                }
            }
        }

        ACAPI_DisposeElemMemoHdls(&wallMemo);

    }

    if (neigCreated.GetSize() > 0)
    {
        ACAPI_Selection_DeselectAll();
        ACAPI_Selection_Select(neigCreated, true);
    }
    ACAPI_DisposeElemMemoHdls(&memoSlab);

    return NoError;
}


GS::ErrCode CreateWallBaseByZone(void)
{
    std::wstring volumnName = L"Volume (Net)";
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    API_Element markWall;
    ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, bottomOffset);
    ACAPI_ELEMENT_MASK_SET(markWall, API_WallType, height);
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> allZone = GetZoneSelected(selNeigs);
    if (allZone.size() == 0) return NoError;
    GS::Array<API_Guid> guidAllFloor = GS::Array<API_Guid>();
    GS::Array<API_Guid> guidOnLayer = GS::Array< API_Guid>();
    ACAPI_Element_GetElemList(API_SlabID, &guidOnLayer, APIFilt_OnActFloor);
    for (GS::Array<API_Guid>::ConstIterator it = guidOnLayer.Enumerate(); it != nullptr; ++it) {
        if (ACAPI_Element_Filter(*it, APIFilt_OnVisLayer)) {
            guidAllFloor.Push(*it);
        }
    }

    if (guidAllFloor.GetSize() == 0) return NoError;

    GS::Array<GuidVolume> arrayGuidVolume = GS::Array<GuidVolume>();
    API_Property property;
    API_PropertyDefinition propertyDefinitionSlab;
    bool hasParameter = false;
    for (API_Guid id : guidAllFloor) 
    {
        if (!hasParameter) {
            ACAPI_Element_GetPropertyDefinitions(id, API_PropertyDefinitionFilter_All, propertyDefinitions);
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringWallSlab(propertyDefinition.name);
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
        GuidVolume guidVolume = GuidVolume();
        guidVolume.guid = id;
        guidVolume.volumne = originVolumne;
        arrayGuidVolume.Push(guidVolume);
    }

    API_Element firstFloor = {};
    firstFloor.header.guid = guidAllFloor[0];
    ACAPI_Element_Get(&firstFloor);

    API_StoryInfo storyInfo;
    BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
    ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
    API_StoryType currentStory;
    API_StoryType* pointerIterator = &(*storyInfo.data)[0];
    for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
        if (pointerIterator != nullptr) {
            if (id == firstFloor.header.floorInd) {
                currentStory = *pointerIterator;
                break;
            }
        }
        ++pointerIterator;
    }


    API_ElementMemo memoZone = {};
    GS::Array<API_Neig> neigCreated = GS::Array < API_Neig>();
    for (auto zone : allZone) 
    {
      
        BNClear(memoZone);
        ACAPI_Element_GetMemo(zone.header.guid, &memoZone);
        auto coords = memoZone.coords;
        auto pends = memoZone.pends;
        auto countCoord = zone.zone.poly.nCoords;
        auto firstEnd = (*pends)[1];
        std::vector<API_Coord> arrayPointBoundary = std::vector<API_Coord>();
        auto interator = &(*coords)[0];
        for (int i = 1; i < countCoord; i++)
        {
            interator++;
            if (i <= firstEnd)
            {
                arrayPointBoundary.push_back(*interator);
            }
        }

        API_Element wallElement = {};
        API_ElementMemo wallMemo = {};
        
        for (int i = 0; i < arrayPointBoundary.size(); i++) 
        {
            BNClear(wallElement);
            BNClear(wallMemo);
            API_Coord first = arrayPointBoundary[i];
            API_Coord second;
            if (i == arrayPointBoundary.size() - 1) {
                second = arrayPointBoundary[0];
            }
            else {
               second = arrayPointBoundary[i+1];
            }
            
            API_Neig neigElement;
            if (!IsSamePointWall(first, second))
            {
                wallElement.header.type.typeID = API_WallID;
                ACAPI_Element_GetDefaults(&wallElement, &wallMemo);
                wallElement.wall.thickness = 0.01;
                wallElement.wall.begC = first;
                wallElement.wall.endC = second;
                wallElement.header.floorInd = currentStory.index;
                wallElement.wall.bottomOffset = -1;
                wallElement.wall.referenceLineLocation = APIWallRefLine_Outside;
                wallElement.wall.relativeTopStory = 0;
                wallElement.wall.height = 2;
                ACAPI_Element_Create(&wallElement, &wallMemo);
                BNZeroMemory(&neigElement, sizeof(API_Neig));
                neigElement.guid = wallElement.header.guid;
                neigElement.neigID = APINeig_Wall;
                neigElement.inIndex = 1;
                neigCreated.Push(neigElement);

                std::vector<double> listOffsetBottomSlab = std::vector<double>();
                double offsetBottomSlab = 0;
                bool isCreateWall = true;
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
                    if (abs(volumeOrigin - volumeCheck) > 0.000001)
                    {
                        offsetBottomSlab = slab.slab.level;
                        listOffsetBottomSlab.push_back(offsetBottomSlab);
                        break;
                    }
                }

                double topOffset = offsetBottomSlab;
                for (auto item : listOffsetBottomSlab) {
                    if (item > topOffset) {
                        topOffset = item;
                    }
                }
                wallElement.wall.bottomOffset = topOffset;
                wallElement.wall.height = 0.06;
                ACAPI_Element_Change(&wallElement, &markWall, nullptr, 0UL, true);
            }
        }

        ACAPI_DisposeElemMemoHdls(&wallMemo);

    }
    if (neigCreated.GetSize() > 0)
    {
        ACAPI_Selection_DeselectAll();
        ACAPI_Selection_Select(neigCreated, true);
    }
    ACAPI_DisposeElemMemoHdls(&memoZone);

    return NoError;
}
