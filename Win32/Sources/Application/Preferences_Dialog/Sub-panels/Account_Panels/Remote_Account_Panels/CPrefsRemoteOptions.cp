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


// CPrefsRemoteOptions.cpp : implementation file
//

#include "CPrefsRemoteOptions.h"

#include "CINETAccount.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteOptions dialog

IMPLEMENT_DYNAMIC(CPrefsRemoteOptions, CTabPanel)

CPrefsRemoteOptions::CPrefsRemoteOptions()
	: CTabPanel(CPrefsRemoteOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsRemoteOptions)
	mUseRemote = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsRemoteOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsRemoteOptions)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_Remote_UseRemote, mUseRemote);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsRemoteOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsRemoteOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteOptions message handlers

// Set data
void CPrefsRemoteOptions::SetContent(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Copy info
	mUseRemote = account->GetLogonAtStart();
}

// Force update of data
bool CPrefsRemoteOptions::UpdateContent(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mUseRemote);
	
	return true;
}
