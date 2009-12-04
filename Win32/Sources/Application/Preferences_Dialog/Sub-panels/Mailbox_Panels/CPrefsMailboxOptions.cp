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


// CPrefsMailboxOptions.cpp : implementation file
//

#include "CPrefsMailboxOptions.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxOptions dialog

IMPLEMENT_DYNAMIC(CPrefsMailboxOptions, CTabPanel)

CPrefsMailboxOptions::CPrefsMailboxOptions()
	: CTabPanel(CPrefsMailboxOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailboxOptions)
	//}}AFX_DATA_INIT
}


void CPrefsMailboxOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailboxOptions)
	DDX_Check(pDX, IDC_PROMPTSTARTUPDISCONNECT, mPromptStartup);
	DDX_Check(pDX, IDC_AUTODIAL, mAutoDial);
	DDX_Check(pDX, IDC_UIDVALIDITYMISMATCH, mUIDValidityMismatch);
	DDX_Check(pDX, IDC_USE_OS_LOCATIONS, mOSDefaultLocation);
	DDX_Check(pDX, IDC_SLEEP_RECONNECT, mSleepReconnect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailboxOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMailboxOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxOptions message handlers

// Set data
void CPrefsMailboxOptions::SetContent(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Set values
	mPromptStartup = copyPrefs->mPromptDisconnected.GetValue();
	mAutoDial = copyPrefs->mAutoDial.GetValue();
	mUIDValidityMismatch = copyPrefs->mUIDValidityMismatch.GetValue();
	mOSDefaultLocation = copyPrefs->mOSDefaultLocation.GetValue();
	mSleepReconnect = copyPrefs->mSleepReconnect.GetValue();
}

// Force update of data
bool CPrefsMailboxOptions::UpdateContent(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Get values
	copyPrefs->mPromptDisconnected.SetValue(mPromptStartup);
	copyPrefs->mAutoDial.SetValue(mAutoDial);
	copyPrefs->mUIDValidityMismatch.SetValue(mUIDValidityMismatch);
	copyPrefs->mOSDefaultLocation.SetValue(mOSDefaultLocation);
	copyPrefs->mSleepReconnect.SetValue(mSleepReconnect);
	
	return true;
}
