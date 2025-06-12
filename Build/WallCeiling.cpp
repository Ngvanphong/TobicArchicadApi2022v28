#include "WallCeiling.h"
#include <algorithm>

API_Coord	SubCoord (API_Coord c1, API_Coord c2)

{
    API_Coord c;
    c.x = c1.x - c2.x;
    c.y = c1.y - c2.y;
    return c;
}

API_Coord	NormalizeCoord2D(const API_Coord& v)

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

double DistanceCoord2D(const API_Coord& v1, const API_Coord& v2) 
{
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

bool IsSamePoint(const API_Coord& v1, const API_Coord& v2)
{
    return  abs(v1.x - v2.x) < 0.0001 && abs(v1.y - v2.y) < 0.0001;
}


bool CheckParell(const API_Coord& v1, const API_Coord& v2)
{
    if((abs(v1.x)<0.0001 && abs(v1.y)<0.0001) || (abs(v2.x)<0.0001 && abs(v2.y)<0.0001)) return true;
    auto result = (v1.x * v2.x + v1.y * v2.y);
    return  abs(abs(result) - 1)< 0.001;
}



std::vector<API_Element> GetSelectedCeiling(GS::Array<API_Neig> selNeigs) 
{
    std::vector<API_Element> result = std::vector<API_Element>();
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
        if (err == NoError) {
            if (element.header.type.typeID = API_SlabID) {
                result.push_back(element);
            }
        }
    }
    return result;
}

bool CheckExistEndPoint(GS::Int32* pends,int index) 
{
    auto pointer = &(pends)[0];
    while (pointer!=nullptr)
    {
        auto i = *pointer;
        if (i == index) {
            return true;
        }
        else if(i!=0 && index < i) 
        {
            return false;
        }
        ++pointer;
    }
    return false;
}

std::vector<std::pair<API_Coord,API_Coord>> GetAllCoordOfFloor(API_Element element,API_ElementMemo* memo) 
{
    std::vector<std::pair<API_Coord, API_Coord>> result = std::vector<std::pair<API_Coord, API_Coord>>();
    GS::ErrCode err = ACAPI_Element_GetMemo(element.header.guid, memo);
    if (err == NoError) {
        auto allCoord = *memo->coords;
        auto allEnd = *memo->pends;
        auto countCoord = element.slab.poly.nCoords;
        for (auto i = 1; i < countCoord; i++) 
        {
            bool sameEndPoint = CheckExistEndPoint(allEnd, i);
            if (sameEndPoint) {
                continue;
            }
            else {
                std::pair<API_Coord, API_Coord> pair;
                pair.first = *(&allCoord[i]);
                pair.second = *(&allCoord[i + 1]);
                result.push_back(pair);
            }
        }
    }
    return result;
}



