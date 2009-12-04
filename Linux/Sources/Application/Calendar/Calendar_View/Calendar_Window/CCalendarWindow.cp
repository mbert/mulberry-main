/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CCalendarWindow class

#include "CCalendarWindow.h"

#include "CCalendarView.h"
#include "CCommands.h"
#include "CEventPreview.h"
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "HResourceMap.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"

#include <algorithm>
#include <cassert>

// Static

// Static members

cdmutexprotect<CCalendarWindow::CCalendarWindowList> CCalendarWindow::sCalendarWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarWindow::CCalendarWindow(JXDirector* owner) :
	CWindow(owner)
{
	mPreviewVisible = true;
	mNode = NULL;

	// Add to list
	{
		cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
		sCalendarWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this, false);
}

// Default destructor
CCalendarWindow::~CCalendarWindow()
{
	// Save window state
	SaveState();

	// Deactivate the node (will close calendar if no more references to it)
	if (mNode != NULL)
		calstore::CCalendarStoreManager::sCalendarStoreManager->DeactivateNode(mNode);
	
	// Remove from list
	{
		cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
		CCalendarWindowList::iterator found = std::find(sCalendarWindows->begin(), sCalendarWindows->end(), this);
		if (found != sCalendarWindows->end())
			sCalendarWindows->erase(found);
	}
	CWindowsMenu::RemoveWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CCalendarWindow::MakeWindow(calstore::CCalendarStoreNode* node)
{
	// Look for existing window with this node
	
	CCalendarWindow* newWindow = NULL;
	try
	{
		// Activate the node
		if (node != NULL)
			calstore::CCalendarStoreManager::sCalendarStoreManager->ActivateNode(node);
		
		// Create the message window
		newWindow = new CCalendarWindow(CMulberryApp::sApp);
		newWindow->OnCreate();
		newWindow->SetNode(node);
		newWindow->ResetState();
		newWindow->Show();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (WindowExists(newWindow))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here
		CLOG_LOGRETHROW;
		throw;
	}
}

// Create a free busy window
void CCalendarWindow::CreateFreeBusyWindow(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date)
{
	// Look for existing window with this node

	CCalendarWindow* newWindow = NULL;
	try
	{
		// Create the message window
		newWindow = new CCalendarWindow(CMulberryApp::sApp);
		newWindow->OnCreate();
		newWindow->SetFreeBusy(calref, id, organizer, attendees, date);
		newWindow->Show();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (WindowExists(newWindow))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here
		CLOG_LOGRETHROW;
		throw;
	}
}


// Create it or bring it to the front
void CCalendarWindow::CreateSubscribedWindow()
{
	// Create find & replace window or bring to front
	CCalendarWindow* found = FindWindow(NULL);
	if (found != NULL)
	{
		FRAMEWORK_WINDOW_TO_TOP(found)
	}
	else
	{
		MakeWindow(NULL);
	}
}

void CCalendarWindow::CloseAllWindows()
{
	cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
	while(sCalendarWindows->size() != 0)
		FRAMEWORK_DELETE_WINDOW(sCalendarWindows->back());
}

const int cWindowWidth = 500;
const int cWindowHeight = 200;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cSplitterTop = cToolbarTop + cToolbarHeight;
const int cSplitterHeight = cWindowHeight - cSplitterTop;

// Do various bits
void CCalendarWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,200, "");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout

	window->SetTitle("Subscribed Calendars");

	// Get toolbar
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Deal with splitter
	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cSplitterHeight/2);
	heights.InsertElementAtIndex(2, cSplitterHeight/2);
	minHeights.InsertElementAtIndex(1, 16);
	minHeights.InsertElementAtIndex(2, 16);

	mSplitter = new CSplitterView(heights, 1, minHeights, window,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);
	
	// Get calendar views
	mCalendarView = new CCalendarView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
	mCalendarView->OnCreate();

	mPreview = new CEventPreview(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 550, 200);
	mPreview->OnCreate();

	GetCalendarView()->SetPreview(mPreview);

	// Install the splitter items
	mSplitter->InstallViews(GetCalendarView(), mPreview, false);

	GetToolbarView()->SetSibling(mSplitter);
	GetCalendarView()->MakeToolbars(GetToolbarView());
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetCalendarViewOptions().GetShowPreview())
		ShowPreview(false);
	
	GetCalendarView()->InitView();
	GetCalendarView()->Focus();

	// Do inherited
	CWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);


	// Set current width and height as minimum
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cCalendarWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_CALENDARDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	GetCalendarView()->SetOpen();
}

// Respond to commands
bool CCalendarWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eWindowsSaveDefault:
		SaveDefaultState();
		return true;

	case CCommand::eWindowsResetToDefault:
		ResetState(true);
		return true;

	case CCommand::eWindowsShowPreview:
	case CCommand::eToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		return true;

	default:;
	}

	return CWindow::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CCalendarWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eWindowsSaveDefault:
	case CCommand::eWindowsResetToDefault:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eWindowsShowPreview:
	{
		// Always enabled but text may change
		OnUpdateShowPreview(cmdui);
		return;
	}

	case CCommand::eToolbarShowPreview:
	{
		// Always enabled but text may change
		OnUpdateShowPreviewBtn(cmdui);
		return;
	}

	default:;
	}

	CWindow::UpdateCommand(cmd, cmdui);
}

void CCalendarWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	pCmdUI->SetText(txt);
}

void CCalendarWindow::OnUpdateShowPreviewBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

void CCalendarWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change preview state
	mSplitter->ShowView(true, show);
	GetCalendarView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetCalendarViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetCalendarViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}
