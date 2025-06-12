
#include "LayerListBox.h"
#include	"APIEnvir.h"
#include	"ACAPinc.h"

#include	"Algorithms.hpp"

#include	"MemoryIChannel32.hpp"
#include	"MemoryOChannel32.hpp"
#include	"SetPlatformProtocol.hpp"

LayerForm::LayerForm(GSResModule dialResModule, GS::HashTable<GS::UniString,
	ConvertStatus>& statusTableIn, TabId& lastSortedTabIn, bool& lastDirectionUpIn) :
	DG::ModalDialog(dialResModule, LAYER_DIALOG_RESID, dialResModule),
	layerList(GetReference(), ListBoxId),
	okButton(GetReference(), OkButtonId),
	cancelButton(GetReference(), CancelButtonId),
	searchEdit(GetReference(), searchEditId),
	searchButton(GetReference(), SearchButtonId),
	filterStatusPopup(GetReference(), FilterPopUpId),
	lastSortedTab(lastSortedTabIn),
	lastDirectionUp(lastDirectionUpIn),
	statusTable(statusTableIn)
{
	const short width = layerList.GetItemWidth();  // lay chieu rong cua tat ca item trong list box(hay la tong chieu rong cua list box)
	const short popUpTabWidth = filterStatusPopup.GetWidth(); // get chieu rong cua popup khi click;
	const short tabWidth = (width - popUpTabWidth) / 3; // tinh lai chieu rong thuc su cua list box tru di popup;
	returnHotKey = RegisterHotKey(DG::Key::Return); // dang ky key lui lai;
	enterHotKey = RegisterHotKey(DG::Key::Enter); // dang ly  keyboard enter;

	layerList.SetTabFieldCount(NTabFields);  // set tong cac tab trong list box  hien tai = 4 

	short pos = 0; // vi tri dat cac tab trong list box;

	layerList.SetHeaderSynchronState(false); 

	layerList.SetHeaderItemSize(layerNameTab, tabWidth); // set cheu rong cho tab cua list box ;  
	layerList.SetHeaderItemSize(statusTab, tabWidth);
	layerList.SetHeaderItemSize(filterStatusTab, tabWidth + popUpTabWidth);
	layerList.SetHeaderItemSize(popUpTab, 0);

	layerList.SetTabFieldProperties(layerNameTab, pos, pos + tabWidth, DG::ListBox::Left, DG::ListBox::NoTruncate, false); // set vi tri cho cac tab trong list box
	pos += tabWidth;
	layerList.SetTabFieldProperties(statusTab, pos, pos + tabWidth, DG::ListBox::Left, DG::ListBox::NoTruncate, false);
	pos += tabWidth;
	layerList.SetTabFieldProperties(filterStatusTab, pos, width - popUpTabWidth, DG::ListBox::Left, DG::ListBox::NoTruncate, false);
	pos = width - popUpTabWidth;
	layerList.SetTabFieldProperties(popUpTab, pos, width, DG::ListBox::Left, DG::ListBox::EndTruncate, false);

	// set text;
	GS::UniString headerName;
	// lay string theo id va index da duoc ghi trong file  AddOn.grc
	RSGetIndString(&headerName, LAYER_DIALOG_LIST_BOX_HEADER_STRINGS_RESID, 1, ACAPI_GetOwnResModule());
	layerList.SetHeaderItemText(layerNameTab, headerName); // set text cho header cua list box;

	RSGetIndString(&headerName, LAYER_DIALOG_LIST_BOX_HEADER_STRINGS_RESID, 2, ACAPI_GetOwnResModule());
	layerList.SetHeaderItemText(statusTab, headerName);

	RSGetIndString(&headerName, LAYER_DIALOG_LIST_BOX_HEADER_STRINGS_RESID, 3, ACAPI_GetOwnResModule());
	layerList.SetHeaderItemText(filterStatusTab, headerName);

	layerList.SetHeaderPushableButtons(true);
	
	UInt32 nLayer = 0;
	ACAPI_Attribute_GetNum(API_LayerID, nLayer);
	// add du lieu vao cac row cua list box;
	for (UInt32 i = 2; i <= nLayer; ++i) 
	{
		ListBoxRow row;
		API_Attribute attr;
		BNZeroMemory(&attr, sizeof(attr));
		GS::UniString attrName;
		attr.header.typeID = API_LayerID;
		attr.header.index =ACAPI_CreateAttributeIndex(i);
		attr.header.uniStringNamePtr = &attrName;
		if (ACAPI_Attribute_Get(&attr) == NoError)
		{
			if (!statusTable.ContainsKey(attrName)) {
				statusTable.Add(attrName, Convert); // tao mot mang luu tru lai string cua enum;

				row.layerName = attrName;
				row.flagName = FlagsToText(attr.header.flags);// doi kieu index cua attribute ra text
				row.convStatName = StatusToText(Convert); // doi kieu cua enum 
				sortedList.Push(row);  // them data vao soredList de sap xep;
			}
			else {
				row.layerName = attrName;
				row.flagName = FlagsToText(attr.header.flags);
				row.convStatName = StatusToText(statusTable[attrName]);
				sortedList.Push(row);
			}
		}

		filterStatusPopup.Initialize(StatusToText(Convert), 100, 1); //  them du lieu vao popup cua status
		filterStatusPopup.AppendItem(StatusToText(Convert));
		filterStatusPopup.AppendItem(StatusToText(DontConvert));
		filterStatusPopup.AppendItem(StatusToText(LoadBearing));
		filterStatusPopup.AppendItem(StatusToText(NonLoadBearing));
		filterStatusPopup.Hide(); // ban dau an popup
		if (lastDirectionUp) {
			layerList.SetHeaderItemArrowType((short)lastSortedTab, DG::ListBox::Down); // trang thai dau mui ten sap xep tren header cua list box
		}
		else {
			layerList.SetHeaderItemArrowType((short)lastSortedTab, DG::ListBox::Up); // trang thai dau mui ten sap xep tren header cua list box
		}
		SortListBy(lastSortedTab); // goi ham sap xep du lieu trong list box;
	}

}


