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


// CPrefsMessage.cpp : implementation file
//


#include "CPrefsMessage.h"

#include "CPreferences.h"
#include "CPrefsMessageGeneral.h"
#include "CPrefsMessageOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessage property page

IMPLEMENT_DYNCREATE(CPrefsMessage, CPrefsPanel)

CPrefsMessage::CPrefsMessage() : CPrefsPanel(CPrefsMessage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMessage)
	//}}AFX_DATA_INIT
}

CPrefsMessage::~CPrefsMessage()
{
}

void CPrefsMessage::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMessage)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMessage, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsMessage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MESSAGE_TABS, OnSelChangeMessageTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsMessage::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
}

// Get params from DDX
void CPrefsMessage::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessage message handlers

void CPrefsMessage::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_MESSAGE_TABS, this);

	// Create tab panels
	CPrefsMessageGeneral* msg_general = new CPrefsMessageGeneral;
	mTabs.AddPanel(msg_general);

	CPrefsMessageOptions* msg_options = new CPrefsMessageOptions;
	mTabs.AddPanel(msg_options);
}

void CPrefsMessage::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsMessage::OnSelChangeMessageTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
