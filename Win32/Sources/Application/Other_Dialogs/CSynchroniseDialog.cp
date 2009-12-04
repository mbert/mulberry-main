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


// CSynchroniseDialog.cpp : implementation file
//

#include "CSynchroniseDialog.h"

#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSynchroniseDialog dialog


CSynchroniseDialog::CSynchroniseDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSynchroniseDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSynchroniseDialog)
	mAll = -1;
	mFull = -1;
	mSize = 10;
	mHasSelection = false;
	//}}AFX_DATA_INIT
}


void CSynchroniseDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSynchroniseDialog)
	DDX_Radio(pDX, IDC_SYNCHRONISE_ALL, mAll);
	DDX_Radio(pDX, IDC_SYNCHRONISE_FULL, mFull);
	DDX_UTF8Text(pDX, IDC_SYNCHRONISE_SIZE, mSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSynchroniseDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSynchroniseDialog)
	ON_BN_CLICKED(IDC_SYNCHRONISE_FULL, OnSynchroniseFull)
	ON_BN_CLICKED(IDC_SYNCHRONISE_BELOW, OnSynchroniseBelow)
	ON_BN_CLICKED(IDC_SYNCHRONISE_PARTIAL, OnSynchronisePartial)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSynchroniseDialog message handlers

BOOL CSynchroniseDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Disable cache all if its not set
	if (mFull != 1)
		GetDlgItem(IDC_SYNCHRONISE_SIZE)->EnableWindow(false);
	if (!mHasSelection)
		GetDlgItem(IDC_SYNCHRONISE_SELECTED)->EnableWindow(false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSynchroniseDialog::OnSynchroniseFull() 
{
	GetDlgItem(IDC_SYNCHRONISE_SIZE)->EnableWindow(false);
}

void CSynchroniseDialog::OnSynchroniseBelow() 
{
	GetDlgItem(IDC_SYNCHRONISE_SIZE)->EnableWindow(true);
}

void CSynchroniseDialog::OnSynchronisePartial() 
{
	GetDlgItem(IDC_SYNCHRONISE_SIZE)->EnableWindow(false);
}

// Set the details
void CSynchroniseDialog::SetDetails(bool has_selection)
{
	// Get disconnection options from prefs
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	mHasSelection = has_selection;
	if (!has_selection)
		mAll = (disconnect.mMboxSync != CConnectionManager::eNewMessages) ? 0 : 1;
	else
		mAll = 2;

	mFull = disconnect.mMsgSync;
	mSize = disconnect.mMsgSyncSize/1024;
}

// Get the details
void CSynchroniseDialog::GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection)
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	fast = (mAll == 1);
	partial = (mFull == 2);
	selection = (mAll == 2);

	// Sync if requested
	if (mAll != 2)
		disconnect.mMboxSync = static_cast<CConnectionManager::EMboxSync>(mAll);

	if (mFull == 1)
		size = mSize * 1024;
	else
		size = 0;

	// Message options
	disconnect.mMsgSync = static_cast<CConnectionManager::EMessageSync>(mFull);
	disconnect.mMsgSyncSize = mSize * 1024;
	
	// Mark prefs as dirty if needed
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}
void CSynchroniseDialog::PoseDialog(CMboxList* mbox_list)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = false;
	
	if (PoseDialog(fast, partial, size, selected))
	{
		// Synchonise each mailbox
		for(CMboxList::iterator iter = mbox_list->begin(); iter != mbox_list->end(); iter++)
			static_cast<CMbox*>(*iter)->Synchronise(fast, partial, size);
	}
}

void CSynchroniseDialog::PoseDialog(CMbox* mbox, ulvector& selection)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = selection.size();
	
	if (PoseDialog(fast, partial, size, selected))
	{
		if (selected)
			// Synchronise the messages
			mbox->SynchroniseMessage(selection, false, fast, partial, size, true);
		else
			// Full synchonise of mailbox
			mbox->Synchronise(fast, partial, size);
	}
}

bool CSynchroniseDialog::PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected)
{
	// Create the dialog
	CSynchroniseDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(selected);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{					
		dlog.GetDetails(fast, partial, size, selected);
		return true;
	}
	else
		return false;
}