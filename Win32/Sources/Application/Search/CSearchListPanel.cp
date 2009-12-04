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


// CSearchListPanel.cp : implementation of the CSearchListPanel class
//

#include "CSearchListPanel.h"

#include "CIconTextTable.h"
#include "CMulberryApp.h"
#include "CSDIFrame.h"
#include "CSearchEngine.h"
#include "CSearchWindow.h"
#include "CSearchOptionsDialog.h"

const int cOptionsBtnWidth = 60;
const int cOptionsBtnHeight = 24;
const int cOptionsBtnHOffset = 76;
const int cOptionsBtnVOffset = 6;

/////////////////////////////////////////////////////////////////////////////
// CSearchListPanel

BEGIN_MESSAGE_MAP(CSearchListPanel, CMailboxListPanel)
	ON_COMMAND(IDC_SEARCH_CLEARLIST, OnClearMailboxList)
	ON_COMMAND(IDC_SEARCH_OPTIONS, OnOptions)
	ON_COMMAND(IDC_SEARCH_MAILBOXLIST, OnOpenMailboxList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchListPanel construction/destruction

CSearchListPanel::CSearchListPanel()
{
}

CSearchListPanel::~CSearchListPanel()
{
}

void CSearchListPanel::CreateSelf(CWnd* parent_frame, CWnd* move_parent1, CWnd* move_parent2, int width, int height)
{
	// Do inherited
	CMailboxListPanel::CreateSelf(parent_frame, move_parent1, move_parent2, width, height);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	CString s;
	s.LoadString(IDS_SEARCH_OPTIONSBTN);
	CRect r = CRect(width - cOptionsBtnHOffset - 2*large_offset, cOptionsBtnVOffset, width - cOptionsBtnHOffset - 2*large_offset + cOptionsBtnWidth, cOptionsBtnVOffset + cOptionsBtnHeight);
	mOptionsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mMailboxArea, IDC_SEARCH_OPTIONS);
	mOptionsBtn.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mOptionsBtn, CWndAlignment::eAlign_TopRight));
	
	// Turn on select with tab in mailbox list
	mMailboxList.SetTabSelection(true, IDI_SEARCH_HIT);
}

void CSearchListPanel::SetInProgress(bool in_progress)
{
	CMailboxListPanel::SetInProgress(in_progress);

	mOptionsBtn.EnableWindow(!mInProgress);
}

#pragma mark ____________________________Mailbox List

void CSearchListPanel::OnClearMailboxList()
{
	// Delete everything in list and reset table
	CSearchEngine::sSearchEngine.ClearTargets();
	ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
										CSearchEngine::sSearchEngine.GetTargetHits());
}

// Add a list of mailboxes
void CSearchListPanel::AddMboxList(const CMboxList* list)
{
	CSearchWindow::sSearchWindow->AddMboxList(list);
}

void CSearchListPanel::OnOptions()
{
	// Create the dialog
	CSearchOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetItems();

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
		dlog.GetItems();
}


void CSearchListPanel::OnOpenMailboxList()
{
	// Do not allow open while in progress (at least for now)
	if (mInProgress)
	{
		::MessageBeep(-1);
		return;
	}

	ulvector selection;

	mMailboxList.GetSelection(selection);
	
	// Open each mailbox
	for(ulvector::reverse_iterator riter = selection.rbegin(); riter != selection.rend(); riter++)
		// Get search engine to open it
		CSearchEngine::sSearchEngine.OpenTarget(*riter);
}
