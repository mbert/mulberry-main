/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Source for CServerWindow class

#include "CServerWindow.h"

#include "CErrorDialog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "C3PaneWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

cdmutexprotect<CServerWindow::CServerWindowList> CServerWindow::sServerWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerWindow::CServerWindow()
{
	// Add to list
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		sServerWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this, true);
}

// Constructor from stream
CServerWindow::CServerWindow(LStream *inStream)
		: CTableViewWindow(inStream)
{
	// Add to list
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		sServerWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this, true);
}

// Default destructor
CServerWindow::~CServerWindow()
{
	// Remove from list
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		CServerWindowList::iterator found = std::find(sServerWindows->begin(), sServerWindows->end(), this);
		if (found != sServerWindows->end())
			sServerWindows->erase(found);
	}
	CWindowsMenu::RemoveWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CServerWindow::FinishCreateSelf()
{
	// Do inherited
	CTableViewWindow::FinishCreateSelf();

	SetLatentSub(GetServerView()->GetTable());

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_ServerToolbarView);
	mToolbarView->SetSibling(GetServerView());

	// Create toolbars for a view we own
	GetServerView()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
}

// Check for window
bool CServerWindow::WindowExists(const CServerWindow* wnd)
{
	cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
	CServerWindowList::iterator found = std::find(sServerWindows->begin(), sServerWindows->end(), wnd);
	return found != sServerWindows->end();
}

// Hide instead of close
void CServerWindow::AttemptClose()
{
	// Save window state
	SaveState();

	// See whether at least one server window is still visible
	bool one_visible = false;
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		for(CServerWindowList::const_iterator iter = sServerWindows->begin(); iter != sServerWindows->end(); iter++)
		{
			if (((*iter) != this) && (*iter)->IsVisible())
			{
				one_visible = true;
				break;
			}
		}
	}

	// If there are no 1-pane windows left and no 3-pane window the user
	// may be intending to quit
	if (!one_visible && !C3PaneWindow::s3PaneWindow)
	{
		// Ask user for quit
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ServerCloseQuit",
																		"ErrorDialog::Btn::ServerCloseClose",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::ServerClose", 3);
		if (result == CErrorDialog::eBtn3)
			return;
		if (result == CErrorDialog::eBtn1)
			CMulberryApp::sApp->DoQuit();
	}

	// Just hide if manager
	if (GetServerView()->GetTable()->IsManager())
		Hide();
	else
		// Close the view here - this will delete the window at idle time
		GetServerView()->DoClose();
}
