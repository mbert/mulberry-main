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


// CAboutBox.cpp : implementation file
//


#include "CAboutBox.h"
#include "CUnicodeUtils.h"

void AFXAPI DDX_UTF8TextSubs(CDataExchange* pDX, int nIDC, cdstring& value);
void AFXAPI DDX_UTF8TextSubs(CDataExchange* pDX, int nIDC, cdstring& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		value = CUnicodeUtils::GetWindowTextUTF8(hWndCtrl);
	}
	else
	{
		cdstring temp = CUnicodeUtils::GetWindowTextUTF8(hWndCtrl);
		temp.Substitute(value);
		CUnicodeUtils::SetWindowTextUTF8(hWndCtrl, temp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAboutBox dialog


CAboutBox::CAboutBox(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CAboutBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutBox)
	//}}AFX_DATA_INIT
}


void CAboutBox::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutBox)
	DDX_UTF8TextSubs(pDX, IDC_ABOUTVERSION, mVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutBox, CHelpDialog)
	//{{AFX_MSG_MAP(CAboutBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutBox message handlers

BOOL CAboutBox::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();
	
	return true;
}
