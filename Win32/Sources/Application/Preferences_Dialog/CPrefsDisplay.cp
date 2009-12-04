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


// CPrefsDisplay.cp : implementation file
//


#include "CPrefsDisplay.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsDisplayServer.h"
#include "CPrefsDisplayMailbox.h"
#include "CPrefsDisplayMessage.h"
#include "CPrefsDisplayLabel.h"
#include "CPrefsDisplayQuotes.h"
#include "CPrefsDisplayStyles.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay property page

IMPLEMENT_DYNCREATE(CPrefsDisplay, CPrefsPanel)

CPrefsDisplay::CPrefsDisplay() : CPrefsPanel(CPrefsDisplay::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplay)
	//}}AFX_DATA_INIT
}

CPrefsDisplay::~CPrefsDisplay()
{
}

void CPrefsDisplay::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplay)
	mTabs.DoDataExchange(pDX);

	DDX_Check(pDX, IDC_DISPLAY_MULTI_ALLOWSHORTCUTS, mAllowKeyboardShortcuts);
	DDX_Check(pDX, IDC_DISPLAY_MULTI_TASKBAR, mMultiTaskbar);
	DDX_Control(pDX, IDC_DISPLAY_MULTI_TASKBAR, mMultiTaskbarCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplay, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsDisplay)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DISPLAY_TABS, OnSelChangeDisplayTabs)
	ON_BN_CLICKED(IDC_DISPLAY_RESET, OnDisplayReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsDisplay::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
	
	mAllowKeyboardShortcuts = mCopyPrefs->mAllowKeyboardShortcuts.GetValue();

	mMultiTaskbar = mCopyPrefs->mMultiTaskbar.GetValue();
}

// Get params from DDX
void CPrefsDisplay::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
	
	prefs->mAllowKeyboardShortcuts.SetValue(mAllowKeyboardShortcuts);
	
	prefs->mMultiTaskbar.SetValue(mMultiTaskbar);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay message handlers

void CPrefsDisplay::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_DISPLAY_TABS, this);

	// Create tab panels
	CPrefsDisplayServer* display_server = new CPrefsDisplayServer;
	mTabs.AddPanel(display_server);

	CPrefsDisplayMailbox* display_mailbox = new CPrefsDisplayMailbox;
	mTabs.AddPanel(display_mailbox);

	CPrefsDisplayMessage* display_message = new CPrefsDisplayMessage;
	mTabs.AddPanel(display_message);

	CPrefsDisplayLabel* display_label = new CPrefsDisplayLabel;
	mTabs.AddPanel(display_label);

	CPrefsDisplayQuotes* display_quotes = new CPrefsDisplayQuotes;
	mTabs.AddPanel(display_quotes);

	CPrefsDisplayStyles* display_styles = new CPrefsDisplayStyles;
	mTabs.AddPanel(display_styles);
}

void CPrefsDisplay::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsDisplay::OnSelChangeDisplayTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}

void CPrefsDisplay::OnDisplayReset()
{
	mCopyPrefs->ResetAllStates(true);
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetParentOwner();
	prefs_dlog->SetForceWindowReset(true);
}