LayerForm::~LayerForm()
{
	UnregisterHotKey(returnHotKey);
	UnregisterHotKey(enterHotKey);
}

GS::UniString LayerForm::FlagsToText(short layerFlags) const 
{
	GS::UniString				lfText;
	GS::Array<GS::UniString>	lfTextsList;

	if (layerFlags & APILay_Hidden)
		lfTextsList.Push(RSGetIndString(LAYER_DIALOG_FLAG_STRINGS_RESID, 1, ACAPI_GetOwnResModule())); // lay text theo index da duoc dinh nghia trong file AddOn.grc
	if (layerFlags & APILay_Locked)
		lfTextsList.Push(RSGetIndString(LAYER_DIALOG_FLAG_STRINGS_RESID, 2, ACAPI_GetOwnResModule()));
	if (layerFlags & APILay_NotMine)
		lfTextsList.Push(RSGetIndString(LAYER_DIALOG_FLAG_STRINGS_RESID, 3, ACAPI_GetOwnResModule()));
	if (layerFlags & APILay_Xref)
		lfTextsList.Push(RSGetIndString(LAYER_DIALOG_FLAG_STRINGS_RESID, 4, ACAPI_GetOwnResModule()));

	for (short i = 0; i < (short)lfTextsList.GetSize(); ++i) {
		if (!lfText.IsEmpty())
			lfText += ", ";
		lfText += lfTextsList[i];
	}

	return lfText;
}

GS::UniString	LayerForm::StatusToText(ConvertStatus filterStatus) const
{
	// lay text da ghi trong file AddOn.grc theo id va index;
	return RSGetIndString(LAYER_DIALOG_CONV_STRINGS_RESID, (short)filterStatus + 1, ACAPI_GetOwnResModule()); 
}

