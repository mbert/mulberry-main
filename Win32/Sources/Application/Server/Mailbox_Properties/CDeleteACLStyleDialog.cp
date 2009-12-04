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


// CDeleteACLStyleDialog.cpp : implementation file
//

#include "CDeleteACLStyleDialog.h"

#include "CACL.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteACLStyleDialog dialog


CDeleteACLStyleDialog::CDeleteACLStyleDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CDeleteACLStyleDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteACLStyleDialog)
	//}}AFX_DATA_INIT
}


void CDeleteACLStyleDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteACLStyleDialog)
	//}}AFX_DATA_MAP
	
	// Set selection if exiting
	if (pDX->m_bSaveAndValidate)
	{
		mStyleList.GetSelection(mStyleSelect);
	}
}


BEGIN_MESSAGE_MAP(CDeleteACLStyleDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CDeleteACLStyleDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteACLStyleDialog message handlers

BOOL CDeleteACLStyleDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mStyleList.SubclassDlgItem(IDC_ACLSTYLE_TABLE, this);

	// Create columns and adjust flag rect
	mStyleList.InsertCols(1, 0);

	// Fill table with items
	cdstrvect items;
	for(SACLStyleList::const_iterator iter = CPreferences::sPrefs->mMboxACLStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mMboxACLStyles.GetValue().end(); iter++)
		items.push_back((*iter).first);

	// Fill table with items
	mStyleList.SetContents(items);
	
	CRect client;
	mStyleList.GetClientRect(client);
	int cx = client.Width();

	// Name column has variable width
	mStyleList.SetColWidth(cx, 1, 1);

	return true;
}
