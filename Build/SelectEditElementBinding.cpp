#include "SelectEditElementBinding.h"

GS::ErrCode SelectionEditElems(void)
{
	GS::Array<API_Neig> selNeigs;
	API_SelectionInfo* selectionInfo = new API_SelectionInfo();
	GS::ErrCode err = ACAPI_Selection_Get(selectionInfo, &selNeigs, false);
	if (selectionInfo->typeID == API_SelEmpty) {
		return NoError;
	}
	GS::Array<API_Neig> idRemoves = GS::Array<API_Neig>();
	for (int i = 0; i < selectionInfo->sel_nElem; i++) {
		bool isEdit=ACAPI_Element_Filter(selNeigs[i].guid, APIFilt_IsEditable);
		if (!isEdit)idRemoves.Push(selNeigs[i]);
	}
	ACAPI_Selection_Select(idRemoves, false);
	delete selectionInfo;
	return NoError;
}
