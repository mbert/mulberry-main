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


// CPrefsAlerts.cpp : implementation file
//


#include "CPrefsAlerts.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAlertsMessage.h"
#include "CPrefsAlertsAttachment.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlerts property page

IMPLEMENT_DYNCREATE(CPrefsAlerts, CPrefsPanel)

CPrefsAlerts::CPrefsAlerts() : CPrefsPanel(CPrefsAlerts::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAlerts)
	//}}AFX_DATA_INIT
}

CPrefsAlerts::~CPrefsAlerts()
{
}

void CPrefsAlerts::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAlerts)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAlerts, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsAlerts)
	ON_NOTIFY(TCN_SELCHANGE, IDC_ALERTS_TABS, OnSelChangeMailboxTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsAlerts::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
}

// Get params from DDX
void CPrefsAlerts::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlerts message handlers

void CPrefsAlerts::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_ALERTS_TABS, this);

	// Create tab panels
	CPrefsAlertsMessage* alerts_message = new CPrefsAlertsMessage;
	mTabs.AddPanel(alerts_message);

	CPrefsAlertsAttachment* alerts_attachment = new CPrefsAlertsAttachment;
	mTabs.AddPanel(alerts_attachment);
}

void CPrefsAlerts::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsAlerts::OnSelChangeMailboxTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
