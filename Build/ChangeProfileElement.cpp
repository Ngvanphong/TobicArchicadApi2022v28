#include "ChangeProfileElement.h"

std::vector<API_Element> SelectionColumnBeams(GS::Array<API_Neig> selNeigs) {
	std::vector<API_Element> vectorElements;
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) return vectorElements;
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++) {
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError&&(element.header.type.typeID==API_BeamID||
			element.header.type.typeID==API_ColumnID))
		{
			vectorElements.push_back(element);
		} 
	}
	return vectorElements;
}

static void DisposeAttribute(API_Attribute* attrib)
{
	switch (attrib->header.typeID) {
	case API_MaterialID:			delete attrib->material.texture.fileLoc;											break;
	default:						/* nothing has to be disposed */													break;
	}
}

GS::ErrCode ChangeProfileElement()
{
	GS::ErrCode err = GS::NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> vectorElements = SelectionColumnBeams(selNeigs);
	API_Attribute attrib;
	UInt32	count=0, i;
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

	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	API_Element mask;
	for (API_Element element : vectorElements) 
	{
		err = ACAPI_Element_GetMemo(element.header.guid, &memo);
		if (err != NoError) {
			ACAPI_DisposeElemMemoHdls(&memo);
			continue;
		}
		for (auto att : allProfile)
		{
			//if (element.header.type.typeID==API_WallID) 
			//{
			//	if (att.header.name == *memo.elemInfoString&&att.profile.wallType) 
			//	{
			//		element.wall.composite = API_ProfileStructure;
			//		ACAPI_ELEMENT_MASK_CLEAR(mask);
			//		ACAPI_ELEMENT_MASK_SET(mask, API_WallType, composite);
			//		err=ACAPI_Element_Change(&element, &mask, nullptr, 0UL, true);

			//		/*element.wall.profileAttr = att.profile.head.index;
			//		ACAPI_ELEMENT_MASK_CLEAR(mask);
			//		ACAPI_ELEMENT_MASK_SET(mask, API_WallType, profileAttr);
			//		ACAPI_Element_Change(&element, &mask, nullptr, 0UL, true);*/
			//		ACAPI_DisposeElemMemoHdls(&memo);
			//		break;
			//	}
			//}
			if (element.header.type.typeID == API_BeamID &&att.profile.beamType)
			{
				if (att.header.name == *memo.elemInfoString)
				{
					for (USize idx = 0;  idx < element.beam.nSegments; ++idx) 
					{
						memo.beamSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
						memo.beamSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
					}
					API_Element maskElem;
					ACAPI_ELEMENT_MASK_CLEAR(maskElem);
					err = ACAPI_Element_Change(&element, &maskElem, &memo, APIMemoMask_BeamSegment, true);
					ACAPI_DisposeElemMemoHdls(&memo);
					break;
				}
			}
			else if (element.header.type.typeID == API_ColumnID && att.profile.coluType) 
			{
				if (att.header.name == *memo.elemInfoString)
				{
					for (USize idx = 0; idx < element.column.nSegments; ++idx)
					{
						memo.columnSegments[idx].assemblySegmentData.modelElemStructureType = API_ProfileStructure;
						memo.columnSegments[idx].assemblySegmentData.profileAttr = att.profile.head.index;
					}
					API_Element maskElem;
					ACAPI_ELEMENT_MASK_CLEAR(maskElem);
					err = ACAPI_Element_Change(&element, &maskElem, &memo, APIMemoMask_ColumnSegment, true);
					ACAPI_DisposeElemMemoHdls(&memo);
					break;
				}
			}
		}
	}

	return NoError;
}
