#include "APIEnvir.h"
#include "ACAPinc.h"
#include "FromRollDown.h"

#define RollDownDialogStringsId	32660

#define RollDownHeaderTabPageId	32660
#define FirstRollDownTabPageId	32661
#define SecondRollDownTabPageId	32662
#define ThirdRollDownTabPageId	32663
#define RollDownBottomTabPageId	32664

ElementIConsRollDownDialog::ElementIConsRollDownDialog(DG::NativePoint* position):
	UD::RollDownModalDialog(dialogGuid,position) 
{
	SetGrowType(NoGrow);
	UD::IRollDownSurface* rollDownSurface = GetRollDownSurface();
	bool valid = rollDownSurface != nullptr && UD::RollDownModalDialog::IsValid();

	try 
	{
		UD::IRollDownPanelSet* rollDownPanelSet = nullptr;
		if (valid) {
			SetTitle(RSGetIndString(RollDownDialogStringsId, 1, ACAPI_GetOwnResModule()));
			rollDownPanelSet = rollDownSurface->CreateRollDownPanelSet();
			if (rollDownPanelSet == nullptr) 
			{
				valid = false;
			}
		}

		if (valid) {
			UD::IPanel* headerPanel = rollDownSurface->CreateHeaderPanel();
			if (headerPanel == nullptr) {
				valid = false;
			}
			else {
				new HeaderTabPage(headerPanel->GetTabControl());
			}
		}

		if (valid) {
			UD::IRollPanel* firstRollDownPanel = rollDownPanelSet->CreateRollDownPanel();
			firstRollDownPanel->SetTitle(RSGetIndString(RollDownDialogStringsId, 2, ACAPI_GetOwnResModule()));
			firstRollDownPanel->SetGrowType(UD::IRollPanel::NoGrow);
			if (firstRollDownPanel == nullptr)
				valid = false;
			else
				new FirstRollDownTabPage(firstRollDownPanel->GetTabControl());
		}

		if (valid) {
			UD::IRollPanel* secondRollDownPanel = rollDownPanelSet->CreateRollDownPanel();
			secondRollDownPanel->SetTitle(RSGetIndString(RollDownDialogStringsId, 3, ACAPI_GetOwnResModule()));
			secondRollDownPanel->SetGrowType(UD::IRollPanel::NoGrow);
			if (secondRollDownPanel == nullptr)
				valid = false;
			else
				new SecondRollDownTabPage(secondRollDownPanel->GetTabControl());
		}

		if (valid) {
			UD::IRollPanel* thirdRollDownPanel = rollDownPanelSet->CreateRollDownPanel();
			thirdRollDownPanel->SetTitle(RSGetIndString(RollDownDialogStringsId, 4, ACAPI_GetOwnResModule()));
			thirdRollDownPanel->Collapse(); // Collapse by default
			thirdRollDownPanel->SetGrowType(UD::IRollPanel::NoGrow);
			if (thirdRollDownPanel == nullptr)
				valid = false;
			else
				new ThirdRollDownTabPage(thirdRollDownPanel->GetTabControl());
		}

		if (valid) {
			UD::IPanel* bottomPanel = rollDownSurface->CreateBottomPanel();
			if (bottomPanel == nullptr) {
				valid = false;
			}
			else {
				new BottomTabPage(bottomPanel->GetTabControl(), *this);
			}
		}
	}
	catch(...)
	{
		DBBREAK();
	}

}


ElementIConsRollDownDialog::~ElementIConsRollDownDialog() 
{
	UD::IRollDownSurface* rollDownSurface = GetRollDownSurface();
	if (rollDownSurface != nullptr) {
		UD::IPanel* headerPanel = rollDownSurface->GetHeaderPanel();
		if (headerPanel != nullptr) {
			DestroyPage(*headerPanel);
		}

		for (UD::IRollDownSurface::ConstIterator its = rollDownSurface->EnumerateRollDownPanelSets(); its != nullptr; ++its) {
			for (UD::IRollDownPanelSet::ConstIterator itp = (*its)->EnumerateRollDownPanels(); itp != nullptr; ++itp) {
				DestroyPage(**itp);
			}
		}

		UD::IPanel* bottomPanel = rollDownSurface->GetBottomPanel();
		if (bottomPanel != nullptr)
			DestroyPage(*bottomPanel);

	}
}

void ElementIConsRollDownDialog::DestroyPage(const UD::IPanel& panel)
{
	DG::TabPage* tabPage = panel.GetTabControl().GetTabPage(1);
	if (DBVERIFY(tabPage != nullptr))
		delete tabPage;
}

HeaderTabPage::HeaderTabPage(const DG::TabControl& tabControl) :
	DG::TabPage(tabControl, 1, ACAPI_GetOwnResModule(), RollDownHeaderTabPageId, ACAPI_GetOwnResModule())
{
}


HeaderTabPage::~HeaderTabPage(void)
{
}


FirstRollDownTabPage::FirstRollDownTabPage(const DG::TabControl& tabControl) :
	DG::TabPage(tabControl, 1, ACAPI_GetOwnResModule(), FirstRollDownTabPageId, ACAPI_GetOwnResModule())
{
}


FirstRollDownTabPage::~FirstRollDownTabPage(void)
{
}

SecondRollDownTabPage::SecondRollDownTabPage(const DG::TabControl& tabControl) :
	DG::TabPage(tabControl, 1, ACAPI_GetOwnResModule(), SecondRollDownTabPageId, ACAPI_GetOwnResModule())
{
}


SecondRollDownTabPage::~SecondRollDownTabPage(void)
{
}


ThirdRollDownTabPage::ThirdRollDownTabPage(const DG::TabControl& tabControl) :
	DG::TabPage(tabControl, 1, ACAPI_GetOwnResModule(), ThirdRollDownTabPageId, ACAPI_GetOwnResModule())
{
}


ThirdRollDownTabPage::~ThirdRollDownTabPage(void)
{
}



BottomTabPage::BottomTabPage(const DG::TabControl& tabControl, DG::ModalDialog& parentDialog) :
	DG::TabPage(tabControl, 1, ACAPI_GetOwnResModule(), RollDownBottomTabPageId, ACAPI_GetOwnResModule()),
	closeButton(GetReference(), CloseButtonId),
	parentDialog(parentDialog)
{
	closeButton.Attach(*this);
}

BottomTabPage::~BottomTabPage(void)
{
	closeButton.Detach(*this);
}


void BottomTabPage::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource()->GetId() == CloseButtonId)
		parentDialog.PostCloseRequest(DG::ModalDialog::Accept);
}

void Show_BasicElementIconsRollDownDialog(void)
{
	ElementIConsRollDownDialog	dialog(nullptr);
	if (DBERROR(dialog.GetId() == 0))
		return;

	dialog.Invoke();
}
