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


// CDeleteIdentityDialog.cpp : implementation file
//

#include "CDeleteIdentityDialog.h"

#include "CACL.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteIdentityDialog dialog


CDeleteIdentityDialog::CDeleteIdentityDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CDeleteIdentityDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteIdentityDialog)
	//}}AFX_DATA_INIT
}


void CDeleteIdentityDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteIdentityDialog)
	//}}AFX_DATA_MAP
	
	// Set selection if exiting
	if (pDX->m_bSaveAndValidate)
	{
		mIdentityList.GetSelection(mIdentitySelect);
	}
}


BEGIN_MESSAGE_MAP(CDeleteIdentityDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CDeleteIdentityDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteIdentityDialog message handlers

BOOL CDeleteIdentityDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mIdentityList.SubclassDlgItem(IDC_DELETEIDENTITY_TABLE, this);

	cdstrvect items;
	CIdentityList& ids = CPreferences::sPrefs->mIdentities.Value();
	for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
		items.push_back((*iter).GetIdentity());

	// Fill table with items
	mIdentityList.SetContents(items);

	return true;
}
