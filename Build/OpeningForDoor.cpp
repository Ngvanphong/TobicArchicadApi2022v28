#include "OpeningForDoor.h"
std::wstring ToWStringOpening(const GS::UniString& str)
{
    static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
    return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode CreateOpeningForDoor(void)
{
    std::wstring widthName = L"Nominal W/D Opening Width on the Reveal Side";
    std::wstring heightName = L"Nominal W/D Opening Height on the Side Opposite to the Reveal Side";
    API_SelectionInfo* selectionInfo = new API_SelectionInfo();
    GS::ErrCode err = NoError;
    GS::Array<API_Neig> selNeigs;
    err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
    if (selectionInfo->typeID == API_SelEmpty) {
        delete selectionInfo;
        return NoError;
    }
    API_Element door = {};
    API_Element wall = {};
    bool isHasDoor = false;
    bool isHasWall = false;
    for (int i = 0; i < selectionInfo->sel_nElem; i++) {
        API_Element element;
        element.header.guid = selNeigs[i].guid;
        err = ACAPI_Element_Get(&element);
        if (err == NoError && element.header.type.typeID == API_DoorID) {
            door = element;
            isHasDoor = true;
        }
        else if (err == NoError && element.header.type.typeID == API_WallID) {
            wall = element;
            isHasWall = true;
        }
    }
    if (!isHasDoor || !isHasWall) return NoError;
    
    if (err != NoError) return NoError;
    API_Coord locationDoor = door.door.startPoint;

    double widthDoor = 0;
    double heightDoor = 0;
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    ACAPI_Element_GetPropertyDefinitions(door.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
    for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
    {
        if (propertyDefinition.measureType != API_PropertyLengthMeasureType) continue;
        auto name = ToWStringOpening(propertyDefinition.name);
        if (name == widthName) {
            API_Property property;
            ACAPI_Element_GetPropertyValue(door.header.guid, propertyDefinition.guid, property);
            widthDoor = property.value.singleVariant.variant.doubleValue;
        }
        if (name == heightName) {
            API_Property property;
            ACAPI_Element_GetPropertyValue(door.header.guid, propertyDefinition.guid, property);
            heightDoor = property.value.singleVariant.variant.doubleValue;
        }
    }
    if (widthDoor == 0 || heightDoor == 0) return NoError;
    
    API_Element opening = {};
    BNClear(opening);
    opening.header.type = API_OpeningID;
    ACAPI_Element_GetDefaults(&opening,nullptr);
    opening.opening.owner = wall.header.guid;

    opening.opening.extrusionGeometryData.frame.basePoint.x = locationDoor.x;
    opening.opening.extrusionGeometryData.frame.basePoint.y = locationDoor.y;
    opening.opening.extrusionGeometryData.frame.basePoint.z = 0;

    opening.opening.extrusionGeometryData.frame.axisX.x = door.door.dirVector.x;
    opening.opening.extrusionGeometryData.frame.axisX.y = door.door.dirVector.y;
    opening.opening.extrusionGeometryData.frame.axisX.z = 0;

    
    opening.opening.extrusionGeometryData.frame.axisY.x = 0;
    opening.opening.extrusionGeometryData.frame.axisY.y = 0;
    opening.opening.extrusionGeometryData.frame.axisY.z = 1;


    API_Coord perdendicular = API_Coord{ -door.door.dirVector.y, door.door.dirVector.x };
    opening.opening.extrusionGeometryData.frame.axisZ.x = perdendicular.x;
    opening.opening.extrusionGeometryData.frame.axisZ.y = perdendicular.y;
    opening.opening.extrusionGeometryData.frame.axisZ.z = 0;

    opening.opening.extrusionGeometryData.parameters.width = widthDoor;
    opening.opening.extrusionGeometryData.parameters.height = heightDoor;
    opening.opening.extrusionGeometryData.parameters.anchor = APIAnc_MB;
    ACAPI_Element_Create(&opening, nullptr);
    



    return NoError;
}
