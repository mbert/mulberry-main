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
#include "CRemotePrefsSets.h"
#include "CTableScrollbarSet.h"
#include "CTextTable.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>

#include <algorithm>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsRemoteSets::OnCreate()
{
// begin JXLayout1

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 340,30);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Remote Preference Sets", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,5, 145,20);
    assert( obj2 != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 340,130);
    assert( sbs != NULL );

    mRefreshBtn =
        new JXTextButton("Refresh", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,175, 70,20);
    assert( mRefreshBtn != NULL );
    mRefreshBtn->SetFontSize(10);

    mRenameBtn =
        new JXTextButton("Rename...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,175, 70,20);
    assert( mRenameBtn != NULL );
    mRenameBtn->SetFontSize(10);

    mDeleteBtn =
        new JXTextButton("Delete", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,175, 70,20);
    assert( mDeleteBtn != NULL );
    mDeleteBtn->SetFontSize(10);

// end JXLayout1

	sbs->NoTitles();
	mList = new CTextTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,0, 10, 10);
	mList->OnCreate();
	mList->SetSelectionMsg(true);

	// Start listening
	ListenTo(mList);
	ListenTo(mRefreshBtn);
	ListenTo(mRenameBtn);
	ListenTo(mDeleteBtn);
}

// Handle buttons
void CPrefsRemoteSets::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mRefreshBtn)
		{
			DoRefresh();
			return;
		}
		else if (sender == mRenameBtn)
		{
			DoRename();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			DoDelete();
			return;
		}
	}
	else if (sender == mList)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			if (mList->IsSelectionValid())
			{
				mRenameBtn->Activate();
				mDeleteBtn->Activate();
			}
			else
			{
				mRenameBtn->Deactivate();
				mDeleteBtn->Deactivate();
			}
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsRemoteSets::SetData(void* data)
{
}

// Force update of prefs
bool CPrefsRemoteSets::UpdateData(void* data)
{
	// Do nothing
	return true;
}

// Force refresh of list
void CPrefsRemoteSets::DoRefresh(void)
{
	// Force update of prefs to get current server IP
	CPreferencesDialog::sPrefsDlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(CPreferencesDialog::sPrefsDlog->GetCopyPrefs()))
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
	// Force update of prefs to get current server IP
	CPreferencesDialog::sPrefsDlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(CPreferencesDialog::sPrefsDlog->GetCopyPrefs()))
	{
		// Rename entire selection
		STableCell aCell(0, 0);
		while(mList->GetNextSelectedCell(aCell))
		{
			bool duplicate = true;

			while(duplicate)
			{
				duplicate = false;

				// Get a new name for the mailbox (use old name as starter)
				cdstring old_name = CMulberryApp::sRemotePrefs->GetRemoteSets().at(aCell.row - 1);
				cdstring new_name = old_name;
				if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameSet", "Alerts::Preferences::RenameSet", new_name))
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
						mList->SetItemText(aCell.row, new_name);
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
	// Force update of prefs to get current server IP
	CPreferencesDialog::sPrefsDlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(CPreferencesDialog::sPrefsDlog->GetCopyPrefs()))
	{
		// Delete entire selection in reverse order to tay in sync
		ulvector selected;
		mList->GetSelectedRows(selected);
		for(ulvector::reverse_iterator riter = selected.rbegin(); riter != selected.rend(); riter++)
		{
			cdstring set = CMulberryApp::sRemotePrefs->GetRemoteSets().at(*riter - 1);
			CMulberryApp::sRemotePrefs->DeleteSet(set);

			// Delete list item
			mList->RemoveRows(1, *riter, false);
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
