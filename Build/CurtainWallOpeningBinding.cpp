#include "CurtainWallOpeningBinding.h"

double DistanceCoordCurtain(const API_Coord& v1, const API_Coord& v2)
{
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}
API_Vector NormalizeVector2DCurtain(const API_Vector& v)

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



bool CheckParellVectorCurtain(const API_Vector& v1, const API_Vector& v2)
{
    if ((abs(v1.x) < 0.0001 && abs(v1.y) < 0.0001) || (abs(v2.x) < 0.0001 && abs(v2.y) < 0.0001)) return false;
    auto result = (v1.x * v2.x + v1.y * v2.y);
    return  abs(abs(result) - 1) < 0.000000000000001;
}

GS::ErrCode  CreateOpeningForCurtainWall(void)
{
    std::wstring homeOffsetName = L"Home Offset";
    API_SelectionInfo* selectionInfo = new API_SelectionInfo();
    GS::ErrCode err = NoError;
    GS::Array<API_Neig> selNeigs;
    err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
    if (selectionInfo->typeID == API_SelEmpty) {
        delete selectionInfo;
        return NoError;
    }
    API_Element curtain = {};
    API_Element wall = {};
    bool isHasCurtain = false;
    bool isHasWall = false;
    for (int i = 0; i < selectionInfo->sel_nElem; i++) {
        API_Element element;
        element.header.guid = selNeigs[i].guid;
        err = ACAPI_Element_Get(&element);
        if (err == NoError && element.header.type.typeID == API_CurtainWallID) {
            curtain = element;
            isHasCurtain = true;
        }
        else if (err == NoError && element.header.type.typeID == API_WallID) {
            wall = element;
            isHasWall = true;
        }
    }
    if (!isHasCurtain || !isHasWall) return NoError;

    
    double heightCurtain = curtain.curtainWall.height;
    API_ElementMemo curtainMemo = {};
    ACAPI_Element_GetMemo(curtain.header.guid, &curtainMemo);
    auto coords = curtainMemo.coords;
    auto pEnds = curtainMemo.pends;
    auto countSegment = BMhGetSize(reinterpret_cast<GSHandle> (curtainMemo.coords)) / Sizeof32(API_Coord) - 2;
    auto poly = curtain.curtainWall.polygon.nCoords;
   
    auto pointerCoord = &(*curtainMemo.coords)[1];

    GS::Int32 previousEnd = 0;
    std::vector<API_Coord> arrayStart = std::vector<API_Coord>();
    std::vector<API_Coord> arrayEnd = std::vector<API_Coord>();
    for (int seg = 1; seg <= countSegment; seg++) 
    {
        arrayStart.push_back(*pointerCoord);
        pointerCoord++;
        arrayEnd.push_back(*pointerCoord);
    }

    API_Coord begWall = wall.wall.begC;
    API_Coord endWall = wall.wall.endC;
    API_Coord wallDir{ endWall.x - begWall.x,endWall.y - begWall.y };
    API_Coord wallDirNor = NormalizeVector2DCurtain(wallDir);

    for (int i = 0; i < arrayStart.size(); i++) 
    {
        API_Element opening = {};
        BNClear(opening);
        opening.header.type = API_OpeningID;
        ACAPI_Element_GetDefaults(&opening, nullptr);
        opening.opening.owner = wall.header.guid;

        API_Coord direction{ arrayEnd[i].x - arrayStart[i].x, arrayEnd[i].y - arrayStart[i].y };
        API_Coord directionNor = NormalizeVector2DCurtain(direction);
        bool isParell = CheckParellVectorCurtain(wallDirNor, directionNor);
        if (!isParell) continue;

        API_Coord midPoint{ (arrayStart[i].x + arrayEnd[i].x) / 2,  (arrayStart[i].y + arrayEnd[i].y) / 2 };

        opening.opening.extrusionGeometryData.frame.basePoint.x = midPoint.x;
        opening.opening.extrusionGeometryData.frame.basePoint.y = midPoint.y;
        opening.opening.extrusionGeometryData.frame.basePoint.z = 0;

        opening.opening.extrusionGeometryData.frame.axisX.x = -1.0;
        opening.opening.extrusionGeometryData.frame.axisX.y = 0.0;
        opening.opening.extrusionGeometryData.frame.axisX.z = 0.0;

        opening.opening.extrusionGeometryData.frame.axisY.x = 0.0;
        opening.opening.extrusionGeometryData.frame.axisY.y = 0.0;
        opening.opening.extrusionGeometryData.frame.axisY.z = 1.0;

        // extrusionDir
        opening.opening.extrusionGeometryData.frame.axisZ.x = 0.0;
        opening.opening.extrusionGeometryData.frame.axisZ.y = 1.0;
        opening.opening.extrusionGeometryData.frame.axisZ.z = 0.0;

        double width = DistanceCoordCurtain(arrayStart[i], arrayEnd[i]);

        opening.opening.extrusionGeometryData.parameters.width = width;
        opening.opening.extrusionGeometryData.parameters.height = heightCurtain;
        opening.opening.extrusionGeometryData.parameters.anchor = APIAnc_MB;
        opening.opening.extrusionGeometryData.parameters.anchorAltitude = curtain.curtainWall.storyRelLevel;
        err = ACAPI_Element_Create(&opening, nullptr);

    }

    return NoError;



}
