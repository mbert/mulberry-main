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


// Source for CMailboxView class

#include "CMailboxView.h"

#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMessageView.h"
#include "CServerView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

BEGIN_MESSAGE_MAP(CMailboxView, CTableView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDC_MAILBOXSORTBTN, ToggleShowBy)
END_MESSAGE_MAP()

// Static members

cdmutexprotect<CMailboxView::CMailboxViewList> CMailboxView::sMailboxViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxView::CMailboxView()
{
	mToolbar = NULL;
	mCopyBtn = NULL;
	mServerView = NULL;
	mMessageView = NULL;
	mUsePreview = true;
	mPreviewInit = false;
	mUseSubstitute = false;
	mSubstituteIndex = 0;
	mSubstituteAccess = 1;
	mUseDynamicTab = false;

	// Add to list
	{
		cdmutexprotect<CMailboxViewList>::lock _lock(sMailboxViews);
		sMailboxViews->push_back(this);
	}
}

// Default destructor
CMailboxView::~CMailboxView()
{
	// Remove from list
	cdmutexprotect<CMailboxViewList>::lock _lock(sMailboxViews);
	CMailboxViewList::iterator found = std::find(sMailboxViews->begin(), sMailboxViews->end(), this);
	if (found != sMailboxViews->end())
		sMailboxViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMailboxView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetViewInfo(eMboxColumnNumber, 32, false);
	if (CTableView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Focus ring
	mFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width, height), this, IDC_STATIC);
	if (Is3Pane())
		mFocusRing.SetFocusBorder();
	AddAlignment(new CWndAlignment(&mFocusRing, CWndAlignment::eAlign_WidthHeight));
	UINT focus_indent = Is3Pane() ? 3 : 0;

	mSortBtn.Create(_T(""), CRect(width - focus_indent - 16, focus_indent, width - focus_indent, 16 + focus_indent), &mFocusRing, IDC_MAILBOXSORTBTN, 0, IDI_SORTASCENDING, IDI_SORTDESCENDING);
	mFocusRing.AddAlignment(new CWndAlignment(&mSortBtn, CWndAlignment::eAlign_TopRight));

	// Set status
	SetOpen();

	return 0;
}

// Set preview pane
void CMailboxView::SetPreview(CMessageView* view)
{
	mMessageView = view;
	
	// Make sure preview's super-commander is the table to get command handling done right
	if (mMessageView != NULL)
		mMessageView->SetSuperCommander(GetTable());
}

// Force toggle of sort direction
void CMailboxView::ToggleShowBy()
{
	GetTable()->OnSortDirection();
}

// Refresh Smart Address column
void CMailboxView::RefreshSmartAddress(void)
{
	// Force mailbox to reset smart address flags on all messages
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	mbox->ClearSmartAddress();

	// Look for smart address column
	for(int i = 0; i < mColumnInfo.size(); i++)
	{
		SColumnInfo col_info = mColumnInfo[i];

		if (col_info.column_type == eMboxColumnSmart)
		{
			// If any exist force complete refresh
			GetTable()->RedrawWindow();
			break;
		}
	}
}

void CMailboxView::FocusOwnerView()
{
	if (mServerView)
	{
		// If currently active, force focus to owner view
		if (GetTable()->IsTarget())
			mServerView->Focus();
	}
}
