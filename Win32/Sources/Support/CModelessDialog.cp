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


// CModelessDialog.cpp : implementation file
//


#include "CModelessDialog.h"

#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CModelessDialog dialog

IMPLEMENT_DYNAMIC(CModelessDialog, CHelpDialog)

CModelessDialog::CModelessDialog()
{
}

CModelessDialog::~CModelessDialog()
{
}

BEGIN_MESSAGE_MAP(CModelessDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CModelessDialog)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CModelessDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Bump title counter
	TitleCounter()++;
	
	// Alter title if needed
	if (TitleCounter() > 1)
	{
		cdstring title = CUnicodeUtils::GetWindowTextUTF8(this);
		title += " ";
		title += cdstring(TitleCounter());
		CUnicodeUtils::SetWindowTextUTF8(this, title);
	}

	return true;
}

void CModelessDialog::PostNcDestroy()
{
	// Bump down counter
	if (TitleCounter() > 0)
		TitleCounter()--;
	
	// Now delete the dialog object (its safe to do it here)
	delete this;
}

void CModelessDialog::OnOK()
{
	// Now do inherited
	CDialog::OnOK();

	// Now close the dialog
	DestroyWindow();
}

void CModelessDialog::OnCancel()
{
	// Now do inherited
	CDialog::OnCancel();

	// Now close the dialog
	DestroyWindow();
}

