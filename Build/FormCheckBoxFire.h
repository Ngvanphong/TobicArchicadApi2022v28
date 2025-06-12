#pragma once
#include "DGModule.hpp"
#include "DGMenu.hpp"
#include "DGCommandDescriptor.hpp"
#include "UCModule.hpp"

void ShowCheckBoxFireDiaglog(void);

class CheckBoxFireDialog: public DG::ModalDialog,
	public DG::PanelObserver,public DG::CheckItemObserver, public DG::ButtonItemObserver, public UC::FontPopUpObserver
{
protected:
	enum Controls
	{
		RectangleID = 1,
		HID = 2,
		OkButtonID = 3
	};

	DG::PushCheck rectangeCheck;
	DG::PushCheck hCheck;
	DG::Button okButton;

	virtual void	CheckItemChanged(const DG::CheckItemChangeEvent&)	override;  // su kien bat thay doi cua check box
	virtual void	ButtonClicked(const DG::ButtonClickEvent& ev)	override;

private:
	static const short DGTEST_FONTPOPUPDIALOG_RESID = 32678; // id cua dialog trong file AddOn.grc
public:

	CheckBoxFireDialog();
	virtual ~CheckBoxFireDialog();

	static	bool IsCheckH;
};

