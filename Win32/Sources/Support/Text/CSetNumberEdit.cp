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


// CSetNumberEdit.cpp : implementation file
//


#include "CSetNumberEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CSetNumberEdit

CSetNumberEdit::CSetNumberEdit()
{
}

CSetNumberEdit::~CSetNumberEdit()
{
}


BEGIN_MESSAGE_MAP(CSetNumberEdit, CNumberEdit)
	//{{AFX_MSG_MAP(CSetNumberEdit)
	ON_WM_MOVE()
	ON_WM_ENABLE()
	ON_WM_SHOWWINDOW()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSetNumberEdit::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nSpinID, bool read_only)
{
	mVisible = false;

	// Do default first
	BOOL result = CNumberEdit::Create(rect, pParentWnd, nID, read_only);

	// Create spinner
	CRect copyRect = rect;
	copyRect.left = rect.right;
	copyRect.right = copyRect.left + 8;
	copyRect.top += 2;
	copyRect.bottom -= 2;
	mSpinner.Create(WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_NOTHOUSANDS | UDS_ALIGNRIGHT, copyRect, pParentWnd, nSpinID);
	mSpinner.SetBuddy(this);

	return result;
}

/////////////////////////////////////////////////////////////////////////////
// CSetNumberEdit message handlers

// Control moved
void CSetNumberEdit::OnMove(int x, int y)
{
	// Do default
	CNumberEdit::OnMove(x, y);

	// Adjust to move to rhs of edit control
#if 0
	CRect rect;
	GetWindowRect(rect);
	x += rect.Width() - 8;
	y -= 2;

	// Move spinner
	mSpinner.SetWindowPos(nil, x, y, 8, rect.Height() - 2, SWP_NOZORDER);
#else
	mSpinner.SetBuddy(this);
	
	// Must hide again
	if (!mVisible)
		mSpinner.ShowWindow(SW_HIDE);
#endif
}

// Control enabled
void CSetNumberEdit::OnEnable(BOOL bEnable)
{
	// Do default
	CNumberEdit::OnEnable(bEnable);

	// Enable spinner
	mSpinner.EnableWindow(bEnable);
}

// Control show/hide
void CSetNumberEdit::OnShowWindow(BOOL bShow, UINT nStatus)
{
	mVisible = bShow;

	// Do default
	CNumberEdit::OnShowWindow(bShow, nStatus);

	// Explicit show/hide spinner only if ShowWindow called for edit control (nStatus == 0)
	if (!nStatus)
		mSpinner.ShowWindow(bShow);
}

void CSetNumberEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == VK_RETURN)
	{
		// Send command to parent
		GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
		CNumberEdit::OnChar(nChar, nRepCnt, nFlags);
}
