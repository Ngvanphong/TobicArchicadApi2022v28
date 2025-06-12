#include "FormCheckBoxFire.h"
#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"MDIDs_APICD.h"

#include	"DGTEFontData.hpp"

bool CheckBoxFireDialog::IsCheckH = true;

void CheckBoxFireDialog::CheckItemChanged(const DG::CheckItemChangeEvent&)
{
}

void CheckBoxFireDialog::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource() == &okButton) 
	{
		if (rectangeCheck.IsChecked()) {
			IsCheckH = false;
		}
		else 
		{
			IsCheckH = true;
		}
		PostCloseRequest(Accept);
	}
}

CheckBoxFireDialog::CheckBoxFireDialog():
	DG::ModalDialog(ACAPI_GetOwnResModule(), DGTEST_FONTPOPUPDIALOG_RESID, ACAPI_GetOwnResModule()), // khoi tao mot modal dialog;
	rectangeCheck(GetReference(),RectangleID),hCheck(GetReference(),HID), okButton(GetReference(),OkButtonID)
{
	rectangeCheck.Attach(*this);
	hCheck.Attach(*this);
	okButton.Attach(*this);
}

CheckBoxFireDialog::~CheckBoxFireDialog()
{
	

}

void ShowCheckBoxFireDiaglog(void)
{
	CheckBoxFireDialog dialog;
	if (DBERROR(dialog.GetId() == 0))
		return;
	dialog.Invoke();
}
