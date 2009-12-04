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


// CSDIDoc.cp : implementation of the CSDIDoc class
//

#include "CSDIDoc.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSDIDoc

IMPLEMENT_DYNCREATE(CSDIDoc, CDocument)

BEGIN_MESSAGE_MAP(CSDIDoc, CDocument)
	//{{AFX_MSG_MAP(CSDIDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSDIDoc commands

void CSDIDoc::SetTitle(LPCTSTR lpszTitle)
{
	CDocument::SetTitle(lpszTitle);
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	
	// Title will be utf8 text
	CUnicodeUtils::SetWindowTextUTF8(aFrame, lpszTitle);

	CSDIFrame::UpdateWindows();
}
