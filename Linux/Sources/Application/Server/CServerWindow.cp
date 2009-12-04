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


// CServerWindow.cp : implementation of the CServerWindow class
//


#include "CServerWindow.h"

#include "CErrorDialog.h"
#include "CMenuBar.h"
#include "CMulberryWMClass.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CWindowsMenu.h"
#include "C3PaneWindow.h"

#include "HResourceMap.h"

#include <JXMenuBar.h>

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CServerWindow construction/destruction

cdmutexprotect<CServerWindow::CServerWindowList> CServerWindow::sServerWindows;

CServerWindow::CServerWindow(JXDirector* owner)
  : CTableViewWindow(owner), mDoQuit(false)
{
	// Add to list
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		sServerWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this, true);
}

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

	if (mDoQuit)
		CMulberryApp::sApp->Quit();
}

// Manually create document

CServerWindow* CServerWindow::ManualCreate(void)
{
	CServerWindow * pWnd = new CServerWindow(CMulberryApp::sApp);
	if (pWnd)
  		pWnd->OnCreate();
	return pWnd;
}

// Check for window
bool CServerWindow::WindowExists(const CServerWindow* wnd)
{
	cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
	CServerWindowList::iterator found = std::find(sServerWindows->begin(), sServerWindows->end(), wnd);
	return found != sServerWindows->end();
}

// Hide instead of close
bool CServerWindow::AttemptClose()
{
	// Save window state
	SaveState();

	// See whether at least one server window is still visible
	bool one_visible = false;
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		for(CServerWindow::CServerWindowList::iterator iter = CServerWindow::sServerWindows->begin(); iter != CServerWindow::sServerWindows->end(); iter++)
		{
			if (((*iter) != this) && !(*iter)->GetWindow()->IsIconified())
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
		//If we're already quitting then it doesn't matter
		if (CMulberryApp::mQuitting) {
			return true;
		}
		// Ask user for quit
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ServerCloseQuit",
																		"ErrorDialog::Btn::ServerCloseClose",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::ServerClose", 3);

		if (result == CErrorDialog::eBtn2)
			// Cancel close attempt
			return kFalse;
		else if (result == CErrorDialog::eBtn1)
		{
			// Do app quit
			mDoQuit = true;
			return true;
		}
	}
	// Just hide if manager
	if (GetServerView()->GetTable()->IsManager() && !C3PaneWindow::s3PaneWindow)
	{
		GetWindow()->Iconify();
		return false;
	}
	else
		// Carry on with close attempt
		return true;
}

const int cWindowWidth = 340;
const int cWindowHeight = 270;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cViewTop = cToolbarTop + cToolbarHeight;
const int cViewHeight = cWindowHeight - cViewTop;

void CServerWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,270, "Servers");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout
		
	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Create server view
	mServerView = new CServerView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cViewTop, 500, 200);
	mServerView->OnCreate();
	mServerView->AdjustSize(cWindowWidth - 500, cViewHeight - 200);
	SetTableView(mServerView);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(GetServerView());
	GetServerView()->MakeToolbars(GetToolbarView());
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Do inherited
	CTableViewWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fMailboxes | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Set current width and height as minimum
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cServerWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_SERVERDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	GetServerView()->SetOpen();
}
