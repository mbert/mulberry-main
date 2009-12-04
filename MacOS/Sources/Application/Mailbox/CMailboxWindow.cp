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


// Source for CMailboxWindow class

#include "CMailboxWindow.h"

#include "CCommands.h"
#include "CMessageView.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CWindowsMenu.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members

cdmutexprotect<CMailboxWindow::CMailboxWindowList> CMailboxWindow::sMboxWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxWindow::CMailboxWindow()
{
	mPreviewVisible = true;

	// Add to list
	{
		cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
		sMboxWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this);
}

// Constructor from stream
CMailboxWindow::CMailboxWindow(LStream *inStream)
		: CTableViewWindow(inStream)
{
	mPreviewVisible = true;

	// Add to list
	{
		cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
		sMboxWindows->push_back(this);
	}
	CWindowsMenu::AddWindow(this);
}

// Default destructor
CMailboxWindow::~CMailboxWindow()
{
	// Remove from list
	cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
	CMailboxWindowList::iterator found = std::find(sMboxWindows->begin(), sMboxWindows->end(), this);
	if (found != sMboxWindows->end())
		sMboxWindows->erase(found);
	CWindowsMenu::RemoveWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CMailboxWindow::FinishCreateSelf(void)
{
	// Do inherited
	CTableViewWindow::FinishCreateSelf();

	// Deal with splitter
	mSplitter = (CSplitterView*) FindPaneByID(paneid_MailboxSplitter);
	
	// Get preview pane and pass to main view
	mPreview = (CMessageView*) FindPaneByID(paneid_MailboxPreview);
	GetMailboxView()->SetPreview(mPreview);
	mPreview->SetOwnerView(GetMailboxView());

	// Install the splitter items
	mSplitter->InstallViews(GetMailboxView(), mPreview, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview())
		ShowPreview(false);

	SetLatentSub(GetMailboxView()->GetTable());

	// Set status
	GetMailboxView()->SetOpen();
}

//	Respond to commands
Boolean CMailboxWindow::ObeyCommand(CommandT inCommand,void *ioParam)
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
void CMailboxWindow::FindCommandStatus(
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

// Possibly do expunge
Boolean CMailboxWindow::AttemptQuitSelf(SInt32 inSaveOption)
{
	if (GetMailboxView()->TestClose() && CTableViewWindow::AttemptQuitSelf(inSaveOption))
	{
		// Close the view
		GetMailboxView()->DoClose();

		// If quit and close allowed then delete it now
		FRAMEWORK_DELETE_WINDOW(this)
		return true;
	}
	else
		return false;
}

// Activate table
void CMailboxWindow::BeTarget(void)
{
	// Always forvce table to be active
	SwitchTarget(GetMailboxView()->GetTable());
}