void LayerForm::RebuildListContent() 
{
	layerList.DisableDraw(); // chan ve du lieu lien tu khi xoa, nghia la xoa item nhung tren giao dien khong doi
	if (layerList.GetItemCount() != 0) {
		layerList.DeleteItem(DG::ListBox::AllItems); // remove item trong list box;
	}
	for (short i = 1; i < static_cast<short>(sortedList.GetSize()); i++) 
	{
		const ListBoxRow& row = sortedList[i - 1];
		layerList.AppendItem();
		layerList.SetTabItemText(i, LayerForm::layerNameTab, row.layerName);
		layerList.SetTabItemText(i, LayerForm::statusTab, row.flagName);
		layerList.SetTabItemText(i, LayerForm::filterStatusTab, row.convStatName);
	}
	layerList.EnableDraw(); // cho phep de lieu duoc ve lai
	layerList.Redraw(); // ve lai du lieu moi tren giao dien
}


void LayerForm::SortListBy(const TabId sortBy)
{
	class ElementSorter  
	{
		private:
			const LayerForm& dialog; // form layer de sort
			TabId sortBy; // id cua tab tren list box;
			DG::ListBox::ArrowType sortDirection; // trang thai mui ten tren header cua list box;

		public:

			ElementSorter(const LayerForm& dilogIn, GS::Array<ListBoxRow>& row, TabId sortBy, DG::ListBox::ArrowType lastArrow)
				:dialog(dilogIn), sortBy(sortBy)
			{
				if (lastArrow == DG::ListBox::Up) {
					sortDirection = DG::ListBox::Down;
				}
				else {
					sortDirection = DG::ListBox::Up;
				}
				GS::Sort(row.Begin(), row.End(), *this); // sort 
			}

			bool operator() (const ListBoxRow& row1, const ListBoxRow& row2) // ham se quy dinh sap xep nhu the nao
			{
				Int32 cmpRes = 0;
				GS::UniString::CompareResult result;
				GS::UniString name1, name2;
				switch (sortBy) 
				{
					case LayerForm::layerNameTab:
						name1 = row1.layerName;
						name2 = row2.layerName;
						break;
					case LayerForm::statusTab:
						name1 = row1.flagName;
						name2 = row2.flagName;
						break;
					case LayerForm::filterStatusTab:
						name1 = row1.convStatName;
						name2 = row2.convStatName;
						break;
					default:
						DBBREAK();
						break;

				}
				result = name1.Compare(name2); // so sanh 2 string
				GS::UniString::CompareResult whatIsLess = (sortDirection == DG::ListBox::Up ? GS::UniString::Less : GS::UniString::Greater); // xem nhu sep xuoi hay nguoc
				cmpRes = (result == GS::UniString::Equal) ? 0 : ((result == whatIsLess) ? -1 : 1); // ket qua sep theo mui ten
				return cmpRes < 0;
			}

			// tra ve trang thai cua mui ten cho ben ngoai truy cap
			DG::ListBox::ArrowType GetNewArrow() const {
				return sortDirection;
			}
	};

	ElementSorter sorter (*this, sortedList, sortBy, layerList.GetHeaderItemArrowType((short)sortBy)); // tao doi tuong su ly sort
	layerList.SetHeaderItemArrowType((short)lastSortedTab, DG::ListBox::NoArrow); // remove dau mui ten sap xep tren tab cu
	layerList.SetHeaderItemArrowType((short)sortBy, sorter.GetNewArrow()); // set lai dau mui ten sap xep tren tab moi
	lastSortedTab = sortBy; // luu lai trang thai dau mui ten vua sap
	if (layerList.GetHeaderItemArrowType((short)sortBy) == DG::ListBox::Down) {
		lastDirectionUp = false;
	}
	else {
		lastDirectionUp = true;
	}
	RebuildListContent(); // xoa va add lai du lieu theo bien sortedList duoc tra ra sau ham sorter;
}

/// Class Observable;
LayerObserver::LayerObserver(LayerForm* testDialog):dialog(testDialog)
{
	dialog->Attach(*this);
	AttachToAllItems(*dialog);
}

LayerObserver::~LayerObserver() {
	dialog->Detach(*this);
	DetachFromAllItems(*dialog);
}

void LayerObserver::PanelOpened(const DG::PanelOpenEvent&)
{
	dialog->SetClientSize(dialog->GetOriginalClientWidth(), dialog->GetOriginalClientHeight());
}

