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


// Source for CBrowseMailboxDialog class

#include "CBrowseMailboxDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerBrowseTable.h"

#include <LCheckBox.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CBrowseMailboxDialog::CBrowseMailboxDialog()
{
}

// Constructor from stream
CBrowseMailboxDialog::CBrowseMailboxDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CBrowseMailboxDialog::~CBrowseMailboxDialog()
{
	// Make sure window position is saved
	SaveState();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CBrowseMailboxDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mTable = (CServerBrowseTable*) FindPaneByID(paneid_BrTable);
	mTable->AddListener((CBalloonDialog*) GetSuperCommander());
	mTable->SetManager();

	// Make text edit field active
	SetLatentSub(mTable);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, mPaneID);

	// Reset window state
	ResetState();
}

// Set open mode
void CBrowseMailboxDialog::SetOpenMode(bool mode)
{
	mTable->SetOpenMode(mode);
	mTable->ResetTable();
}

// Set open mode
void CBrowseMailboxDialog::SetSending(bool sending)
{
	// Hide set as default button and increase table size
	if (!sending)
	{
		LCheckBox* cbox = (LCheckBox*) FindPaneByID(paneid_BrSaveDefaultBtn);
		if (cbox)
		{
			cbox->Hide();
			LView* move = (LView*) FindPaneByID(paneid_BrMovePane);
			if (move)
				move->ResizeFrameBy(0, 30, true);
		}
	}
}

// Set open mode
void CBrowseMailboxDialog::SetMultipleSelection()
{
	mTable->SetMultipleSelection();
}

// Get selected mbox
CMbox* CBrowseMailboxDialog::GetSelectedMbox(void)
{
	return mTable->GetSelectedMbox();
}

// Get selected mboxes
void CBrowseMailboxDialog::GetSelectedMboxes(CMboxList& mbox_list)
{
	mTable->GetSelectedMboxes(mbox_list);
}

// Get set as default checkbox state
bool CBrowseMailboxDialog::GetSetAsDefault()
{
	LCheckBox* cbox = (LCheckBox*) FindPaneByID(paneid_BrSaveDefaultBtn);
	if (cbox)
		return cbox->GetValue() == 1;
	else
		return false;
}

// Reset state from prefs
void CBrowseMailboxDialog::ResetState(void)
{
	CWindowState& state = CPreferences::sPrefs->mServerBrowseDefault.Value();

	// Do not set if empty
	Rect set_rect = state.GetBestRect(state);
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}
}

// Save state in prefs
void CBrowseMailboxDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;

	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CWindowState state(name, &mUserBounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mServerBrowseDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerBrowseDefault.SetDirty();
}

bool CBrowseMailboxDialog::PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default)
{
	bool result = false;
	{
		
		// Create the dialog
		CBalloonDialog	dlog(sending ? paneid_BrowseCopyMessageDialog : paneid_BrowseMailboxDialog, CMulberryApp::sApp);
		((CBrowseMailboxDialog*) dlog.GetDialog())->SetOpenMode(open_mode);
		((CBrowseMailboxDialog*) dlog.GetDialog())->SetSending(sending);

		// Run modal loop waiting for Copy, No Copy or Cancel message
		dlog.StartDialog();
		while (true)
		{
			MessageT hitMessage = dlog.DoDialog();

			if (hitMessage == msg_OK)
			{
				mbox = ((CBrowseMailboxDialog*) dlog.GetDialog())->GetSelectedMbox();
				set_as_default = ((CBrowseMailboxDialog*) dlog.GetDialog())->GetSetAsDefault();
				result = true;
				break;
			}
			else if (hitMessage == msg_BrNoCopy)
			{
				mbox = (CMbox*) -1;
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				mbox = NULL;
				break;
			}
		}
	}

	return result;
}

bool CBrowseMailboxDialog::PoseDialog(CMboxList& mbox_list)
{
	bool result = false;
	{
		
		// Create the dialog
		CBalloonDialog	theHandler(paneid_BrowseMailboxDialog, CMulberryApp::sApp);
		((CBrowseMailboxDialog*) theHandler.GetDialog())->SetMultipleSelection();	// Must do this before we start adding rows
		((CBrowseMailboxDialog*) theHandler.GetDialog())->SetOpenMode(false);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				((CBrowseMailboxDialog*) theHandler.GetDialog())->GetSelectedMboxes(mbox_list);
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
