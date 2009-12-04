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


// CPrefsAttachments.cpp : implementation file
//


#include "CPrefsAttachments.h"

#include "CPreferences.h"
#include "CPrefsAttachmentsSend.h"
#include "CPrefsAttachmentsReceive.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachments property page

IMPLEMENT_DYNCREATE(CPrefsAttachments, CPrefsPanel)

CPrefsAttachments::CPrefsAttachments() : CPrefsPanel(CPrefsAttachments::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAttachments)
	//}}AFX_DATA_INIT
}

CPrefsAttachments::~CPrefsAttachments()
{
}

void CPrefsAttachments::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAttachments)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAttachments, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsAttachments)
	ON_NOTIFY(TCN_SELCHANGE, IDC_ATTACHMENT_TABS, OnSelChangeAttachmentTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsAttachments::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
}

// Get params from DDX
void CPrefsAttachments::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachments message handlers

void CPrefsAttachments::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_ATTACHMENT_TABS, this);

	// Create tab panels
	CPrefsAttachmentsSend* attachment_send = new CPrefsAttachmentsSend;
	mTabs.AddPanel(attachment_send);

	CPrefsAttachmentsReceive* attachment_receive = new CPrefsAttachmentsReceive;
	mTabs.AddPanel(attachment_receive);
}

void CPrefsAttachments::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsAttachments::OnSelChangeAttachmentTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