void LayerObserver::ButtonClicked(const DG::ButtonClickEvent& ev) {
	if (ev.GetSource() == &dialog->okButton) {
		dialog->PostCloseRequest(DG::ModalDialog::Accept);
	}
	else if (ev.GetSource() == &dialog->cancelButton) {
		dialog->PostCloseRequest(DG::ModalDialog::Cancel);
	}
	else if (ev.GetSource() == &dialog->searchButton) {
		Search();
	}
}


void LayerObserver::SetPopUpAfterSelection() {
	short selItem = 0;
	selItem = dialog->layerList.GetSelectedItem(selItem + 1);  
	GS::UniString layerName = dialog->layerList.GetTabItemText(selItem, LayerForm::layerNameTab);
	dialog->filterStatusPopup.SetValue((short)dialog->statusTable[layerName] + 1); // set lai gia tri cho popup active; 
	dialog->layerList.SetOnTabItem(LayerForm::popUpTab, dialog->filterStatusPopup); // set on tab(dau tick) cho item active; 
} 

void LayerObserver::Search()
{
	dialog->layerList.RemoveOnTabItem(LayerForm::popUpTab); // Xoa toan bo cac item cua tab popup
	dialog->layerList.DeselectItem(DG_ALL_ITEMS); // bo cho nhung doi tuong hien tai

	GS::UniString	text = dialog->searchEdit.GetText(); 
	short 			nLayers = dialog->layerList.GetItemCount();
	bool			noSelectedItems = true;
	text.SetToLowerCase();

	for (short j = 1; j <= nLayers; ++j) {
		// lay mot gia tri tren tab cua lish box theo hang va index of tab
		GS::UniString	tabText = dialog->layerList.GetTabItemText(j, LayerForm::layerNameTab);
		tabText.SetToLowerCase();

		if (tabText.Contains(text)) {
			if (noSelectedItems)
				noSelectedItems = false;

			dialog->layerList.SelectItem(j); //set select item tren list box;
		}
	}
	if (!noSelectedItems) {
		SetPopUpAfterSelection();  // Bind lai du lien popup cho doi tuong vua duoc select
	}
}


void LayerObserver::PanelHotkeyPressed(const DG::PanelHotKeyEvent& ev, bool* processed) // set action for hot key o tren;
{
	short keyId = ev.GetKeyId();

	if (keyId == dialog->returnHotKey || keyId == dialog->enterHotKey) {
		*processed = false;

		if (dialog->searchEdit.IsFocused()) // kiem tra xem hien tai co 
		{
			Search();
			dialog->layerList.SetFocus(); // focus  vao item cua list box;
			*processed = true;
		}
	}
}

void LayerObserver::ListBoxClicked(const DG::ListBoxClickEvent& /*ev*/)
{
}

void LayerObserver::PanelResized(const DG::PanelResizeEvent& ev)
{
	short vGrow = ev.GetVerticalChange();
	if (vGrow != 0) {
		dialog->BeginMoveResizeItems();

		dialog->okButton.Move(0, vGrow);
		dialog->cancelButton.Move(0, vGrow);
		dialog->layerList.Resize(0, vGrow);

		dialog->EndMoveResizeItems();
	}
}

void LayerObserver::ListBoxHeaderItemClicked(const DG::ListBoxHeaderItemClickEvent& ev)
{
	if (ev.GetSource() == &dialog->layerList) 
	{
		TabId clickedTab = (TabId)(ev.GetHeaderItem()); // get tab id cua list box;
		dialog->SortListBy(clickedTab); // goi ham sort;
	}
}

