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


// CQuickSearchText.cpp : implementation file
//


#include "CQuickSearchText.h"

#include "CGrayBackground.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText

CQuickSearchText::CQuickSearchText()
{
}

CQuickSearchText::~CQuickSearchText()
{
}


BEGIN_MESSAGE_MAP(CQuickSearchText, CCmdEdit)
	//{{AFX_MSG_MAP(CQuickSearchText)
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText message handlers

void CQuickSearchText::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch(nChar)
	{
	case VK_RETURN:
		Broadcast_Message(eBroadcast_Return, this);
		break;
	case VK_TAB:
		Broadcast_Message(eBroadcast_Tab, this);
		break;
	default:
		CCmdEdit::OnChar(nChar, nRepCnt, nFlags);
		Broadcast_Message(eBroadcast_Key, this);
		break;
	}
}

void CQuickSearchText::OnSetFocus(CWnd* pOldWnd)
{
	CCmdEdit::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);

	// Check whether parent is focus border
	CGrayBackground* focus = dynamic_cast<CGrayBackground*>(GetParent()->GetParent());
	if (focus)
		focus->SetBorderFocus(true);
}

void CQuickSearchText::OnKillFocus(CWnd* pNewWnd)
{
	CCmdEdit::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());

	// Check whether parent is focus border
	CGrayBackground* focus = dynamic_cast<CGrayBackground*>(GetParent()->GetParent());
	if (focus)
		focus->SetBorderFocus(false);
}

