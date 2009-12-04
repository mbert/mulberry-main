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


// Source for CAddressBookWindow class

#include "CAddressBookWindow.h"

#include "CAddressBook.h"
#include "CAddressView.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CGroupTable.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"

#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S B O O K W I N D O W
// __________________________________________________________________________________________________

// Static members

cdmutexprotect<CAddressBookWindow::CAddressBookWindowList> CAddressBookWindow::sAddressBookWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressBookWindow::CAddressBookWindow()
{
	InitAddressBookWindow();
}

// Constructor from stream
CAddressBookWindow::CAddressBookWindow(LStream *inStream)
		: CTableViewWindow(inStream)
{
	InitAddressBookWindow();
}

// Default destructor
CAddressBookWindow::~CAddressBookWindow()
{
	// Remove from list
	{
		cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
		CAddressBookWindowList::iterator found = std::find(sAddressBookWindows->begin(), sAddressBookWindows->end(), this);
		if (found != sAddressBookWindows->end())
			sAddressBookWindows->erase(found);
	}

	CWindowsMenu::RemoveWindow(this);
}

// Common init
void CAddressBookWindow::InitAddressBookWindow(void)
{
	mPreviewVisible = true;

	{
		cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
		sAddressBookWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressBookWindow::FinishCreateSelf(void)
{
	// Do inherited
	CTableViewWindow::FinishCreateSelf();
	AddAttachment(new LUndoer);

	// Deal with splitter
	mSplitter = (CSplitterView*) FindPaneByID(paneid_AddressBookSplitter);
	
	// Get preview pane and pass to main view
	CAddressView* preview = (CAddressView*) FindPaneByID(paneid_AddressBookPreview);
	GetAddressBookView()->SetPreview(preview);

	// Install the splitter items
	mSplitter->InstallViews(GetAddressBookView(), preview, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview())
		ShowPreview(false);

	SetLatentSub(GetAddressBookView()->GetAddressTable());

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_AddressBookToolbarView);
	mToolbarView->SetSibling(mSplitter);

	// Create toolbars for a view we own
	GetAddressBookView()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetAddressBookView()->SetOpen();

	CContextMenuAttachment::AddUniqueContext(this, 9003, this);
}

bool CAddressBookWindow::OpenWindow(CAddressBook* adbk)
{
	// Does window already exist?
	CAddressBookWindow* theWindow = CAddressBookWindow::FindWindow(adbk);

	if (theWindow != NULL)
	{
		// Found existing window so make visible and select
		FRAMEWORK_WINDOW_TO_TOP(theWindow)
		
		return true;
	}
	else
	{
		try
		{
			// Create window for our document
			theWindow = (CAddressBookWindow*) CAddressBookWindow::CreateWindow(paneid_AddressBookWindow, CMulberryApp::sApp);
			MyCFString name(adbk->GetAccountName(), kCFStringEncodingUTF8);
			theWindow->SetCFDescriptor(name);
			theWindow->SetAddressBook(adbk);
		
			// Now reset the address book display
			theWindow->ResetAddressBook();
			theWindow->Show();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		
		return theWindow != NULL;
	}
}

CAddressBookWindow* CAddressBookWindow::FindWindow(const CAddressBook* adbk)
{
	cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
	for(CAddressBookWindowList::const_iterator iter = sAddressBookWindows->begin(); iter != sAddressBookWindows->end(); iter++)
	{
		if (adbk == (*iter)->GetAddressBook())
			return *iter;
	}

	return NULL;
}

// Check for window
bool CAddressBookWindow::WindowExists(const CAddressBookWindow* wnd)
{
	cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
	CAddressBookWindowList::iterator found = std::find(sAddressBookWindows->begin(), sAddressBookWindows->end(), wnd);
	return found != sAddressBookWindows->end();
}

//	Set dirty state
void CAddressBookWindow::SetDirty(bool dirty)
{
	GetAddressBookView()->GetAddressTable()->SetDirty(dirty);
	GetAddressBookView()->GetGroupTable()->SetDirty(dirty);
}

// Get dirty state
bool CAddressBookWindow::IsDirty(void)
{
	return GetAddressBookView()->GetAddressTable()->IsDirty() ||
			GetAddressBookView()->GetGroupTable()->IsDirty();
}

//	Respond to commands
Boolean CAddressBookWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ShowPreview:
	case cmd_ToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		break;

	default:
		cmdHandled = CTableViewWindow::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAddressBookWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
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
		CTableViewWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

CAddressBook* CAddressBookWindow::GetAddressBook()
{
	return GetAddressBookView()->GetAddressBook();
}

// Set address and group lists
void CAddressBookWindow::SetAddressBook(CAddressBook* adbk)
{
	GetAddressBookView()->SetAddressBook(adbk);
}

// Refresh address and group lists
void CAddressBookWindow::ResetAddressBook()
{
	GetAddressBookView()->ResetAddressBook();
}

void CAddressBookWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change its state based on geometry
	mSplitter->ShowView(true, show);
	GetAddressBookView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetAddressViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}
