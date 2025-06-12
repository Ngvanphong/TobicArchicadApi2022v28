#include "OpeningWallBinding.h"
#include	"basicgeometry.h"

static void		API2AC_Tranmat(TRANMAT* tranmat, const API_Tranmat& api_tranmat)
{
    tranmat->status = 0;
    tranmat->dummy1 = tranmat->dummy2 = tranmat->dummy3 = 0;
    Geometry::Vector3d vec1(api_tranmat.tmx[0], api_tranmat.tmx[4], api_tranmat.tmx[8]), vec2(api_tranmat.tmx[1], api_tranmat.tmx[5], api_tranmat.tmx[9]), vec3(api_tranmat.tmx[2], api_tranmat.tmx[6], api_tranmat.tmx[10]), vec4(api_tranmat.tmx[3], api_tranmat.tmx[7], api_tranmat.tmx[11]);
    tranmat->SetMatrix(Geometry::Matrix34::CreateFromColVectors(vec1, vec2, vec3, vec4));
}		/* API2AC_Tranmat */
std::wstring ToWStringOpeningDoor(const GS::UniString& str)
{
    static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
    return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode CreateOpening(void)
{
    //std::wstring widthName = L"Nominal W/D Opening Width on the Reveal Side";
    //std::wstring heightName = L"Nominal W/D Opening Height on the Side Opposite to the Reveal Side";
    std::wstring homeOffsetName = L"Home Offset";
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

    double widthDoor = door.door.openingBase.width+0.08;
    double heightDoor = door.door.openingBase.height+0.04;
    double bootomOffsetDoor = 0;

    GS::Array<API_PropertyDefinition> propertyDefinitions;
    ACAPI_Element_GetPropertyDefinitions(door.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
    for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
    {
        if (propertyDefinition.measureType != API_PropertyLengthMeasureType) continue;
        auto name = ToWStringOpeningDoor(propertyDefinition.name);
        if (name == homeOffsetName) {
            API_Property property;
            ACAPI_Element_GetPropertyValue(door.header.guid, propertyDefinition.guid, property);
            bootomOffsetDoor = property.value.singleVariant.variant.doubleValue;
        }
    }

    API_Tranmat tm;
    ACAPI_Element_GetOpeningTransformation(&door.header.guid, &tm);
    TRANMAT		openingPlaceTransformation;
    Coord3D locationOpening(0.0, 0.0, 0.0);
    API2AC_Tranmat(&openingPlaceTransformation, tm);
    locationOpening = Geometry::TransformPoint(openingPlaceTransformation, locationOpening);

   
    if (widthDoor == 0 || heightDoor == 0) return NoError;

    API_Element opening = {};
    BNClear(opening);
    opening.header.type = API_OpeningID;
    ACAPI_Element_GetDefaults(&opening, nullptr);
    opening.opening.owner = wall.header.guid;


    opening.opening.extrusionGeometryData.frame.basePoint.x = locationOpening.x;
    opening.opening.extrusionGeometryData.frame.basePoint.y = locationOpening.y;
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

    opening.opening.extrusionGeometryData.parameters.width = widthDoor;
    opening.opening.extrusionGeometryData.parameters.height = heightDoor;
    opening.opening.extrusionGeometryData.parameters.anchor = APIAnc_MB;
    opening.opening.extrusionGeometryData.parameters.anchorAltitude = bootomOffsetDoor;
    err= ACAPI_Element_Create(&opening, nullptr);
    if (err != NoError) return NoError;

    return NoError;
}




GS::ErrCode CreateOpeningWindow(void)
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
    API_Element window = {};
    API_Element wall = {};
    bool isHasWindow = false;
    bool isHasWall = false;
    for (int i = 0; i < selectionInfo->sel_nElem; i++) {
        API_Element element;
        element.header.guid = selNeigs[i].guid;
        err = ACAPI_Element_Get(&element);
        if (err == NoError && element.header.type.typeID == API_WindowID) {
            window = element;
            isHasWindow = true;
        }
        else if (err == NoError && element.header.type.typeID == API_WallID) {
            wall = element;
            isHasWall = true;
        }
    }
    if (!isHasWindow || !isHasWall) return NoError;

    double widthWindow = window.window.openingBase.width;
    double heightWindow = window.window.openingBase.height;
    double bootomOffsetWindow = 0;

    GS::Array<API_PropertyDefinition> propertyDefinitions;
    ACAPI_Element_GetPropertyDefinitions(window.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
    for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
    {
        if (propertyDefinition.measureType != API_PropertyLengthMeasureType) continue;
        auto name = ToWStringOpeningDoor(propertyDefinition.name);

        if (name == homeOffsetName) {
            API_Property property;
            ACAPI_Element_GetPropertyValue(window.header.guid, propertyDefinition.guid, property);
            bootomOffsetWindow = property.value.singleVariant.variant.doubleValue;
        }
    }

    API_Tranmat tm;
    ACAPI_Element_GetOpeningTransformation(&window.header.guid, &tm);
    TRANMAT		openingPlaceTransformation;
    Coord3D locationOpening(0.0, 0.0, 0.0);
    API2AC_Tranmat(&openingPlaceTransformation, tm);
    locationOpening = Geometry::TransformPoint(openingPlaceTransformation, locationOpening);


    if (widthWindow == 0 || heightWindow == 0) return NoError;

    API_Element opening = {};
    BNClear(opening);
    opening.header.type = API_OpeningID;
    ACAPI_Element_GetDefaults(&opening, nullptr);
    opening.opening.owner = wall.header.guid;


    opening.opening.extrusionGeometryData.frame.basePoint.x = locationOpening.x;
    opening.opening.extrusionGeometryData.frame.basePoint.y = locationOpening.y;
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

    opening.opening.extrusionGeometryData.parameters.width = widthWindow;
    opening.opening.extrusionGeometryData.parameters.height = heightWindow;
    opening.opening.extrusionGeometryData.parameters.anchor = APIAnc_MB;
    opening.opening.extrusionGeometryData.parameters.anchorAltitude = bootomOffsetWindow;
    err = ACAPI_Element_Create(&opening, nullptr);
    if (err != NoError) return NoError;

    return NoError;
}
