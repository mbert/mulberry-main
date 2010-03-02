/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


	// Source for CMailboxWindow class

#include "CMailboxWindow.h"

#include "CMailboxTable.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxWindow

BEGIN_MESSAGE_MAP(CMailboxWindow, CTableViewWindow)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWPREVIEW, OnUpdateMenuShowPreview)
	ON_COMMAND(IDM_WINDOWS_SHOWPREVIEW, OnShowPreview)
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWPREVIEW, OnUpdateShowPreview)
	ON_COMMAND(IDC_TOOLBARSHOWPREVIEW, OnShowPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailboxWindow construction/destruction

cdmutexprotect<CMailboxWindow::CMailboxWindowList> CMailboxWindow::sMboxWindows;

// Default constructor
CMailboxWindow::CMailboxWindow()
{
	mPreviewVisible = true;

	// Add to list
	{
		cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
		sMboxWindows->push_back(this);
	}

	mMailboxFrame = NULL;
}

// Default destructor
CMailboxWindow::~CMailboxWindow()
{
}

// Manually create document
// O T H E R  M E T H O D S ____________________________________________________________________________

// Tidy up when window closed
void CMailboxWindow::OnDestroy(void)
{
	// Remove from list
	{
		cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
		CMailboxWindowList::iterator found = std::find(sMboxWindows->begin(), sMboxWindows->end(), this);
		if (found != sMboxWindows->end())
			sMboxWindows->erase(found);
	}

	// Kill visual representation
	CTableViewWindow::OnDestroy();
}

void CMailboxWindow::OnUpdateMenuShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	::OnUpdateMenuTitle(pCmdUI, txt);
}

void CMailboxWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

// Tidy up when window closed
void CMailboxWindow::OnShowPreview(void)
{
	ShowPreview(!mPreviewVisible);
}
