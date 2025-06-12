#include "ParametersSet.h"

std::wstring ToWStringPara(const GS::UniString& str)
{
    static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
    return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

// set is default = false de co the edit duoc
GS::ErrCode SetParameterToWallSlab()
{
    GS::ErrCode err = NoError;
    std::wstring nameLengthAr = L"Length of the Wall on the Outside Face";
    std::wstring nameAreaAr = L"Surface Area of the Wall Outside Face (Net)";
    std::wstring nameAreaArSlab = L"Area";

    std::wstring nameLength = L"Length Addin";
    std::wstring nameArea = L"Area Addin";
    std::wstring nameThickness = L"Thickness Addin";
    std::wstring nameHeight = L"Height Addin";


    GS::Array<API_Guid> wallIds = GS::Array<API_Guid>();
    ACAPI_Element_GetElemList(API_ElemTypeID::API_WallID, &wallIds);
    GS::Array<API_Guid> slabIds= GS::Array<API_Guid>();
    ACAPI_Element_GetElemList(API_ElemTypeID::API_SlabID, &slabIds);
    for (API_Guid id : slabIds) wallIds.Push(id);

    API_Element element;
    BNZeroMemory(&element, sizeof(API_Element));
    for (API_Guid id : wallIds) 
    {
        element.header.guid = id;
        err = ACAPI_Element_Get(&element);
        if (err != NoError) continue;
        GS::Array<API_PropertyDefinition> propertyDefinitions;
        ACAPI_Element_GetPropertyDefinitions(element.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
        if (element.header.type.typeID == API_WallID) 
        {
            auto thick = element.wall.thickness * 1000;
            auto height = element.wall.height * 1000;
            API_Property areaProperty;
            API_Property lengthProperty;
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions) 
            {
                
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameLengthAr) 
                {
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, lengthProperty);
                    break;
                }
            }
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameAreaAr)
                {
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, areaProperty);
                    break;
                }
            }

            // set
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameArea)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = areaProperty.value.singleVariant.variant.doubleValue;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameLength)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = lengthProperty.value.singleVariant.variant.doubleValue *1000;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameThickness)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = thick;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameHeight)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = height;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }
            

        }
        else if(element.header.type.typeID= API_SlabID)
        {
            auto thick = element.slab.thickness * 1000;
            API_Property areaProperty;
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameAreaArSlab)
                {
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, areaProperty);
                    break;
                }
            }

            // set
            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {

                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameArea)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = areaProperty.value.singleVariant.variant.doubleValue;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }

            for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
            {
                auto name = ToWStringPara(propertyDefinition.name);
                if (name == nameThickness)
                {
                    API_Property property;
                    ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
                    property.isDefault = false;
                    property.value.singleVariant.variant.doubleValue = thick;
                    ACAPI_Element_SetProperty(element.header.guid, property);
                    break;
                }
            }
        }

    }



    return NoError;
}
