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


// CMailboxDoc.cp : implementation of the CMailboxDoc class
//


#include "CMailboxDoc.h"

#include "CMailboxView.h"
#include "CMailboxWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxDoc

IMPLEMENT_DYNCREATE(CMailboxDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CMailboxDoc, CSDIDoc)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMailboxDoc commands

void CMailboxDoc::OnFileClose()
{
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	CMailboxWindow* wnd = (CMailboxWindow*) aFrame->GetTopWindow();

	// Check with window first
	if (wnd->GetMailboxView()->TestClose())
		CSDIDoc::OnFileClose();
}
