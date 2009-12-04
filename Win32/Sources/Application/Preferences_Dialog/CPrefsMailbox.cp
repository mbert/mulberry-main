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


// CPrefsMailbox.cpp : implementation file
//


#include "CPrefsMailbox.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsMailboxBasic.h"
#include "CPrefsMailboxAdvanced.h"
#include "CPrefsMailboxDisconnected.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailbox property page

IMPLEMENT_DYNCREATE(CPrefsMailbox, CPrefsPanel)

CPrefsMailbox::CPrefsMailbox() : CPrefsPanel(CPrefsMailbox::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailbox)
	//}}AFX_DATA_INIT
}

CPrefsMailbox::~CPrefsMailbox()
{
}

void CPrefsMailbox::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailbox)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailbox, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsMailbox)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MAILBOX_TABS, OnSelChangeMailboxTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsMailbox::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
}

// Get params from DDX
void CPrefsMailbox::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailbox message handlers

void CPrefsMailbox::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_MAILBOX_TABS, this);

	// Create tab panels
	CPrefsMailboxBasic* mailbox_basic = new CPrefsMailboxBasic;
	mTabs.AddPanel(mailbox_basic);

	CPrefsMailboxAdvanced* mailbox_advanced = new CPrefsMailboxAdvanced;
	mTabs.AddPanel(mailbox_advanced);

	CPrefsMailboxDisconnected* mailbox_disconnected = new CPrefsMailboxDisconnected;
	mTabs.AddPanel(mailbox_disconnected);
}

void CPrefsMailbox::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsMailbox::OnSelChangeMailboxTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
