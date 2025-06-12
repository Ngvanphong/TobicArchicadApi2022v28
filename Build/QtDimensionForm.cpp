#include "QtDimensionForm.h"

void ExampleDialog::PanelResized(const DG::PanelResizeEvent& ev) {
	BeginMoveResizeItems();
	okButton.Move(ev.GetHorizontalChange(), ev.GetVerticalChange());
	cancelButton.Move(ev.GetHorizontalChange(), ev.GetVerticalChange());
	separator.MoveAndResize(0, ev.GetVerticalChange(), ev.GetHorizontalChange(), 0);
	textEdit.Resize(ev.GetHorizontalChange(), 0);
	EndMoveResizeItems();
}

void  ExampleDialog::ButtonClicked(const DG::ButtonClickEvent& ev) {
	if (ev.GetSource() == &okButton) {
		PostCloseRequest(DG::ModalDialog::Accept);
		g_value = textEdit.GetText();
	}
	else if (ev.GetSource() == &cancelButton) {
		PostCloseRequest(DG::ModalDialog::Cancel);
	}
}

void ExampleDialog::PanelOpened(const DG::PanelOpenEvent& ev) {
	ev.GetStatus();
	std::string sLine = readFile();
	if (sLine.size() > 0)
	{
		textEdit.SetText(sLine.c_str());
	}
	else
	{
		textEdit.SetText("200");
	}

}

std::string readFile()
{
	TCHAR path_buf[MAX_PATH];
	DWORD ret_val = GetTempPath(MAX_PATH, path_buf);
	_tcscat(path_buf, TEXT("text.txt"));
	std::string sLine;
	if (ret_val < MAX_PATH || (ret_val != 0))
	{
		std::ifstream infile;
		infile.open(path_buf);

		if (!infile.fail())
		{
			while (!infile.eof())
			{
				infile >> sLine;
			}
			if (sLine.size() > 0)
			{
				return sLine;
			}

		}

	}
	return sLine;
}

void SaveFile(std::string saveValue)
{
	TCHAR path_buf[MAX_PATH];
	DWORD ret_val = GetTempPath(MAX_PATH, path_buf);
	_tcscat(path_buf, TEXT("text.txt"));
	if (ret_val < MAX_PATH || (ret_val != 0))
	{
		std::ofstream fw(path_buf, std::ofstream::out);
		fw << saveValue << "\n";
		fw.close();
	}
}