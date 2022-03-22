#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include "wxcrafter.h"

class MainDialog : public MainDialogBaseClass
{
public:
    MainDialog(wxWindow* parent);
    virtual ~MainDialog();
protected:
    virtual void OnInit(wxInitDialogEvent& event);
    virtual void OnExit(wxCloseEvent& event);
    virtual void OnButtonOK(wxCommandEvent& event);
};
#endif // MAINDIALOG_H
