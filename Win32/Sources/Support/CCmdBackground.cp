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


// CCmdBackground.cpp : implementation file
//


#include "CCmdBackground.h"

IMPLEMENT_DYNAMIC(CCmdBackground, CStatic)

/////////////////////////////////////////////////////////////////////////////
// CCmdBackground

CCmdBackground::CCmdBackground()
{
}

CCmdBackground::~CCmdBackground()
{
}


BEGIN_MESSAGE_MAP(CCmdBackground, CStatic)
	//{{AFX_MSG_MAP(CCmdBackground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCmdBackground::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Pass message up to owning window
	if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CStatic::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
