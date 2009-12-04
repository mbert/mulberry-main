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


// Source for CKeyChoiceDialog class

#include "CKeyChoiceDialog.h"

#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyChoiceDialog dialog


CKeyChoiceDialog::CKeyChoiceDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CKeyChoiceDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyChoiceDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	mOK = false;
}


void CKeyChoiceDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyChoiceDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyChoiceDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CKeyChoiceDialog)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyChoiceDialog message handlers

void CKeyChoiceDialog::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Cache key press value
	mKey = nChar;

	// Cache modifiers
	mMods = CKeyModifiers(0);

	mOK = true;
	EndDialog(IDOK);
}

BOOL CKeyChoiceDialog::PreTranslateMessage(MSG* pMsg)
{
	ASSERT_VALID(this);

	// Special for tab key
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB))
	{
		OnChar(VK_TAB, 0, 0);
		return TRUE;
	}
	else if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		OnChar(VK_RETURN, 0, 0);
		return TRUE;
	}
	else if (pMsg->message == WM_CHAR)
	{
		OnChar(pMsg->wParam, 0, 0);
		return TRUE;
	}

	// Do inherited
	return CHelpDialog::PreTranslateMessage(pMsg);
}

bool CKeyChoiceDialog::PoseDialog(unsigned char& key, CKeyModifiers& mods)
{
	bool result = false;

	CKeyChoiceDialog dlog(CSDIFrame::GetAppTopWindow());

	dlog.DoModal();
	if (dlog.mOK)
	{
		key = dlog.mKey;
		mods = dlog.mMods;
		result = true;
	}
	

	return result;
}
