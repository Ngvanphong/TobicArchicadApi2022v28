#include "BeamCadForm.h"
#include "APIEnvir.h"
#include "ACAPinc.h"
#include "BeamCad.h"
#include "../Build/ReadDataCad.h"

BeamCadDialog::BeamCadDialog(GSResModule dialResModule) :
	DG::ModalDialog(dialResModule, BEAMCAD_DIALOG_RESID, dialResModule),
	createBeamCadButton(GetReference(), CreateBeamButtonId),
	cancelButton(GetReference(), CancelButtonId)
{
}

BeamCadDialog::~BeamCadDialog()
{
}

//-------------------------- Class BeamObserver -----------------------

BeamObserver::BeamObserver(BeamCadDialog* testDialog)
	:beamdialog(testDialog)
{
	beamdialog->Attach(*this);
	AttachToAllItems(*beamdialog);
}

BeamObserver::~BeamObserver()
{
	beamdialog->Detach(*this);
	DetachFromAllItems(*beamdialog);
}

void BeamObserver::PanelOpened(const DG::PanelOpenEvent& ev)
{
	beamdialog->SetClientSize(beamdialog->GetOriginalClientWidth(), beamdialog->GetOriginalClientHeight());
}

void BeamObserver::PanelResized(const DG::PanelResizeEvent& ev)
{
	short vGrow = ev.GetVerticalChange();
	if (vGrow != 0) {
		beamdialog->BeginMoveResizeItems();

		beamdialog->createBeamCadButton.Move(0, vGrow);
		beamdialog->cancelButton.Move(0, vGrow);

		beamdialog->EndMoveResizeItems();
	}
}

void BeamObserver::ButtonClicked(const DG::ButtonClickEvent& ev)
{
	if (ev.GetSource() == &beamdialog->cancelButton) {
		beamdialog->PostCloseRequest(DG::ModalDialog::Cancel);
	}
	else if (ev.GetSource() == &beamdialog->createBeamCadButton) {
		beamdialog->PostCloseRequest(DG::ModalDialog::Accept);
	}
}