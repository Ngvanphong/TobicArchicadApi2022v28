#pragma once
#include "APIEnvir.h"
#include "ACAPinc.h"
#include "DGModule.hpp"
#include "ResourceIds.hpp"
#include <tchar.h>
#include <fstream>

std::string ReadFileText();
void FileTextSave(std::string);

class OffsetFromTopForm: public DG::ModalDialog,
	public DG::PanelObserver,
	public DG::ButtonItemObserver,
	public DG::CompoundItemObserver
{
public:

	GS::UniString g_value;
	enum DialogResourceIds
	{
		ExampleDialogResourceId = 32666,
		OKButtonId = 1,
		CancelButtonId = 2,
		SeparatorId = 3,
		TextInputId = 4
	};
	OffsetFromTopForm() :
		DG::ModalDialog(ACAPI_GetOwnResModule(), ExampleDialogResourceId, ACAPI_GetOwnResModule()),
		okButton(GetReference(), OKButtonId),
		cancelButton(GetReference(), CancelButtonId),
		separator(GetReference(), SeparatorId),
		textEdit(GetReference(), TextInputId) 
	{
		AttachToAllItems(*this);
		Attach(*this);
	}
	~OffsetFromTopForm() 
	{
		Detach(*this);
		DetachFromAllItems(*this);
	}

private:
	virtual void PanelResized(const DG::PanelResizeEvent& ev) override;

	virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;

	virtual void PanelOpened(const DG::PanelOpenEvent& ev) override;


	DG::Button		okButton;
	DG::Button		cancelButton;
	DG::Separator	separator;
	DG::TextEdit	textEdit;

};

