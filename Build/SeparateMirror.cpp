#include "SeparateMirror.h"

std::vector<API_Element> GetWallMirror(GS::Array<API_Neig> selNeigs)
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
        if (err == NoError && element.header.type.typeID == API_WallID)
        {
            result.push_back(element);
        }
    }
    delete selectionInfo;
    return result;
}
bool IsSamePoint2D(const API_Coord& v1, const API_Coord& v2)
{
    return  abs(v1.x - v2.x) < 0.0001 && abs(v1.y - v2.y) < 0.0001;
}

API_Vector NormalizeVector2D(const API_Vector& v)

{
    API_Vector ret = v;
    double len = v.x * v.x + v.y * v.y;
    if (len > 1e-10) {
        double lambda = 1.0 / sqrt(len);
        ret.x = ret.x * lambda;
        ret.y = ret.y * lambda;
    }
    return ret;
}

double DistanceCoord2(const API_Coord& v1, const API_Coord& v2)
{
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

bool CheckParellVector(const API_Vector& v1, const API_Vector& v2)
{
    if ((abs(v1.x) < 0.0001 && abs(v1.y) < 0.0001) || (abs(v2.x) < 0.0001 && abs(v2.y) < 0.0001)) return false;
    auto result = (v1.x * v2.x + v1.y * v2.y);
    return  abs(abs(result) - 1) < 0.000000000000001;
}

void RemoveElementInList(std::vector<API_Element>& arrayElement, API_Element elementRemove) {
    int index = -1;
    if (arrayElement.empty()) return;
    for (int i = 0; i < arrayElement.size(); i++) {
        if (arrayElement[i].header.guid == elementRemove.header.guid) {
            index = i;
            break;
       }
    }
    if (index != -1) arrayElement.erase(std::next(arrayElement.begin(), index));
}

void SortListElement(std::vector<API_Element>& arraySort) 
{
    API_Coord begin = arraySort[0].wall.begC;
    API_Coord end = arraySort[0].wall.endC;
    API_Vector directionItem = API_Vector();
    directionItem.x = end.x - begin.x;
    directionItem.y = end.y - begin.y;
    directionItem = NormalizeVector2D(directionItem);
    
    double dotX = abs(abs(directionItem.x * 1 + directionItem.y * 0) - 1);
    bool isX = !(dotX < 0.0000001);

    API_Element tem;
    for (int i = 0; i < arraySort.size(); i++)
    {
        for (int j = i+1; j < arraySort.size(); j++) 
        {
            API_Coord sItem = arraySort[i].wall.begC;
            API_Coord sNextItem = arraySort[j].wall.begC;
            if (isX) {
                if (sItem.x > sNextItem.x) {
                    tem = arraySort[j];
                    arraySort[j] = arraySort[i];
                    arraySort[i] = tem;
                }
            }
            else {
                if (sItem.y > sNextItem.y) {
                    tem = arraySort[j];
                    arraySort[j] = arraySort[i];
                    arraySort[i] = tem;
                }
            }
        }
    }
}

std::pair<API_Coord, API_Coord> GetFirtLineWall(std::vector<API_Element> listWallSort,API_Vector& normalWall) 
{
    std::pair<API_Coord, API_Coord> result = std::pair<API_Coord, API_Coord>();
    API_Coord begin = listWallSort[0].wall.begC;
    API_Coord end = listWallSort[0].wall.endC;
    API_Vector directionItem = API_Vector();
    directionItem.x = end.x - begin.x;
    directionItem.y = end.y - begin.y;
    directionItem = NormalizeVector2D(directionItem);

    API_Vector perpendicular = API_Vector();
    perpendicular.x = directionItem.y;
    perpendicular.y = -directionItem.x;

    double mainThick = listWallSort[0].wall.thickness;

    API_Coord left = API_Coord();
    left.x = begin.x + perpendicular.x * mainThick / 2;
    left.y = begin.y + perpendicular.y * mainThick / 2;
    API_Coord leftEnd = API_Coord();
    leftEnd.x = end.x + perpendicular.x * mainThick / 2;
    leftEnd.y = end.y + perpendicular.y * mainThick / 2;

    API_Coord right = API_Coord();
    right.x = begin.x - perpendicular.x * mainThick / 2;
    right.y = begin.y - perpendicular.y * mainThick / 2;
    API_Coord rightEnd = API_Coord();
    rightEnd.x = end.x - perpendicular.x * mainThick / 2;
    rightEnd.y = end.y - perpendicular.y * mainThick / 2;

    double dotX = abs(abs(directionItem.x * 1 + directionItem.y * 0) - 1);
    bool isX = !(dotX < 0.0000001);
    if (isX) 
    {
        if (left.x < right.x) {
            result.first = left;
            result.second = leftEnd;
            normalWall.x = -perpendicular.x;
            normalWall.y = -perpendicular.y;
        }
        else {
            result.first = right;
            result.second = rightEnd;
            normalWall.x = perpendicular.x;
            normalWall.y = perpendicular.y;
        }
    }
    else 
    {
        if (left.y < right.y) {
            result.first = left;
            result.second = leftEnd;
            normalWall.x = -perpendicular.x;
            normalWall.y = -perpendicular.y;
        }
        else {
            result.first = right;
            result.second = rightEnd;
            normalWall.x = perpendicular.x;
            normalWall.y = perpendicular.y;
        }
    }

    return result;

}

double GetTotalThickness(std::vector<API_Element> arrayElement) {
    double totalThickness = 0;
    for (API_Element ele : arrayElement) {
        totalThickness += ele.wall.thickness;
    }
    return totalThickness;
}

void GetTreeBesideElement(std::vector<API_Element>& resultGroup,std::vector<API_Element>& sourceElement, API_Element currentItem)
{
    API_Coord c1 = currentItem.wall.begC;
    API_Coord c2 = currentItem.wall.endC;
    API_Coord centerMain = API_Coord{ (c2.x + c1.x) / 2,(c2.y + c1.y) / 2 };
    double mainThick = currentItem.wall.thickness;
    API_Vector direction = API_Vector();
    direction.x = c2.x - c1.x;
    direction.y = c2.y - c1.y;
    direction = NormalizeVector2D(direction);

    API_Vector perpendicular = API_Vector();
    perpendicular.x = direction.y;
    perpendicular.y = -direction.x;

    API_Coord left = API_Coord();
    left.x = c1.x + perpendicular.x * mainThick / 2;
    left.y = c1.y + perpendicular.y * mainThick / 2;
    API_Coord leftEnd = API_Coord();
    leftEnd.x = c2.x + perpendicular.x * mainThick / 2;
    leftEnd.y = c2.y + perpendicular.y * mainThick / 2;



    API_Coord right = API_Coord();
    right.x = c1.x - perpendicular.x * mainThick / 2;
    right.y = c1.y - perpendicular.y * mainThick / 2;
    API_Coord rightEnd = API_Coord();
    rightEnd.x = c2.x - perpendicular.x * mainThick / 2;
    rightEnd.y = c2.y - perpendicular.y * mainThick / 2;
    if (sourceElement.empty()) {
       resultGroup.push_back(currentItem);
       return;
    } 
    RemoveElementInList(sourceElement, currentItem);
    if (sourceElement.empty()) 
    {
       resultGroup.push_back(currentItem);
       return;
    }
    for (auto item : sourceElement) 
    {
        double itemThick = item.wall.thickness;
        API_Coord c1Item = item.wall.begC;
        API_Coord c2Item = item.wall.endC;
        API_Coord centerItem = API_Coord{ (c1Item.x + c2Item.x) / 2,(c1Item.y + c2Item.y) / 2 };
        API_Vector directionItem = API_Vector();
        directionItem.x = c2Item.x - c1Item.x;
        directionItem.y = c2Item.y - c1Item.y;
        directionItem = NormalizeVector2D(directionItem);
        if (abs(abs(direction.x * directionItem.x + direction.y * directionItem.y) - 1) < 0.0001)
        {
            double d = DistanceCoord2(centerMain, centerItem);
            if (d <= 1.5)
            {
                API_Coord leftItem = API_Coord();
                leftItem.x = c1Item.x + perpendicular.x * itemThick / 2;
                leftItem.y = c1Item.y + perpendicular.y * itemThick / 2;
                API_Coord leftItemEnd = API_Coord();
                leftItemEnd.x = c2Item.x + perpendicular.x * itemThick / 2;
                leftItemEnd.y = c2Item.y + perpendicular.y * itemThick / 2;

                API_Coord rightItem = API_Coord();
                rightItem.x = c1Item.x - perpendicular.x * itemThick / 2;
                rightItem.y = c1Item.y - perpendicular.y * itemThick / 2;

                API_Vector leftLeft = NormalizeVector2D(API_Vector{ left.x - leftItem.x,left.y - leftItem.y });
                API_Vector leftLeftEnd = NormalizeVector2D(API_Vector{ leftEnd.x - leftItem.x,leftEnd.y - leftItem.y });

                API_Vector leftRight = NormalizeVector2D(API_Vector{ left.x - rightItem.x,left.y - rightItem.y });
                API_Vector leftRightEnd = NormalizeVector2D(API_Vector{ leftEnd.x - rightItem.x,leftEnd.y - rightItem.y });


                API_Vector rightLeft = NormalizeVector2D(API_Vector{ right.x - leftItem.x,right.y - leftItem.y });
                API_Vector rightLeftEnd = NormalizeVector2D(API_Vector{ rightEnd.x - leftItem.x,rightEnd.y - leftItem.y });

                API_Vector rightRight = NormalizeVector2D(API_Vector{ right.x - rightItem.x,right.y - rightItem.y });
                API_Vector rightRightEnd = NormalizeVector2D(API_Vector{ rightEnd.x - rightItem.x,rightEnd.y - rightItem.y });

                if (CheckParellVector(direction,leftLeft) || CheckParellVector(direction, leftLeftEnd)
                    || CheckParellVector(direction, leftRight)|| CheckParellVector(direction, leftRightEnd) 
                    || CheckParellVector(direction, rightLeft) || CheckParellVector(direction, rightLeftEnd)||
                    CheckParellVector(direction, rightRight) || CheckParellVector(direction, rightRightEnd))
                {
                    std::vector<API_Element> newSource = std::vector<API_Element>(sourceElement);
                    GetTreeBesideElement(resultGroup, newSource, item);
                }
            }
        }
    }
    resultGroup.push_back(currentItem);

}



GS::ErrCode SeparateMirrorWall(void)
{
    GS::Array<API_Neig> selNeigs;
    std::vector<API_Element> allWalls = GetWallMirror(selNeigs);
    if (allWalls.size() == 0) return NoError;

    std::vector<std::vector<API_Element>> groupWall = std::vector<std::vector<API_Element>>();
    
    auto allWallCopys = std::vector<API_Element>{ allWalls };
    while (allWallCopys.size()>0) 
    {
        std::vector<API_Element> groupItem = std::vector<API_Element>();
        GetTreeBesideElement(groupItem, allWallCopys, allWallCopys[0]);
        for (auto item : groupItem) {
            RemoveElementInList(allWallCopys, item);
        }
        if (groupItem.size() > 0) groupWall.push_back(groupItem);
    }

    if (groupWall.size() == 0) return NoError;

    for (auto groupItem : groupWall) 
    {
        SortListElement(groupItem);
        API_Vector moveDirection = API_Vector();
        std::pair<API_Coord, API_Coord> lineBase = GetFirtLineWall(groupItem, moveDirection);
        double totalthickness = GetTotalThickness(groupItem);

        API_Coord begMir = API_Coord();
        begMir.x = lineBase.first.x + moveDirection.x * totalthickness / 2;
        begMir.y = lineBase.first.y + moveDirection.y * totalthickness / 2;
        API_Coord endMir = API_Coord();
        endMir.x = lineBase.second.x + moveDirection.x * totalthickness / 2;
        endMir.y = lineBase.second.y + moveDirection.y * totalthickness / 2;

        API_EditPars editPars;
        BNZeroMemory(&editPars, sizeof(API_EditPars));
        editPars.typeID = APIEdit_Mirror;
        editPars.begC = API_Coord3D{ begMir.x, begMir.y, 0 };
        editPars.endC = API_Coord3D{ endMir.x, endMir.y, 0 };
        editPars.withDelete = true;
        GS::Array<API_Neig> neigs= GS::Array<API_Neig>();
        API_Neig theNeigh = {};
        for (API_Element el : groupItem) {
            BNZeroMemory(&theNeigh, sizeof(API_Neig));
            API_Elem_Head elementHead = el.header;
            theNeigh.guid = elementHead.guid;
            theNeigh.neigID = APINeig_Wall;
            theNeigh.inIndex = 1;
            neigs.Push(theNeigh);
        }
        ACAPI_Element_Edit(&neigs, editPars);
    }


    return NoError;
}
