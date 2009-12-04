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


// CLicenseDialog.cpp : implementation file
//

#include "CLicenseDialog.h"

#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CLicenseDialog dialog


CLicenseDialog::CLicenseDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CLicenseDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLicenseDialog)
	//}}AFX_DATA_INIT
}


void CLicenseDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseDialog)
	DDX_UTF8Text(pDX, IDC_DEMOLICENSE, mDemoLicense);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CLicenseDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLicenseDialog message handlers
