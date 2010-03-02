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


// CPrefsRemoteSets.cpp : implementation file
//

#include "CPrefsRemoteSets.h"

#include "CErrorHandler.h"
#include "CINETAccount.h"
#include "CGetStringDialog.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPreferencesDialog.h"
#include "CRemotePrefsSets.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteSets dialog

IMPLEMENT_DYNAMIC(CPrefsRemoteSets, CTabPanel)

CPrefsRemoteSets::CPrefsRemoteSets()
	: CTabPanel(CPrefsRemoteSets::IDD)
{
	//{{AFX_DATA_INIT(CPrefsRemoteSets)
	//}}AFX_DATA_INIT
}


void CPrefsRemoteSets::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsRemoteSets)
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Remote_SetList, mList);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Remote_RefreshSets, mRefreshBtn);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Remote_Renameset, mRenameBtn);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Remote_DeleteSet, mDeleteBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsRemoteSets, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsRemoteSets)
	ON_LBN_SELCHANGE(IDC_PREFS_ACCOUNT_Remote_SetList, OnSelchangeRemoteSets)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Remote_RefreshSets, OnRefreshRemote)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Remote_Renameset, OnRenameRemote)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Remote_DeleteSet, OnDeleteRemote)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteSets message handlers

// Set data
void CPrefsRemoteSets::SetContent(void* data)
{
}

// Force update of data
bool CPrefsRemoteSets::UpdateContent(void* data)
{
	return true;
}

BOOL CPrefsRemoteSets::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Init list
	UpdateList();
	
	return TRUE;
}

void CPrefsRemoteSets::OnSelchangeRemoteSets() 
{
	mRenameBtn.EnableWindow(mList.GetSelCount());
	mDeleteBtn.EnableWindow(mList.GetSelCount());
}

void CPrefsRemoteSets::OnRefreshRemote() 
{
	// Force update of prefs
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	dlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(dlog->GetCopyPrefs()))
	{
		// Make sure sets are read in
		CMulberryApp::sRemotePrefs->ListRemoteSets();
		
		UpdateList();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
}

void CPrefsRemoteSets::OnRenameRemote() 
{
	// Force update of prefs
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	dlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(dlog->GetCopyPrefs()))
	{
		// Get selected items
		int cnt = mList.GetSelCount();
		int* selected = new int[cnt];
		mList.GetSelItems(cnt, selected);

		// Rename entire selection
		for(int i = cnt - 1; i >= 0; i--)
		{
			bool duplicate = true;

			while(duplicate)
			{
				duplicate = false;

				// Get a new name for the mailbox (use old name as starter)
				cdstring old_name = CMulberryApp::sRemotePrefs->GetRemoteSets().at(selected[i]);
				cdstring new_name = old_name;
				if (CGetStringDialog::PoseDialog(IDS_SAVESETASDIALOG, IDS_SAVESETASTITLE, new_name))
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

						// Replace item
						mList.DeleteString(selected[i]);
						mList.InsertString(selected[i], new_name.win_str());
					}
				}
			}
		}

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
}

void CPrefsRemoteSets::OnDeleteRemote() 
{
	// Force update of prefs
	CPreferencesDialog* dlog = (CPreferencesDialog*) GetParentOwner();
	dlog->ForceUpdate();

	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(dlog->GetCopyPrefs()))
	{
		// Get selected items
		int cnt = mList.GetSelCount();
		int* selected = new int[cnt];
		mList.GetSelItems(cnt, selected);

		// Delete entire selection in reverse order to stay in sync
		for(int i = cnt - 1; i >= 0; i--)
		{
			cdstring set = CMulberryApp::sRemotePrefs->GetRemoteSets().at(selected[i]);
			CMulberryApp::sRemotePrefs->DeleteSet(set);

			mList.DeleteString(selected[i]);
		}
		delete[] selected;

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
}

// Update list
void CPrefsRemoteSets::UpdateList(void)
{
	// Delete existing
	mList.ResetContent();

	// Do nothing if no connection
	if (!CMulberryApp::sRemotePrefs)
		return;

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		mList.AddString((*iter).win_str());
}
