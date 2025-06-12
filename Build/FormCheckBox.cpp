#include "FormCheckBox.h"

#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h

#include	"MDIDs_APICD.h"

#include	"DGTEFontData.hpp"

CheckBoxDialog::CheckBoxDialog() :
	DG::ModalDialog(ACAPI_GetOwnResModule(), DGTEST_FONTPOPUPDIALOG_RESID, ACAPI_GetOwnResModule()), // khoi tao mot modal dialog;
	UC::FontPopUpObserver(), // dilog popup de hien thi font, co san trong archicad;
	fontPopUp(GetReference(), FontPopUpID),  // set reference cua popup trung voi id dat trong file AddOn.grc
	richEdit(GetReference(), RichEditID),
	boldCheck(GetReference(), BoldCheckID),
	italicCheck(GetReference(), ItalicCheckID),
	underlineCheck(GetReference(), UnderlineCheckID),
	closeButton(GetReference(), CloseButtonID)
{
	fontPopUp.Attach(*this);
	boldCheck.Attach(*this);
	italicCheck.Attach(*this);
	underlineCheck.Attach(*this);
	closeButton.Attach(*this);
	Attach(*this);

	fontPopUp.SelectItem(TE::FontFamily("Arial"));  // set mac dinh chon mot font cho popup
	richEdit.SetText("Hello"); // set gia tri cho rich text;
	UpdateFont();
}

CheckBoxDialog::~CheckBoxDialog()
{
	//for (auto it = commandTable.EnumerateValues(); it != nullptr; ++it) {
	//	delete* it;
	//}
}


void CheckBoxDialog::CheckItemChanged(const DG::CheckItemChangeEvent& /*ev*/) 
{
	UpdateFont();
}

void CheckBoxDialog::FontPopUpChanged(const UC::FontPopUpChangeEvent& /*ev*/) {
	UpdateFont();
}


void CheckBoxDialog::ButtonClicked(const DG::ButtonClickEvent& ev) {
	if (ev.GetSource() == &closeButton) {
		PostCloseRequest(Cancel);
	}
}

void CheckBoxDialog::UpdateFont() {
	DGTEFontData data; // doi tuong font;
	if (!richEdit.GetFont(&data)) {
		return;
	}
	data.font.Set(fontPopUp.GetSelectedItem());
	Int32 fontStyle= (boldCheck.IsChecked()? TE::Font::FontStyle::bold:0)|
		(italicCheck.IsChecked() ? TE::IFont::FontStyle::italic : 0) |
		(underlineCheck.IsChecked() ? TE::IFont::FontStyle::underline : 0);
	data.font.SetStyle(fontStyle);
	richEdit.SetFont(&data); // set font cho chu trong rich text;
}

void Show_FontPopUpDialog()
{
	CheckBoxDialog dialog;
	if (DBERROR(dialog.GetId() == 0))
		return;
	dialog.Invoke();
}
