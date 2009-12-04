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


// CNumberEdit.cpp : implementation file
//


#include "CNumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumberEdit

CNumberEdit::CNumberEdit()
{
}

CNumberEdit::~CNumberEdit()
{
}


BEGIN_MESSAGE_MAP(CNumberEdit, CCmdEdit)
	//{{AFX_MSG_MAP(CNumberEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Number edit support

BOOL CNumberEdit::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, bool read_only)
{
	BOOL result = false;

	result = CCmdEdit::CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_NUMBER, rect, pParentWnd, nID);

	SetReadOnly(read_only);
	
	return result;
}

// Set text from number
void CNumberEdit::SetValue(int value)
{
	CString s;
	s.Format(_T("%d"), value);
	SetWindowText(s);
}

// Get number from text
int CNumberEdit::GetValue(void) const
{
	CString s;
	GetWindowText(s);
	return ::atol(cdstring(s));
}