std::pair<API_Coord, API_Coord> LineMerge(std::pair<API_Coord, API_Coord> firstLine,
    std::pair<API_Coord, API_Coord> secondLine,
    bool& isMerge) 
{
    std::pair<API_Coord, API_Coord> result = std::pair<API_Coord, API_Coord>();
    auto v1 = NormalizeCoord2D(SubCoord(firstLine.second, firstLine.first));
    auto v2= NormalizeCoord2D(SubCoord(secondLine.second, secondLine.first));
    auto v3= NormalizeCoord2D(SubCoord(firstLine.first, secondLine.second));
    auto v4 = NormalizeCoord2D(SubCoord(firstLine.first, secondLine.first));
    if (CheckParell(v1, v2) && CheckParell(v1, v3) && CheckParell(v1, v4)) 
    {
        isMerge = true;
    }
    else 
    {
        isMerge = false;
        return result;
    }
    double maxDistance = 0;
    double d1 = DistanceCoord2D(firstLine.first, firstLine.second);
    double d2 = DistanceCoord2D(secondLine.first, secondLine.second);

    double d3 = DistanceCoord2D(firstLine.first, secondLine.first);
    double d4 = DistanceCoord2D(firstLine.first, secondLine.second);
    double d5 = DistanceCoord2D(firstLine.second, secondLine.first);
    double d6 = DistanceCoord2D(firstLine.second, secondLine.second);


    double dMax = std::max(std::max(std::max(d1, d2), std::max(d3, d4)), std::max(d5, d6)); //fix lai cho nay
    if (abs(dMax - d1 - d2) < 0.0003 || maxDistance >= (d1 + d2)) 
    {
        isMerge = false;
        return result;
    }

    API_Coord first= API_Coord();
    API_Coord second = API_Coord();
    bool isFirst = false;
    bool isSecond = false;
    if (abs(d3 + d4 - d2) < 0.0003) 
    {
        first = firstLine.first;
        isFirst = true;
    }
    if (abs(d5 + d6 - d2) < 0.0003)
    {
        if (isFirst == false) {
            first = firstLine.second;
            isFirst = true;
        }
        else {
            second = firstLine.second;
            isSecond = true;
        }
    }

    if (abs(d3 + d5 - d1) < 0.0003) 
    {
        if (isFirst == false) {
            first = secondLine.first;
            isFirst = true;
        }
        else if(isSecond==false)
        {
            if (!IsSamePoint(first, second)) 
            {
                second = secondLine.first;
                isSecond = true;
            }
            
        }
    }

    if (abs(d4 + d6 - d1) < 0.0003)
    {
        if (isFirst == false) 
        {
            first = secondLine.second;
            isFirst = true;
        }
        else if(isSecond==false)
        {
            if (!IsSamePoint(first, second)) 
            {
                second = secondLine.second;
                isSecond = true;
            }
        }
    }
    
    isMerge = isFirst && isSecond;
    result.first = first;
    result.second = second;
    return result;

}


bool IsExistData(std::vector<LineWallData> arrayData, LineWallData lineWall) {
    for (auto lineItem : arrayData)
    {
        if ((IsSamePoint(lineWall.start, lineItem.start) && IsSamePoint(lineWall.end, lineItem.end))
            || (IsSamePoint(lineWall.start, lineItem.end) && IsSamePoint(lineWall.end, lineItem.start))) 
       {
            return true;
            break;
       }
    }
    return false;
}

