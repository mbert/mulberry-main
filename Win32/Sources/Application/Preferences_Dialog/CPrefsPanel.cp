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


// CPrefsPanel.cpp : implementation file
//


#include "CPrefsPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPanel property page

IMPLEMENT_DYNCREATE(CPrefsPanel, CHelpPropertyPage)

CPrefsPanel::CPrefsPanel()
{
	mInited = false;
	mCopyPrefs = nil;
}

CPrefsPanel::CPrefsPanel(UINT nID) : CHelpPropertyPage(nID)
{
	mInited = false;
	mCopyPrefs = nil;
}

CPrefsPanel::~CPrefsPanel()
{
}

BEGIN_MESSAGE_MAP(CPrefsPanel, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPrefsPanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Reset params for DDX
void CPrefsPanel::ResetPrefs(CPreferences* prefs)
{
	// Set DDX value
	SetPrefs(prefs);

	if (mInited)
	{
		// Force DDX update
		UpdateData(false);

		// Update Controls
		SetControls();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsPanel message handlers

BOOL CPrefsPanel::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// set initial control states
	InitControls();
	SetControls();
	mInited = true;

	return true;
}
