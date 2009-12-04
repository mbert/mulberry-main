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


// CSpacebarEdit.cpp : implementation file
//


#include "CSpacebarEdit.h"

#include "CMessageWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEdit

CSpacebarEdit::CSpacebarEdit()
{
	mMsgWindow = NULL;
}

CSpacebarEdit::~CSpacebarEdit()
{
}


BEGIN_MESSAGE_MAP(CSpacebarEdit, CCmdEdit)
	//{{AFX_MSG_MAP(CSpacebarEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEdit message handlers

void CSpacebarEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch(nChar)
	{
	case VK_SPACE:
	case VK_BACK:
	case VK_DELETE:
		// Pass to text display
		static_cast<CSpacebarEditView*>(mMsgWindow->GetText())->OnChar(nChar, nRepCnt, nFlags);
		break;
	default:
		CCmdEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}
