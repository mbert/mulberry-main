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


// C3PaneDoc.cp : implementation of the C3PaneDoc class
//


#include "C3PaneDoc.h"

#include "C3PaneWindow.h"

/////////////////////////////////////////////////////////////////////////////
// C3PaneDoc

IMPLEMENT_DYNCREATE(C3PaneDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(C3PaneDoc, CSDIDoc)
	//ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3PaneDoc commands

#if 0
void C3PaneDoc::OnFileClose()
{
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	C3PaneWindow* wnd = (C3PaneWindow*) aFrame->GetTopWindow();

	// Check with window first
	if (wnd->AttemptClose())
		CSDIDoc::OnFileClose();
}
#endif
