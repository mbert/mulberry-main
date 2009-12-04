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
#include "CMailboxTitleTable.h"
#include "CMailboxTable.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CSplitterView.h"
#include "CWindowsMenu.h"

#include <JXDisplay.h>

#include <string.h>

#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members

cdmutexprotect<CMailboxWindow::CMailboxWindowList> CMailboxWindow::sMboxWindows;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxWindow::CMailboxWindow(JXDirector* owner)
  : CTableViewWindow(owner)
{
	mSplitter = NULL;
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
	{
		cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
		CMailboxWindowList::iterator found = std::find(sMboxWindows->begin(), sMboxWindows->end(), this);
		if (found != sMboxWindows->end())
			sMboxWindows->erase(found);
	}
	CWindowsMenu::RemoveWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

//	Respond to commands
bool CMailboxWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eWindowsShowPreview:
	case CCommand::eToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		return true;

	default:;
	};

	return CTableViewWindow::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CMailboxWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
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

	CTableViewWindow::UpdateCommand(cmd, cmdui);
}

void CMailboxWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	pCmdUI->SetText(txt);
}

void CMailboxWindow::OnUpdateShowPreviewBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

