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


// CContainerWnd.cpp : implementation file
//


#include "CContainerWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CContainerWnd

CContainerWnd::CContainerWnd()
{
}

CContainerWnd::~CContainerWnd()
{
}


BEGIN_MESSAGE_MAP(CContainerWnd, CWnd)
	//{{AFX_MSG_MAP(CContainerWnd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CContainerWnd columns
void CContainerWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// Resize top window
	CWnd* top = GetTopWindow();
	if (top)
		top->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);
	
}

BOOL CContainerWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Try focused child window
	CWnd* focus = GetFocus();
	if (focus && IsChild(focus) && focus->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// Ask parent to handle
	if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
