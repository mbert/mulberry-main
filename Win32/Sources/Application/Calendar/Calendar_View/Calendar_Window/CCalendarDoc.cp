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


// CCalendarDoc.cp : implementation of the CCalendarDoc class
//


#include "CCalendarDoc.h"

#include "CCalendarWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CCalendarDoc

IMPLEMENT_DYNCREATE(CCalendarDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CCalendarDoc, CSDIDoc)
	//{{AFX_MSG_MAP(CCalendarDoc)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCalendarDoc::OnFileClose(void)
{
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	CCalendarWindow* wnd = (CCalendarWindow*) aFrame->GetTopWindow();

	// Check with window first
	if (wnd->GetCalendarView()->TestClose())
		CSDIDoc::OnFileClose();
}
