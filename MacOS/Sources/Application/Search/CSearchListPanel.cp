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


// Source for CSearchListPanel class

#include "CSearchListPanel.h"

#include "CBalloonDialog.h"
#include "CIconTextTable.h"
#include "CResources.h"
#include "CSearchEngine.h"
#include "CSearchOptionsDialog.h"
#include "CSearchWindow.h"

#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSearchListPanel::CSearchListPanel()
{
}

// Default destructor
CSearchListPanel::~CSearchListPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CSearchListPanel::FinishCreateSelf(LWindow* parent, LPane* container)
{
	CMailboxListPanel::FinishCreateSelf(parent, container);

	// Get all controls
	mOptionsBtn = (LPushButton*) parent->FindPaneByID(paneid_SearchOptions);
	
	// Turn on select with tab in mailbox list
	mMailboxList->SetTabSelection(true, icnx_SearchHit);

} // CMailboxListPanel::FinishCreateSelf

void CSearchListPanel::SetInProgress(bool in_progress)
{
	CMailboxListPanel::SetInProgress(in_progress);

	if (mInProgress)
		mOptionsBtn->Disable();
	else
		mOptionsBtn->Enable();
}

#pragma mark ____________________________Mailbox List

void CSearchListPanel::OnClearMailboxList()
{
	// Delete everything in list and reset table
	CSearchEngine::sSearchEngine.ClearTargets();
	ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
										CSearchEngine::sSearchEngine.GetTargetHits());
}

// Add a list of mailboxes
void CSearchListPanel::AddMboxList(const CMboxList* list)
{
	CSearchWindow::sSearchWindow->AddMboxList(list);
}

void CSearchListPanel::OnOpenMailboxList()
{
	// Do not allow open while in progress (at least for now)
	if (mInProgress)
	{
		::SysBeep(1);
		return;
	}

	ulvector selection;

	mMailboxList->GetSelection(selection);

	// Open each mailbox
	for(ulvector::reverse_iterator riter = selection.rbegin(); riter != selection.rend(); riter++)
		// Get search engine to open it
		CSearchEngine::sSearchEngine.OpenTarget(*riter);
}

void CSearchListPanel::OnOptions()
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_SearchOptionsDialog, LCommander::GetDefaultCommander());
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Update prefs if OK'd
			((CSearchOptionsDialog*) theHandler.GetDialog())->GetItems();
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
}

