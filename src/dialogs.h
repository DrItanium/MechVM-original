////////////////////////////////////////////////////////////////////////////////
// plab - Plants, Landscape and Buildings
// Copyright Bjoern Ganster 2007
// This file declares functions to display standard dialogs
////////////////////////////////////////////////////////////////////////////////

#ifndef dialogs__h
#define dialogs__h

#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// MessageBox buttons

#ifndef _WIN32
#include <qmessagebox.h>

const int MB_OK = 1;
const int MB_OKCANCEL = 2;
const int MB_YESNO = 3;
const int MB_YESNOCANCEL = 4;

const int IDOK = QMessageBox::Ok;
const int IDCANCEL = QMessageBox::Cancel;
const int IDYES = QMessageBox::Yes;
const int IDNO = QMessageBox::No;
#endif

////////////////////////////////////////////////////////////////////////////////

// Get Open/Save file names
bool getOpenFileName (BGString& FN);
bool getSaveFileName (BGString& FN);

// Show a message box
// see http://msdn2.microsoft.com/en-us/library/ms645505.aspx
int messageBox (const TCHAR* title, const TCHAR* message, int utype = 0);

// Present user with several options to a question
//int ask (const TCHAR* title, const TCHAR* msg, const TCHAR* buttonTexts);

#endif
