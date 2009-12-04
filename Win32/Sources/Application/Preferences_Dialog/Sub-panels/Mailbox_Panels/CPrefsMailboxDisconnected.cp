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


// CPrefsMailboxDisconnected.cpp : implementation file
//

#include "CPrefsMailboxDisconnected.h"

#include "CPreferences.h"
#include "CPrefsMailboxOptions.h"
#include "CPrefsLocalOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxDisconnected dialog

IMPLEMENT_DYNAMIC(CPrefsMailboxDisconnected, CTabPanel)

CPrefsMailboxDisconnected::CPrefsMailboxDisconnected()
	: CTabPanel(CPrefsMailboxDisconnected::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailboxDisconnected)
	//}}AFX_DATA_INIT
}


void CPrefsMailboxDisconnected::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailboxDisconnected)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailboxDisconnected, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMailboxDisconnected)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_MAILBOX_DISCONNECTED_TABS, OnSelChangeTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxDisconnected message handlers

BOOL CPrefsMailboxDisconnected::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_PREFS_MAILBOX_DISCONNECTED_TABS, this);

	// Create tab panels
	CPrefsMailboxOptions* options = new CPrefsMailboxOptions;
	mTabs.AddPanel(options);
	CPrefsLocalOptions* local = new CPrefsLocalOptions;
	local->SetDisconnected();
	mTabs.AddPanel(local);

	// Set initial tab
	mTabs.SetPanel(0);
	return true;
}

// Set data
void CPrefsMailboxDisconnected::SetContent(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Set values
	mTabs.SetContent(copyPrefs);
}

// Force update of data
bool CPrefsMailboxDisconnected::UpdateContent(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Get values
	return mTabs.UpdateContent(copyPrefs);
}

void CPrefsMailboxDisconnected::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