void LayerObserver::UserControlChanged(const DG::UserControlChangeEvent& ev) // doi tuong dang duoc click thuc thi;
{
	if (ev.GetSource() == &dialog->filterStatusPopup) {
		short	selRowsNr = DGListGetSelCount(LayerForm::LAYER_DIALOG_RESID, LayerForm::ListBoxId); // Lay tong so hang duoc select trong list box
		short 	selItem = 0;
		ConvertStatus newConvStat = (ConvertStatus)(dialog->filterStatusPopup.GetValue() - 1); // get index of filter status in popup

		for (short i = 0; i < selRowsNr; ++i) {
			selItem = DGListGetSelected(LayerForm::LAYER_DIALOG_RESID, LayerForm::ListBoxId, selItem + 1); // get index selected cua list box

			GS::UniString layName = dialog->layerList.GetTabItemText(selItem, LayerForm::layerNameTab); // lay name cua doi tuong popup
			dialog->statusTable[layName] = newConvStat;  // tao thanh dictionary gom key la layer name, value la status va luu vao statusTable;
			dialog->sortedList[selItem - 1].convStatName = dialog->filterStatusPopup.GetItemText(dialog->filterStatusPopup.GetValue());  // lay gia tri cua popup gian vao filter status
			dialog->layerList.SetTabItemText(selItem, LayerForm::filterStatusTab, dialog->sortedList[selItem - 1].convStatName);
		}
	}
}

void LayerObserver::ListBoxSelectionChanged(const DG::ListBoxSelectionEvent& ev)
{
	if (ev.GetSource() == &dialog->layerList) {
		SetPopUpAfterSelection();
	}
}



static GSErrCode LoadModuleData(GS::HashTable<GS::UniString, ConvertStatus>& statusTable, TabId& lastSortedTab, bool& lastDirectionUp)
{
	GSErrCode		err = NoError;
	API_ModulData	info;

	BNZeroMemory(&info, sizeof(info));
	err = ACAPI_ModulData_Get(&info);
	if (err != NoError)
		return err;

	if (info.dataHdl == nullptr || BMhGetSize(info.dataHdl) == 0)
		return NoError;

	if (info.dataVersion != 6)
		return err;

	GS::MemoryIChannel32 memChannel(*info.dataHdl, BMGetHandleSize(info.dataHdl));

	IO::SetPlatformIProtocol(memChannel, (GS::PlatformSign)info.platformSign);
	memChannel.Read((int&)lastSortedTab);
	memChannel.Read(lastDirectionUp);
	try {
		statusTable.Read(memChannel);
	}
	catch (...) {
		statusTable.Clear();
	}

	err = memChannel.GetInputStatus();

	BMhKill(&info.dataHdl);

	return err;
}

static GSErrCode StoreModuleData(const GS::HashTable<GS::UniString, ConvertStatus>& statusTable, const TabId& lastSortedTab, const bool& lastDirectionUp)
{
	GS::MemoryOChannel32 memChannel(GS::MemoryOChannel32::BMAllocation);
	GSErrCode errCode = memChannel.Write((int&)lastSortedTab);
	if (errCode == NoError) {
		errCode = memChannel.Write(lastDirectionUp);
		if (errCode == NoError) {
			errCode = statusTable.Write(memChannel);
			if (errCode == NoError) {
				/* store module data */
				USize		nBytes = memChannel.GetDataSize();			// bytes in IO buffers not yet flushed are also included
				const char* pData = memChannel.GetDestination();		// flushes IO buffers

				DBASSERT(pData != nullptr);

				API_ModulData	info;
				BNZeroMemory(&info, sizeof(info));
				info.dataVersion = 6;
				info.platformSign = GS::Act_Platform_Sign;

				info.dataHdl = BMAllocateHandle(nBytes, ALLOCATE_CLEAR, 0);
				if (info.dataHdl == nullptr)
					return ErrMemoryFull;

				BNCopyMemory(*info.dataHdl, pData, nBytes);
				errCode = ACAPI_ModulData_Store(&info);

				BMhFree(info.dataHdl);
			}
		}
	}

	return errCode;
}

void Show_LayerDialog()
{
	GS::HashTable<GS::UniString, ConvertStatus> 	statusTable;
	TabId											lastSortedTab;
	bool 											lastDirectionUp;
	if (LoadModuleData(statusTable, lastSortedTab, lastDirectionUp) != NoError) {
		statusTable.Clear();
		lastSortedTab = LayerNameTab;
		lastDirectionUp = true;
	}

	LayerForm		dialog(ACAPI_GetOwnResModule(), statusTable, lastSortedTab, lastDirectionUp);
	LayerObserver	observer(&dialog);

	if (dialog.Invoke())
		StoreModuleData(statusTable, lastSortedTab, lastDirectionUp);

	return;
}
