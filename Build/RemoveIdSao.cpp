#include "RemoveIdSao.h"

std::vector<API_Element> GetAllSelectedElements(GS::Array<API_Neig> selNeigs) {
	std::vector<API_Element> arrayElements;
	API_SelectionInfo selectionInfo;
	GS::ErrCode err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, false);
	if (selectionInfo.typeID == API_SelEmpty) {
		return arrayElements;
	}
	for (Int32 i = 0; i < selectionInfo.sel_nElem; i++)
	{
		API_Element element;
		BNClear(element);
		element.header.guid = selNeigs[i].guid;
		err = ACAPI_Element_Get(&element);
		if (err == NoError) {
			arrayElements.push_back(element);
		}
	};
	return arrayElements;
}

GSErrCode RemoveSao(void)
{
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElement = GetAllSelectedElements(selNeigs);
	API_Element element,mask;
	API_ElementMemo memo;
	for (API_Element& el : arrayElement) 
	{
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) {
			GS::UniString oldValue = *memo.elemInfoString;
			oldValue.ReplaceAll("*", "");
			if (memo.elemInfoString != nullptr) delete memo.elemInfoString;
			memo.elemInfoString = new GS::UniString(oldValue);
			ACAPI_Element_ChangeParameters({ el.header.guid }, &el, &memo, &mask);
		}
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	return NoError;
}

GSErrCode AddSao(void)
{
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElement = GetAllSelectedElements(selNeigs);
	API_Element element, mask;
	API_ElementMemo memo;
	for (API_Element& el : arrayElement)
	{
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) {
			GS::UniString oldValue = *memo.elemInfoString;
			oldValue.Append("*");
			if (memo.elemInfoString != nullptr) delete memo.elemInfoString;
			memo.elemInfoString = new GS::UniString(oldValue);
			ACAPI_Element_ChangeParameters({ el.header.guid }, &el, &memo, &mask);
		}
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	return NoError;
}
