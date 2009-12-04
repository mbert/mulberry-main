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


// Source for CPrefsRemoteSets class

#include "CPrefsRemoteSets.h"

#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAccount.h"
#include "CRemotePrefsSets.h"
#include "CTextTable.h"
#include "CXStringResources.h"

#include <LPushButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsRemoteSets::CPrefsRemoteSets()
{
}

// Constructor from stream
CPrefsRemoteSets::CPrefsRemoteSets(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsRemoteSets::~CPrefsRemoteSets()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsRemoteSets::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mList = (CTextTable*) FindPaneByID(paneid_PrefsRemoteSetsList);
	mList->AddListener(this);

	mRefreshBtn = (LPushButton*) FindPaneByID(paneid_PrefsRemoteSetsRefreshBtn);
	mRenameBtn = (LPushButton*) FindPaneByID(paneid_PrefsRemoteSetsRenameBtn);
	mRenameBtn->Disable();
	mDeleteBtn = (LPushButton*) FindPaneByID(paneid_PrefsRemoteSetsDeleteBtn);
	mDeleteBtn->Disable();

	// Init list
	UpdateList();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsRemoteSetsBtns);
}

// Handle buttons
void CPrefsRemoteSets::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_PrefsRemoteSetsRefresh:
			DoRefresh();
			break;

		case msg_PrefsRemoteSetsRename:
			DoRename();
			break;

		case msg_PrefsRemoteSetsDelete:
			DoDelete();
			break;

		case msg_PrefsRemoteSetsSelect:
			if (mList->IsSelectionValid())
			{
				mRenameBtn->Enable();
				mDeleteBtn->Enable();
			}
			else
			{
				mRenameBtn->Disable();
				mDeleteBtn->Disable();
			}
			break;

		default:
			break;
	}
}

// Set prefs
void CPrefsRemoteSets::SetData(void* data)
{
}

// Force update of prefs
void CPrefsRemoteSets::UpdateData(void* data)
{
	// Do nothing
}

// Force refresh of list
void CPrefsRemoteSets::DoRefresh(void)
{
	// Get owning panel
	LPane* remote = GetSuperView();
	while(remote->GetPaneID() != paneid_PrefsAccount)
		remote = remote->GetSuperView();

	// Force update of prefs to get current server IP
	((CPrefsAccount*) remote)->UpdatePrefs();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(((CPrefsAccount*) remote)->GetCopyPrefs()))
	{
		// Make sure sets are read in
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();

		UpdateList();
	}
}

// Rename Sets
void CPrefsRemoteSets::DoRename(void)
{
	// Get owning panel
	LPane* remote = GetSuperView();
	while(remote->GetPaneID() != paneid_PrefsAccount)
		remote = remote->GetSuperView();

	// Force update of prefs to get current server IP
	((CPrefsAccount*) remote)->UpdatePrefs();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(((CPrefsAccount*) remote)->GetCopyPrefs()))
	{
		STableCell aCell(0, 0);

		// Rename entire selection
		while(mList->GetNextSelectedCell(aCell))
		{
			bool duplicate = true;

			while(duplicate)
			{
				duplicate = false;

				// Get a new name for the mailbox (use old name as starter)
				cdstring old_name = CMulberryApp::sRemotePrefs->GetRemoteSets().at(aCell.row - 1);
				cdstring new_name = old_name;
				if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameSet", new_name))
				{

					// Check for duplicate
					if (std::find(CMulberryApp::sRemotePrefs->GetRemoteSets().begin(),
								CMulberryApp::sRemotePrefs->GetRemoteSets().end(), new_name) != CMulberryApp::sRemotePrefs->GetRemoteSets().end())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateRenameSet");
						duplicate = true;
						continue;
					}
					else
					{
						// Rename the mailbox
						CMulberryApp::sRemotePrefs->RenameSet(old_name, new_name);
						mList->SetCellData(aCell, new_name.c_str(), new_name.length() + 1);
						mList->RefreshRow(aCell.row);
					}
				}
			}
		}

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
}

// Delete Sets
void CPrefsRemoteSets::DoDelete(void)
{
	// Get owning panel
	LPane* remote = GetSuperView();
	while(remote->GetPaneID() != paneid_PrefsAccount)
		remote = remote->GetSuperView();

	// Force update of prefs to get current server IP
	((CPrefsAccount*) remote)->UpdatePrefs();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(((CPrefsAccount*) remote)->GetCopyPrefs()))
	{
		STableCell aCell(0, 0);

		// Delete entire selection in reverse order to tay in sync
		while(mList->GetPreviousSelectedCell(aCell))
		{
			cdstring set = CMulberryApp::sRemotePrefs->GetRemoteSets().at(aCell.row - 1);
			CMulberryApp::sRemotePrefs->DeleteSet(set);

			// Delete list item
			mList->FocusDraw();
			mList->RemoveRows(1, aCell.row, true);
		}

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
}

// Update list
void CPrefsRemoteSets::UpdateList(void)
{
	// Do nothing if no connection
	if (!CMulberryApp::sRemotePrefs)
	{
		mList->Refresh();
		return;
	}

	cdstrvect items;
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		items.push_back(*iter);

	mList->SetContents(items);
	mList->Refresh();
}
