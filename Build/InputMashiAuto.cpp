#include "InputMashiAuto.h"
#include "ReadExcel.h"
#include "GSUtilsDefs.h"
#include <string>

static void DisposeAttribute(API_Attribute* attrib)
{
	switch (attrib->header.typeID) {
	case API_MaterialID:			delete attrib->material.texture.fileLoc;											break;
	default:						/* nothing has to be disposed */													break;
	}
}

std::wstring ToWString(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}




GS::ErrCode InputMashiAutoExcel()
{
	GS::ErrCode err = GS::NoError;
    auto data = GetDataFromExcelForMashi();
	GS::Array<API_Guid> beamList;
	ACAPI_Element_GetElemList(API_BeamID, &beamList);
	GS::Array<API_Guid> columnList;
	ACAPI_Element_GetElemList(API_ColumnID, &columnList);

	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	GS::Array<API_Guid> allGuid;
	allGuid = beamList;
	for (auto id : columnList) allGuid.Push(id);


	API_Attribute attrib;
	UInt32	count, i;
	err = ACAPI_Attribute_GetNum(API_ProfileID, count);
	if (err != NoError) return err;
	std::vector<API_Attribute> allProfile;
	for (i = 1; i <= count; i++) {
		BNZeroMemory(&attrib, sizeof(API_Attribute));
		attrib.header.typeID = API_ProfileID;
		attrib.header.index = ACAPI_CreateAttributeIndex(i);
		err = ACAPI_Attribute_Get(&attrib);
		allProfile.push_back(attrib);
		DisposeAttribute(&attrib);
	}

	for (API_Guid guidId : allGuid) {
		element.header.guid = guidId;
		err = ACAPI_Element_Get(&element);
		if (err != NoError) continue;
		err = ACAPI_Element_GetMemo(guidId, &memo);
		if (err != NoError) {
			ACAPI_DisposeElemMemoHdls(&memo);
			continue;
		}

		API_Attribute* attributeModify = nullptr;
		if (element.header.type.typeID == API_BeamID)
		{
			for (USize idx = 0; idx < element.beam.nSegments; ++idx)
			{
				for (auto att : allProfile) {
					if (att.header.index == memo.beamSegments[idx].assemblySegmentData.profileAttr) {
						attributeModify = &att;
						break;
					}
				}
			}
		}
		else if (element.header.type.typeID == API_ColumnID) {
			for (USize idx = 0; idx < element.column.nSegments; ++idx)
			{
				for (auto att : allProfile) {
					if (att.header.index == memo.columnSegments[idx].assemblySegmentData.profileAttr) {
						attributeModify = &att;
						break;
					}
				}
			}
		}

		if (attributeModify == nullptr) {
			ACAPI_DisposeElemMemoHdls(&memo);
			continue;
		}
		double top = 0;
		double bottom = 0;
		double left = 0;
		double right = 0;
		bool isExistInExcel = false;
		for (auto dataExcel : data)
		{
			if (dataExcel.Name == attrib.header.name)
			{
				top = dataExcel.Top;
				bottom = dataExcel.Bottom;
				left = dataExcel.Left;
				right = dataExcel.Right;
				isExistInExcel = true;
				break;
			}
		}

		if (isExistInExcel) 
		{
			GS::Array<API_PropertyDefinition> propertyDefinitions;
			err = ACAPI_Element_GetPropertyDefinitions(guidId, API_PropertyDefinitionFilter_All, propertyDefinitions);
			if (err != NoError) continue;
			//std::vector<std::wstring> allName;
			for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
			{
				if (propertyDefinition.definitionType != API_PropertyDynamicBuiltInDefinitionType
					||propertyDefinition.measureType!= API_PropertyLengthMeasureType) continue;

				/*auto nameI = ToWString(propertyDefinition.name);
				allName.push_back(nameI);*/
				
				if (element.header.type.typeID == API_BeamID) 
				{
					if (ToWString(propertyDefinition.name) == L"ó¿ëùÇµë≈Çø-è„")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = top;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"ó¿ëùÇµë≈Çø-â∫")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = bottom;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"ó¿ëùÇµë≈Çø-ç∂")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = left;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"ó¿ëùÇµë≈Çø-âE")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = right;
						ACAPI_Element_SetProperty(guidId, property);
					}
				}
				else if (element.header.type.typeID == API_ColumnID) 
				{
					if (ToWString(propertyDefinition.name) == L"íåëùÇµë≈Çø-è„")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = top;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"íåëùÇµë≈Çø-â∫")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = bottom;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"íåëùÇµë≈Çø-ç∂")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = left;
						ACAPI_Element_SetProperty(guidId, property);
					}
					else if (ToWString(propertyDefinition.name) == L"íåëùÇµë≈Çø-âE")
					{
						API_Property property;
						err = ACAPI_Element_GetPropertyValue(guidId, propertyDefinition.guid, property);
						if (err != NoError) continue;
						property.value.singleVariant.variant.doubleValue = right;
						ACAPI_Element_SetProperty(guidId, property);
					}
				}
			}
		}

		ACAPI_DisposeElemMemoHdls(&memo);
		
	}


    return NoError;
}