std::vector<LineWallData> GetLineWallData(std::vector<API_Element> listCeiling)
{
    API_ElementMemo memo;
    BNZeroMemory(&memo, sizeof(API_ElementMemo));
    API_StoryInfo storyInfo;
    BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
    ACAPI_ProjectSetting_GetStorySettings(&storyInfo);

    std::vector<LineWallData> listLineWallDataResult = std::vector<LineWallData>();
    auto listCeilingCheck = std::vector<API_Element>(listCeiling);
    for (auto ceiling : listCeiling) 
    {
        API_StoryType storyElement;
        API_StoryType* pointerIterator = &(*storyInfo.data)[0];
        double thickness = ceiling.slab.thickness;
        double offsetLevel = ceiling.slab.level;
        
        for (int id = storyInfo.firstStory; id <= storyInfo.lastStory; id++) {
            if (pointerIterator != nullptr) {
                if (id == ceiling.header.floorInd) {
                    storyElement = *pointerIterator;
                    break;
                }
            }
            ++pointerIterator;
        }
        double zCeiling = storyElement.level + offsetLevel;

        std::vector<std::pair<API_Coord, API_Coord>> edgeFloor = GetAllCoordOfFloor(ceiling, &memo);

        for (std::pair<API_Coord, API_Coord> pair:edgeFloor) 
        {
            for (auto ceilingCheck : listCeilingCheck)
            {
                API_StoryType storyElementCheck;
                API_StoryType* pointerIteratorCheck = &(*storyInfo.data)[0];
                double thicknessCheck = ceilingCheck.slab.thickness;
                double offsetLevelCheck = ceilingCheck.slab.level;
                for (int idCheck = storyInfo.firstStory; idCheck <= storyInfo.lastStory; idCheck++) {
                    if (pointerIteratorCheck != nullptr) {
                        if (idCheck == ceilingCheck.header.floorInd) {
                            storyElementCheck = *pointerIteratorCheck;
                            break;
                        }
                    }
                    ++pointerIteratorCheck;
                }
                double zCeilingCheck = storyElementCheck.level + offsetLevelCheck;
                if (abs(zCeilingCheck - zCeiling) < 0.001) continue;
                std::vector<std::pair<API_Coord, API_Coord>> edgeFloorCheck = GetAllCoordOfFloor(ceilingCheck, &memo);
                for (std::pair<API_Coord, API_Coord> pairCheck : edgeFloorCheck)
                {
                    bool isMerge = false;
                    std::pair<API_Coord, API_Coord> pairMerge = LineMerge(pair, pairCheck, isMerge);
                    if (isMerge) 
                    {
                        
                        LineWallData lineWallElement = LineWallData();
                        bool isCeilingBottom = zCeiling < zCeilingCheck;
                        if (isCeilingBottom) {
                            auto coordDirection = NormalizeCoord2D(SubCoord(pair.second, pair.first));
                            lineWallElement.directionMove = API_Vector{ coordDirection.y,-coordDirection.x };
                        }
                        else 
                        {
                            auto coordDirection = NormalizeCoord2D(SubCoord(pairCheck.second, pairCheck.first));
                            lineWallElement.directionMove = API_Vector{ coordDirection.y,-coordDirection.x };
                        }
                        lineWallElement.start = pairMerge.first;
                        lineWallElement.end = pairMerge.second;

                        if (isCeilingBottom) {
                            lineWallElement.storyBottom = storyElement;
                            lineWallElement.thickFloorBottom = thickness;
                            lineWallElement.offsetBottom = offsetLevel;

                            lineWallElement.storyTop = storyElementCheck;
                            lineWallElement.thickFloorTop = thicknessCheck;
                            lineWallElement.offsetTop = offsetLevelCheck;
                        }
                        else {
                            lineWallElement.storyBottom = storyElementCheck;
                            lineWallElement.thickFloorBottom = thicknessCheck;
                            lineWallElement.offsetBottom = offsetLevelCheck;

                            lineWallElement.storyTop = storyElement;
                            lineWallElement.thickFloorTop = thickness;
                            lineWallElement.offsetTop = offsetLevel;
                        }

                        if (!IsExistData(listLineWallDataResult,lineWallElement)) 
                        {
                            listLineWallDataResult.push_back(lineWallElement);
                        }
                    }
                }
            }
        }
       
    }
    return listLineWallDataResult;
}

GS::ErrCode CreateWallFromToCeiling()
{
    std::string nameWall = "CeilingWall";
    API_Element element = {};
    API_ElementMemo memo = {};
    double withWall = 0.1/2;
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> allFloor = GetSelectedCeiling(selNeigs);
    if (allFloor.size() == 0) return NoError;
    std::vector<LineWallData> lineWallData = GetLineWallData(allFloor);
    if (lineWallData.size() == 0) return NoError;

    for (LineWallData lineWall : lineWallData)
    {
        BNClear(element);
        BNClear(memo);
        element.header.type.typeID = API_WallID;
        ACAPI_Element_GetDefaults(&element, &memo);
        withWall = element.wall.thickness/2;
        API_Coord start = API_Coord{ lineWall.start.x - lineWall.directionMove.x * withWall, lineWall.start.y - lineWall.directionMove.y * withWall };
        API_Coord end = API_Coord{ lineWall.end.x - lineWall.directionMove.x * withWall, lineWall.end.y - lineWall.directionMove.y * withWall };
        if (IsSamePoint(start, end)) {
            continue;
        }
        element.wall.begC = start;
        element.wall.endC = end;
        element.wall.bottomOffset = lineWall.offsetBottom + lineWall.thickFloorBottom;
        element.header.floorInd = lineWall.storyBottom.index;
        element.wall.relativeTopStory = 0;
        element.wall.height = (lineWall.storyTop.level + lineWall.offsetTop+ lineWall.thickFloorTop
            - lineWall.storyBottom.level - lineWall.offsetBottom - lineWall.thickFloorBottom);
        element.wall.referenceLineLocation = APIWallRefLine_Center;
        memo.elemInfoString = new GS::UniString(nameWall.c_str());
        ACAPI_Element_Create(&element, &memo);
    }

    ACAPI_DisposeElemMemoHdls(&memo);
    return NoError;
}
