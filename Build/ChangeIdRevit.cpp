
#include "ChangeIdRevit.h"

std::vector<API_Element> GetAllElements(GS::Array<API_Neig>);


GSErrCode ChangeIdFromElement()
{
	GSErrCode err = NoError;
	GS::Array<API_Neig> selNeigs;
	std::vector<API_Element> arrayElement = GetAllElements(selNeigs);
	API_Element		   mask;
	API_ElementMemo		memo;
	for (API_Element& el : arrayElement)
	{
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));

		if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) {
			auto oldValue = (*memo.elemInfoString);
			auto newChars = oldValue.Split(":");
			GS::UniString newValue;
			if (newChars.GetSize() >= 2)newValue = newChars[1];
			else newValue = newChars[0];
			if (memo.elemInfoString != nullptr) {
				delete memo.elemInfoString;
			}
			memo.elemInfoString = new GS::UniString(newValue);
		}

		if (err == NoError) {
			err = ACAPI_Element_ChangeParameters({ el.header.guid }, &el, &memo, &mask);
		}
		ACAPI_DisposeElemMemoHdls(&memo);

	};


	return NoError;

}

std::vector<API_Element> GetAllElements(GS::Array<API_Neig> selNeigs) {
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
		arrayElements.insert(arrayElements.end(), element);
	};
	return arrayElements;
}











