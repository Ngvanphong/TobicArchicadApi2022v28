#pragma once
#include "DGModule.hpp"
#include "DGMenu.hpp"
#include "DGCommandDescriptor.hpp"
#include "UCModule.hpp"

void Show_FontPopUpDialog(void);


class CheckBoxDialog : public DG::ModalDialog,
	public DG::PanelObserver,public UC::FontPopUpObserver, public DG::CheckItemObserver, public DG::ButtonItemObserver
{
protected:
	enum Controls
	{
		FontPopUpID = 1,
		RichEditID = 2,
		BoldCheckID = 3,
		ItalicCheckID = 4,
		UnderlineCheckID = 5,
		CloseButtonID = 6
	};

	//enum MenuStrings {
	//	Restore = 1,
	//	Close = 2
	//};

	UC::FontPopUp fontPopUp;
	DG::RichEdit richEdit;
	DG::PushCheck boldCheck; // khai bao check box 1
	DG::PushCheck italicCheck; // khai bao check box 2
	DG::PushCheck underlineCheck; // khai bao check box 3
	DG::Button closeButton;

	//DG::Menu menu;
	//DG::Command restoreCommand;
	//DG::Command closeCommand;
	//DG::CommandTable commandTable;

	virtual void	CheckItemChanged(const DG::CheckItemChangeEvent&)	override;  // su kien bat thay doi cua check box
	virtual void	FontPopUpChanged(const UC::FontPopUpChangeEvent&)	override;
	virtual void	ButtonClicked(const DG::ButtonClickEvent& ev)	override;
	void			UpdateFont();

private:
	static const short DGTEST_FONTPOPUPDIALOG_RESID = 32655; // id cua dialog trong file AddOn.grc
public:
	CheckBoxDialog();
	virtual ~CheckBoxDialog();
};

