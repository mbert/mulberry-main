/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CEditIdentityOutgoing.cpp : implementation file
//

#include "CEditIdentityOutgoing.h"

#include "CEditIdentityAddress.h"
#include "CEditIdentityDSN.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOutgoing dialog

IMPLEMENT_DYNAMIC(CEditIdentityOutgoing, CTabPanel)


CEditIdentityOutgoing::CEditIdentityOutgoing()
	: CTabPanel(CEditIdentityOutgoing::IDD)
{
	//{{AFX_DATA_INIT(CEditIdentityOutgoing)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditIdentityOutgoing::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentityOutgoing)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditIdentityOutgoing, CTabPanel)
	//{{AFX_MSG_MAP(CEditIdentityOutgoing)
	ON_NOTIFY(TCN_SELCHANGE, IDC_IDENTITY_OUTGOING_TABS, OnSelChangeTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOutgoing message handlers

// Set data
BOOL CEditIdentityOutgoing::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	// Subclass buttons
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_IDENTITY_OUTGOING_TABS, this);

	// Create tab panels
	CEditIdentityAddress* panel_to = new CEditIdentityAddress(true);
	panel_to->SetAddressType(false, false, false, true, false, false, false);
	mTabs.AddPanel(panel_to);
	cdstring title;
	title.FromResource(IDS_IDENTITY_PANEL_TO);
	CUnicodeUtils::SetWindowTextUTF8(panel_to, title);
	mTabs.SetPanelTitle(0, title);

	CEditIdentityAddress* panel_cc = new CEditIdentityAddress(true);
	panel_cc->SetAddressType(false, false, false, false, true, false, false);
	mTabs.AddPanel(panel_cc);
	title.FromResource(IDS_IDENTITY_PANEL_CC);
	CUnicodeUtils::SetWindowTextUTF8(panel_to, title);
	mTabs.SetPanelTitle(1, title);

	CEditIdentityAddress* panel_bcc = new CEditIdentityAddress(true);
	panel_bcc->SetAddressType(false, false, false, false, false, true, false);
	mTabs.AddPanel(panel_bcc);
	title.FromResource(IDS_IDENTITY_PANEL_BCC);
	CUnicodeUtils::SetWindowTextUTF8(panel_to, title);
	mTabs.SetPanelTitle(2, title);

	CEditIdentityDSN* panel_dsn = new CEditIdentityDSN;
	mTabs.AddPanel(panel_dsn);

	return TRUE;
}

// Set data
void CEditIdentityOutgoing::SetContent(void* data)
{
	mTabs.SetContent(data);
	mTabs.SetPanel(0);
}

// Force update of data
bool CEditIdentityOutgoing::UpdateContent(void* data)
{
	// Get values
	return mTabs.UpdateContent(data);
}

void CEditIdentityOutgoing::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
