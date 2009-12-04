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


// CAdbkManagerDoc.cp : implementation of the CAdbkManagerDoc class
//


#include "CAdbkManagerDoc.h"

#include "CAdbkManagerWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerDoc

IMPLEMENT_DYNCREATE(CAdbkManagerDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CAdbkManagerDoc, CSDIDoc)
	//{{AFX_MSG_MAP(CAdbkManagerDoc)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerDoc commands

void CAdbkManagerDoc::OnFileClose()
{
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	CAdbkManagerWindow* wnd = (CAdbkManagerWindow*) aFrame->GetTopWindow();

	// Check with window first
	if (wnd->GetAdbkManagerView()->TestClose())
		CSDIDoc::OnFileClose();
}
