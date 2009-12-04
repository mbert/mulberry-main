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


// CPrefsAccountPanel.cpp : implementation file
//

#include "CPrefsAccountPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountPanel dialog


CPrefsAccountPanel::CPrefsAccountPanel(int id, int tab_id)
	: CTabPanel(id)
{
	//{{AFX_DATA_INIT(CPrefsAccountPanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	mTabID = tab_id;
}


void CPrefsAccountPanel::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAccountPanel)
	//}}AFX_DATA_MAP
	
	// Update items now
	if (pDX->m_bSaveAndValidate)
	{
		mTabs.DoDataExchange(pDX);
	}
}


BEGIN_MESSAGE_MAP(CPrefsAccountPanel, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAccountPanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountPanel message handlers

BOOL CPrefsAccountPanel::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mTabs.SubclassDlgItem(mTabID, this);

	// Create tab panels
	InitTabs();

	// Set initial tab
	mTabs.SetPanel(0);
	return true;
}

void CPrefsAccountPanel::OnSelChangePrefsAccountTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());
	
	*pResult = 0;
}

// Set data
void CPrefsAccountPanel::SetContent(void* data)
{
	mTabs.SetContent(data);
}

// Force update of data
bool CPrefsAccountPanel::UpdateContent(void* data)
{
	return mTabs.UpdateContent(data);
}