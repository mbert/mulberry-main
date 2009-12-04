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


// CNewACLStyleDialog.cpp : implementation file
//

#include "CNewACLStyleDialog.h"

#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CNewACLStyleDialog dialog


CNewACLStyleDialog::CNewACLStyleDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CNewACLStyleDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewACLStyleDialog)
	mName = _T("");
	//}}AFX_DATA_INIT
}


void CNewACLStyleDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewACLStyleDialog)
	DDX_UTF8Text(pDX, IDC_NEWACLSTYLE_NAME, mName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewACLStyleDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNewACLStyleDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewACLStyleDialog message handlers
