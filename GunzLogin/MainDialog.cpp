#include "MainDialog.h"
#include <wx/msgdlg.h>
#include <wx/file.h>

MainDialog::MainDialog(wxWindow* parent)
    : MainDialogBaseClass(parent)
{
}

MainDialog::~MainDialog()
{
}

void MainDialog::OnButtonOK(wxCommandEvent& event)
{
	// check empty id/pw.
	if(m_textCtrlID->GetValue() == "")
	{
		wxMessageBox("Please fill the user name field.", "User name is empty", wxOK | wxICON_WARNING);
		return;
	}
	else if(m_textCtrlPW->GetValue() == "")
	{
		wxMessageBox("Please fill the password field.", "Password is empty", wxOK | wxICON_WARNING);
		return;
	}
	
	// check id/pw length.
	if(m_textCtrlID->GetValue().length() >= 64)
	{
		wxMessageBox("User name is over of allowable length.", "User name is too long", wxOK | wxICON_WARNING);
		return;
	}
	else if(m_textCtrlPW->GetValue().length() >= 64)
	{
		wxMessageBox("Password is over of allowable length.", "Password is too long", wxOK | wxICON_WARNING);
		return;
	}
	
	m_buttonOK->Disable();
	
	// launch gunz.exe with ID as arg 1, PW as arg 2.
	wxExecute(wxString::Format(".\\Gunz.exe \"%s\" \"%s\"", m_textCtrlID->GetValue().mb_str(), m_textCtrlPW->GetValue().mb_str()), wxEXEC_SYNC);
	
	// save last id.
	wxFile file("lastid.conf", wxFile::write);
	if(file.IsOpened() == true)
	{
		file.Write(m_textCtrlID->GetValue().mb_str());
		file.Close();
	}
	
	EndModal(0);
}

void MainDialog::OnExit(wxCloseEvent& event)
{
	EndModal(-1);
}

void MainDialog::OnInit(wxInitDialogEvent& event)
{
	// load last user id.
	wxFile file("lastid.conf", wxFile::read);
	if(file.IsOpened() == false) return;
	
	char szUserID[64]; memset(szUserID, 0, sizeof(szUserID));
	file.Read(szUserID, sizeof(szUserID) - 1);
	
	m_textCtrlID->SetValue(szUserID);
	
	file.Close();
}
