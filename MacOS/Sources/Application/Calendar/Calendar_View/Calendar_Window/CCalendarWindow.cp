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


// Source for CCalendarWindow class

#include "CCalendarWindow.h"

#include "CCalendarView.h"
#include "CCommands.h"
#include "CEventPreview.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"

// Static

// Static members

cdmutexprotect<CCalendarWindow::CCalendarWindowList> CCalendarWindow::sCalendarWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarWindow::CCalendarWindow(LStream *inStream) :
	LWindow(inStream),
	LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this),
	CHelpTagWindow(this)
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
		newWindow = (CCalendarWindow*) CreateWindow(paneid_CalendarWindow, CMulberryApp::sApp);
		newWindow->SetNode(node);
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
		newWindow = (CCalendarWindow*) CreateWindow(paneid_CalendarWindow, CMulberryApp::sApp);
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

// Do various bits
void CCalendarWindow::FinishCreateSelf()
{
	// Do inherited
	LWindow::FinishCreateSelf();
	SetupHelpTags();

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_CalendarToolbarView);

	// Deal with splitter
	mSplitter = (CSplitterView*) FindPaneByID(paneid_CalendarSplitterView);
	mToolbarView->SetSibling(mSplitter);
	
	// Get calendar views
	mCalendarView = (CCalendarView*) FindPaneByID(paneid_CalendarView);
	mPreview = (CEventPreview*) FindPaneByID(paneid_CalendarPreview);
	GetCalendarView()->SetPreview(mPreview);
	GetCalendarView()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Install the splitter items
	mSplitter->InstallViews(GetCalendarView(), mPreview, false);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetCalendarViewOptions().GetShowPreview())
		ShowPreview(false);
	
	GetCalendarView()->InitView();
	GetCalendarView()->Focus();
}

// Respond to commands
Boolean CCalendarWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_SetDefaultSize:
		SaveDefaultState();
		break;

	case cmd_ResetDefaultWindow:
		ResetState(true);
		break;

	case cmd_ShowPreview:
	case cmd_ToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		break;

	default:
		cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_SetDefaultSize:
	case cmd_ResetDefaultWindow:
	case cmd_Toolbar:
		// Always enabled
		outEnabled = true;
		break;

	case cmd_ShowPreview:
	case cmd_ToolbarShowPreview:
	{
		// Always enabled but text may change
		outEnabled = true;
		LStr255 txt(STRx_Standards, !mPreviewVisible ? str_ShowPreview : str_HidePreview);
		::PLstrcpy(outName, txt);
		break;
	}

	default:
		LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
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
