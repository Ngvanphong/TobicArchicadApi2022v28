#include "ChangeIdRevitPrefix.h"

std::vector<API_Element> GetAllElementChangeId(GS::Array<API_Neig>);

GSErrCode ChangeIdRevitLevel()
{
	GSErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElement = GetAllElementChangeId(selNeigs);
	API_Element mask;
	API_ElementMemo memo;
	for (API_Element& el : arrayElement) 
	{
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) 
		{

		}
	}
	

	return NoError;
}

std::vector<API_Element> GetAllElementChangeId(GS::Array<API_Neig> selNeigs) 
{
	std::vector<API_Element> arrayElements;
	API_SelectionInfo* selectionInfo= new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs,false);
	if (selectionInfo->typeID == API_SelEmpty) {
		return arrayElements;
	}
	for (int i = 0; i < selectionInfo->sel_nElem; i++) 
	{
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		arrayElements.push_back(element);
	}
	delete selectionInfo;
	return arrayElements;
}
