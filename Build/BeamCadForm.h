#pragma once
#include "DGModule.hpp"
#include "ResourceIds.hpp"



class BeamCadDialog :public DG::ModalDialog
{
	friend class BeamObserver;
protected:
private:
	static const short BEAMCAD_DIALOG_RESID = 32591;
	enum {
		CreateBeamButtonId = 1,
		CancelButtonId = 2
	};

	DG::Button createBeamCadButton;
	DG::Button cancelButton;

public:
	BeamCadDialog(GSResModule dialResModule);
	~BeamCadDialog();


};


class BeamObserver :
	public DG::PanelObserver,
	public DG::ButtonItemObserver,
	public DG::CompoundItemObserver
{
private:
	BeamCadDialog* beamdialog;

protected:
	virtual void	PanelOpened(const DG::PanelOpenEvent& ev) override;
	virtual void	PanelResized(const DG::PanelResizeEvent& ev) override;
	virtual void	ButtonClicked(const DG::ButtonClickEvent& ev) override;

public:
	explicit BeamObserver(BeamCadDialog* testDialog);
	~BeamObserver();
};



