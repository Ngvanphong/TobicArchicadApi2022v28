#pragma once
#include "UDRollDownDialogs.hpp"

void Show_BasicElementIconsRollDownDialog(void);

class ElementIConsRollDownDialog :public UD::RollDownModalDialog 
{
private:
	GS::Guid dialogGuid = GS::Guid("BA851C84-3E34-4034-BB98-DF50EB7E020C");
public:
	ElementIConsRollDownDialog(DG::NativePoint* position);
	~ElementIConsRollDownDialog();

private:
	void DestroyPage(const UD::IPanel& panel);
};

class HeaderTabPage : public DG::TabPage 
{
public:
	HeaderTabPage(const DG::TabControl& tabControl);
	~HeaderTabPage();
};

class FirstRollDownTabPage : public DG::TabPage 
{
public:
	FirstRollDownTabPage(const DG::TabControl& tabControl);
	~FirstRollDownTabPage(void);
};

class SecondRollDownTabPage : public DG::TabPage
{
public:
	SecondRollDownTabPage(const DG::TabControl& tabControl);
	~SecondRollDownTabPage(void);
};

class ThirdRollDownTabPage : public DG::TabPage
{
public:
	ThirdRollDownTabPage(const DG::TabControl& tabControl);
	~ThirdRollDownTabPage(void);
};


class BottomTabPage : public DG::TabPage, private DG::ButtonItemObserver
{
private:
	enum ItemIDs {
		CloseButtonId= 1
	};
	DG::Button closeButton;
	DG::ModalDialog& parentDialog;

public:
	BottomTabPage(const DG::TabControl& tabControl, DG::ModalDialog& parentDialog);
	~BottomTabPage();
private:
	virtual void ButtonClicked(const DG::ButtonClickEvent& ev);
};

