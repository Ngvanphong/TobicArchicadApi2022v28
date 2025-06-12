#include "AlignBeamToFloor.h"
#include <RealNumber.h>
/// <summary>
/// Nho cho vat lieu cua dam len cao de dam cat san
/// </summary>
/// <param name="selNeigs"></param>
/// <returns></returns>
std::vector<std::vector<API_Element>> GetAllBeamAndFloor(GS::Array<API_Neig> selNeigs) 
{
	std::vector<std::vector<API_Element>> result = std::vector<std::vector<API_Element>>();
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID == API_SelEmpty) {
		delete selectionInfo;
		return result;
	}
	std::vector<API_Element> allSlab;
	std::vector<API_Element> allBeam;
	for (int i = 0; i < selectionInfo->sel_nElem; i++) {
		API_Element element;
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			if (element.header.type.typeID == API_BeamID) {
				allBeam.push_back(element);
			}
			else if (element.header.type.typeID == API_SlabID) {
				allSlab.push_back(element);
			}
		}
	}
	result.push_back(allSlab);
	result.push_back(allBeam);
	return result;

}

std::wstring ToWStringBeam(const GS::UniString& str)
{
	static_assert (sizeof(GS::UniChar::Layout) == sizeof(wchar_t), "string character size mismatch");
	return std::wstring((const wchar_t*)str.ToUStr().Get(), str.GetLength());
}

GS::ErrCode AlignBeamFloor(void)
{
	GS::ErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<std::vector<API_Element>> allSelected = GetAllBeamAndFloor(selNeigs);
	std::vector<API_Element> beams = allSelected[1];
	std::vector<API_Element> slabs = allSelected[0];
	API_ElementMemo			 memo;

	//API_ElementQuantity	quantity;
	//API_QuantityPar		params;
	//BNZeroMemory(&params, sizeof(API_QuantityPar));
	//params.minOpeningSize = EPS;
	//API_Quantities	quantities;
	//quantities.elements = &quantity;
	//API_QuantitiesMask mask;
	//ACAPI_ELEMENT_QUANTITIES_MASK_SETFULL(mask);

	API_Element markBeam;
	ACAPI_ELEMENT_MASK_SET(markBeam, API_BeamType, level);

	GS::Array<API_PropertyDefinition> propertyDefinitions;
	
	std::wstring volumnName = L"Volume (Net)";
	for (API_Element beam : beams) 
	{
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		ACAPI_Element_GetMemo(beam.header.guid, &memo);
		bool isSteel = false;
		auto id = ToWStringBeam(*memo.elemInfoString);
		if (id.length()> 0 && (id.at(0) == 'S' || id.at(0) == 's')) isSteel = true;
		double offsetForBeam = 0;
		for (auto slab : slabs) 
		{
			API_Property property;
			//ACAPI_Element_GetQuantities(slab.header.guid, &params, &quantities, &mask);

			ACAPI_Element_GetPropertyDefinitions(slab.header.guid, API_PropertyDefinitionFilter_All, propertyDefinitions);
			API_PropertyDefinition propertyDefinitionSlab;
			for (const API_PropertyDefinition& propertyDefinition : propertyDefinitions)
			{
				auto name = ToWStringBeam(propertyDefinition.name);
				if (name == volumnName)
				{
					ACAPI_Element_GetPropertyValue(slab.header.guid, propertyDefinition.guid, property);
					propertyDefinitionSlab = propertyDefinition;
					break;
				}
			}
			double originVolumn = property.value.singleVariant.variant.doubleValue;


			auto thick = slab.slab.thickness;
			auto offsetLevel = slab.slab.level;

			ACAPI_Element_SolidLink_Create(slab.header.guid, beam.header.guid, API_SolidOperationID::APISolid_Substract, APISolidFlag_OperatorAttrib);
			//ACAPI_Element_GetQuantities(slab.header.guid, &params, &quantities, &mask);

			ACAPI_Element_GetPropertyValue(slab.header.guid, propertyDefinitionSlab.guid, property);
			double volume2 = property.value.singleVariant.variant.doubleValue;

			bool isBreak = false;
			if (abs(originVolumn - volume2)>0.0001)
			{
				if (isSteel)
				{
					offsetForBeam = offsetLevel - thick;
				}
				else 
				{
					offsetForBeam = offsetLevel;
				}
				isBreak = true;
			}
			else 
			{
				for (double offset = offsetLevel-1.1; offset < offsetLevel + 0.1 ; offset += 1.1)
				{
					beam.beam.level = offset;
					ACAPI_Element_Change(&beam, &markBeam, nullptr, 0UL, true);
					ACAPI_Element_GetPropertyValue(slab.header.guid, propertyDefinitionSlab.guid, property);
					double volume3 = property.value.singleVariant.variant.doubleValue;

					if (abs(originVolumn - volume3) > 0.0001)
					{
						if (isSteel)
						{
							offsetForBeam = offsetLevel - thick;
						}
						else
						{
							offsetForBeam = offsetLevel;
						}
						isBreak = true;
						break;
					}
				}
			}
			
			ACAPI_Element_SolidLink_Remove(slab.header.guid, beam.header.guid);
			if (isBreak) break;
			
		}
		beam.beam.level = offsetForBeam;
		ACAPI_Element_Change(&beam, &markBeam, nullptr, 0UL, true);

	}


	return NoError;
}
