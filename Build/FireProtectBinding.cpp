#include "FireProtectBinding.h"
#include "FormCheckBoxFire.h"
#include <codecvt>
#include <string>
#include <locale>

std::wstring ToWStringFire(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}
std::wstring stringToWstringFire(const char* utf8Bytes)
{
	//setup converter
	using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
	std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(utf8Bytes);
}
std::vector<API_Element> SelectionHostFire(GS::Array<API_Neig> selNeigs) 
{
	std::vector<API_Element> result = std::vector<API_Element>();
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID = API_SelEmpty) {
		delete selectionInfo;
		return result;
	}
	for (int i = 0; i < selectionInfo->sel_nElem; i++) {
		API_Element element;
		element.header.guid = selNeigs[i].guid;
		ACAPI_Element_Get(&element);
		if (element.header.type.typeID == API_BeamID || element.header.type.typeID == API_ColumnID) {
			result.push_back(element);
		}
	}
	delete selectionInfo;
	return result;
}


GS::ErrCode CreateFireProtect(void)
{
	std::wstring nameProfile;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> selectedElements = SelectionHostFire(selNeigs);
	if (selectedElements.size() == 0) return NoError;
	ShowCheckBoxFireDiaglog();
	bool isH = CheckBoxFireDialog::IsCheckH;
	
	std::vector<API_Attribute> allProfile;
	API_Attribute attr;
	UInt32 count, index;
	ACAPI_Attribute_GetNum(API_ProfileID, count);
	for (index = 1; index <= count; index++) {
		BNZeroMemory(&attr, sizeof(API_Attribute));
		attr.header.typeID = API_ProfileID;
		attr.header.index = ACAPI_CreateAttributeIndex(index);
		auto err = ACAPI_Attribute_Get(&attr);
		if (err == NoError && (attr.profile.beamType || attr.profile.coluType)) 
		{
			allProfile.push_back(attr);
		}
	}

	API_Element fire = {};
	API_ElementMemo memoFire = {};
	GS::Array<API_Neig> fireProtectArray = GS::Array<API_Neig>();
	API_Neig neighElement;
	for (auto element : selectedElements)
	{
		BNZeroMemory(&neighElement, sizeof(API_Neig));
		GS::Array<API_PropertyDefinition> propertyDefinitions;
		API_Property property;
		ACAPI_Element_GetPropertyDefinitions(element.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);


		if (element.header.type.typeID == API_BeamID) 
		{
			// Profile of fire
			if (isH) nameProfile = L"B_H";
			else nameProfile = L"B_R";
			API_Coord start = element.beam.begC;
			API_Coord end = element.beam.endC;

			// Parameter Name Beam Fire Protect;
			std::wstring nameWidthFire = L"Nominal Width";
			std::wstring nameHeightFire = L"Nominal Height";
			std::wstring thickNameFire = L"t";
			std::wstring twNameFire = L"tw";
			std::wstring tfNameFire = L"tf";



			BNClear(fire);
			BNClear(memoFire);
			fire.header.type.typeID = API_BeamID;
			ACAPI_Element_GetDefaults(&fire,&memoFire);
			fire.beam.begC = start;
			fire.beam.endC = end;
			fire.header.floorInd = element.header.floorInd;
			fire.beam.level = element.beam.level;
			auto err= ACAPI_Element_Create(&fire, &memoFire);
			
			neighElement.guid = fire.header.guid;
			neighElement.neigID = APINeig_Beam;
			neighElement.inIndex = 1;
			fireProtectArray.Push(neighElement);


			
			double width = 0;
			double height = 0;
			double thickness = 0.01;

			std::wstring widthNameBeam = L"Width";
			std::wstring heightNameBeam = L"Height";
			if (isH) 
			{
				// Parameter Name of Beam
				std::wstring twNameBeam = L"H形鋼-ウェブ厚さ";
				std::wstring tfNameBeam = L"H形鋼-フランジ厚さ";


				double tw = 0;
				double tf = 0;
				API_PropertyDefinition propertyDefinition;
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == widthNameBeam)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						width = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == heightNameBeam)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						height = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == twNameBeam && tw==0) 
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						tw = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == tfNameBeam && tf==0) 
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						tf = property.value.singleVariant.variant.doubleValue;
					}
				}
				if (width == 0 || height == 0 || tw == 0 || tf == 0) continue;
				ACAPI_Element_GetMemo(fire.header.guid, &memoFire);
				for (auto att : allProfile)
				{
					if (stringToWstringFire(att.header.name) == nameProfile)
					{
						for (USize idx = 0; idx < fire.beam.nSegments; ++idx)
						{
							memoFire.beamSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
							memoFire.beamSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
						}
						API_Element maskElem;
						ACAPI_ELEMENT_MASK_CLEAR(maskElem);
						err = ACAPI_Element_Change(&fire, &maskElem, &memoFire, APIMemoMask_BeamSegment, true);
						break;
					}
				}

				GS::Array<API_PropertyDefinition> propertyDefinitionFires;
				API_Property propertyFire;
				ACAPI_Element_GetPropertyDefinitions(fire.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitionFires);
				
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitionFires)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == nameWidthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = width;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == nameHeightFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = height;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == thickNameFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = thickness;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == twNameFire) 
					{

						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = tw;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == tfNameFire) 
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = tf;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					
				}

			}
			else 
			{
				API_PropertyDefinition propertyDefinition;
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == widthNameBeam)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						width = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == heightNameBeam) 
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						height = property.value.singleVariant.variant.doubleValue;
					}
				}
				if (width == 0 || height == 0) continue;

				ACAPI_Element_GetMemo(fire.header.guid, &memoFire);
				for (auto att : allProfile) 
				{
					if (stringToWstringFire(att.header.name) == nameProfile) 
					{
						for (USize idx = 0; idx < fire.beam.nSegments; ++idx)
						{
							memoFire.beamSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
							memoFire.beamSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
						}
						API_Element maskElem;
						ACAPI_ELEMENT_MASK_CLEAR(maskElem);
						err = ACAPI_Element_Change(&fire, &maskElem, &memoFire, APIMemoMask_BeamSegment, true);
						break;
					}
				}


				GS::Array<API_PropertyDefinition> propertyDefinitionFires;
				API_Property propertyFire;
				ACAPI_Element_GetPropertyDefinitions(fire.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitionFires);
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitionFires)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == nameWidthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = width;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == nameHeightFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = height;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == thickNameFire) 
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = thickness;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
				}
			}

		}
		else 
		{
			if (isH) nameProfile = L"C_H";
			else nameProfile = L"C_R";

			API_Coord start = element.column.origoPos;
			double bottomOfset = element.column.bottomOffset;
			double angle = element.column.axisRotationAngle;
			double heightColumn = element.column.height;

			// Parameter Name Column Fire Protect;
			std::wstring nameWidthFire = L"Nominal Width";
			std::wstring nameDepthFire = L"Nominal Height";
			std::wstring thickNameFire = L"t";
			std::wstring twNameFire = L"tw";
			std::wstring tfNameFire = L"tf";



			BNClear(fire);
			BNClear(memoFire);
			fire.header.type.typeID = API_ColumnID;
			ACAPI_Element_GetDefaults(&fire, &memoFire);
			fire.column.origoPos = start;
			fire.header.floorInd = element.header.floorInd;
			fire.column.relativeTopStory = 0;
			fire.column.height = heightColumn;
			fire.column.axisRotationAngle = angle;
			fire.column.bottomOffset = bottomOfset;
			auto err = ACAPI_Element_Create(&fire, &memoFire);

			neighElement.guid = fire.header.guid;
			neighElement.neigID = APINeig_Colu;
			neighElement.inIndex = 1;
			fireProtectArray.Push(neighElement);

			double width = 0;
			double depth = 0;
			double thickness = 0.01;

			std::wstring widthNameColumn = L"Width";
			std::wstring depthNameColumn = L"Core Height / Diameter";
			if (isH)
			{
				// Parameter Name of Column
				std::wstring twNameColumn = L"H形鋼-ウェブ厚さ";
				std::wstring tfNameColumn = L"H形鋼-フランジ厚さ";


				double tw = 0;
				double tf = 0;
				API_PropertyDefinition propertyDefinition;
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == widthNameColumn)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						width = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == depthNameColumn)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						depth = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == twNameColumn && tw == 0)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						tw = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == tfNameColumn && tf == 0)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						tf = property.value.singleVariant.variant.doubleValue;
					}
				}
				if (width == 0 || depth == 0 || tw == 0 || tf == 0) continue;
				ACAPI_Element_GetMemo(fire.header.guid, &memoFire);
				for (auto att : allProfile)
				{
					if (stringToWstringFire(att.header.name) == nameProfile)
					{
						for (USize idx = 0; idx < fire.column.nSegments; ++idx)
						{
							memoFire.columnSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
							memoFire.columnSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
						}
						API_Element maskElem;
						ACAPI_ELEMENT_MASK_CLEAR(maskElem);
						err = ACAPI_Element_Change(&fire, &maskElem, &memoFire, APIMemoMask_ColumnSegment, true);
						break;
					}
				}

				GS::Array<API_PropertyDefinition> propertyDefinitionFires;
				API_Property propertyFire;
				ACAPI_Element_GetPropertyDefinitions(fire.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitionFires);

				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitionFires)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == nameWidthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = width;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == nameDepthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = depth;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == thickNameFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = thickness;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == twNameFire)
					{

						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = tw;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == tfNameFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = tf;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}

				}

			}
			else
			{
				API_PropertyDefinition propertyDefinition;
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == widthNameColumn)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						width = property.value.singleVariant.variant.doubleValue;
					}
					else if (name == depthNameColumn)
					{
						ACAPI_Element_GetPropertyValue(element.header.guid, propertyDefinition.guid, property);
						depth = property.value.singleVariant.variant.doubleValue;
					}
				}
				if (width == 0 || depth == 0) continue;

				ACAPI_Element_GetMemo(fire.header.guid, &memoFire);
				for (auto att : allProfile)
				{
					if (stringToWstringFire(att.header.name) == nameProfile)
					{
						for (USize idx = 0; idx < fire.column.nSegments; ++idx)
						{
							memoFire.columnSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
							memoFire.columnSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
						}
						API_Element maskElem;
						ACAPI_ELEMENT_MASK_CLEAR(maskElem);
						err = ACAPI_Element_Change(&fire, &maskElem, &memoFire, APIMemoMask_ColumnSegment, true);
						break;
					}
				}


				GS::Array<API_PropertyDefinition> propertyDefinitionFires;
				API_Property propertyFire;
				ACAPI_Element_GetPropertyDefinitions(fire.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitionFires);
				for (const API_PropertyDefinition& propertyDefinition : propertyDefinitionFires)
				{
					auto name = ToWStringFire(propertyDefinition.name);
					if (name == nameWidthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = width;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == nameDepthFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = depth;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
					else if (name == thickNameFire)
					{
						ACAPI_Element_GetPropertyValue(fire.header.guid, propertyDefinition.guid, propertyFire);
						propertyFire.value.singleVariant.variant.doubleValue = thickness;
						ACAPI_Element_SetProperty(fire.header.guid, propertyFire);
					}
				}
			}


		}
	}

	if (fireProtectArray.GetSize() > 0) {
		ACAPI_Selection_DeselectAll();
		ACAPI_Selection_Select(fireProtectArray, true);
	}

	ACAPI_DisposeElemMemoHdls(&memoFire);

	return NoError;
}
