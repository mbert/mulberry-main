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


// Source for CAddressView class

#include "CAddressView.h"

#include "CMulberryCommon.h"

// Static members

cdmutexprotect<CAddressView::CAddressViewList> CAddressView::sAddressViews;

BEGIN_MESSAGE_MAP(CAddressView, CBaseView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Default constructor
CAddressView::CAddressView()
{
	mCurrent = NULL;

	// Add to list
	{
		cdmutexprotect<CAddressViewList>::lock _lock(sAddressViews);
		sAddressViews->push_back(this);
	}
}

// Default destructor
CAddressView::~CAddressView()
{
	// Remove from list
	cdmutexprotect<CAddressViewList>::lock _lock(sAddressViews);
	CAddressViewList::iterator found = std::find(sAddressViews->begin(), sAddressViews->end(), this);
	if (found != sAddressViews->end())
		sAddressViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CAddressView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Tabs with alignment
	InitPreviews();
	SetOpen();

	return 0;
}

// Get details of sub-panes
void CAddressView::InitPreviews(void)
{
	// Read the address view resource
	mAddressPreview.Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), this, IDC_STATIC);
	::ExpandChildToFit(this, &mAddressPreview, true, true);
	mAddressPreview.ShowWindow(SW_HIDE);
	AddAlignment(new CWndAlignment(&mAddressPreview, CWndAlignment::eAlign_WidthHeight));

	// Read the group view resource
	mGroupPreview.CreateDialogItems(CGroupPreview::IDD, this);
	::ExpandChildToFit(this, &mGroupPreview, true, true);
	mGroupPreview.ShowWindow(SW_HIDE);
	AddAlignment(new CWndAlignment(&mGroupPreview, CWndAlignment::eAlign_WidthHeight));
	
	// Now force address preview to be visible but empty
	SetAddress(NULL, NULL);
}

// Check for window
bool CAddressView::ViewExists(const CAddressView* wnd)
{
	cdmutexprotect<CAddressViewList>::lock _lock(sAddressViews);
	CAddressViewList::iterator found = std::find(sAddressViews->begin(), sAddressViews->end(), wnd);
	return found != sAddressViews->end();
}

void CAddressView::DoClose()
{
	// Close any existing items
	if (mCurrent)
		mCurrent->Close();
	mCurrent = NULL;
	
	// Clear out address/group
	ClearAddress();
	ClearGroup();
	
	// Make sure preview items are NULL
	mAddressPreview.SetAddressBook(NULL);
	mAddressPreview.SetAddress(NULL);
	mGroupPreview.SetAddressBook(NULL);
	mGroupPreview.SetGroup(NULL);
}

void CAddressView::SetAddress(CAddressBook* adbk, CAdbkAddress* addr, bool refresh)
{
	// Close any existing items
	if (mCurrent && !refresh)
		mCurrent->Close();
	
	// Hide the group and empty it
	mGroupPreview.ShowWindow(SW_HIDE);
	mGroupPreview.SetGroup(NULL);
	
	// Set the address and show
	mAddressPreview.SetAddressBook(adbk);
	mAddressPreview.SetAddress(addr);
	mAddressPreview.ShowWindow(SW_SHOW);
	mCurrent = &mAddressPreview;

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

// Address removed by someone else
void CAddressView::ClearAddress()
{
	mAddressPreview.ClearAddress();
}

CAdbkAddress* CAddressView::GetAddress() const
{
	return mAddressPreview.GetAddress();
}

void CAddressView::SetGroup(CAddressBook* adbk, CGroup* grp, bool refresh)
{
	// Close any existing items
	if (mCurrent && !refresh)
		mCurrent->Close();
	
	// Hide the address and empty it
	mAddressPreview.ShowWindow(SW_HIDE);
	mAddressPreview.SetAddress(NULL);
	
	// Set the address and show
	mGroupPreview.SetAddressBook(adbk);
	mGroupPreview.SetGroup(grp);
	mGroupPreview.ShowWindow(SW_SHOW);
	mCurrent = &mGroupPreview;

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

// Address removed by someone else
void CAddressView::ClearGroup()
{
	mAddressPreview.ClearAddress();
}

CGroup* CAddressView::GetGroup() const
{
	return mGroupPreview.GetGroup();
}

bool CAddressView::HasFocus() const
{
	// Switch target to the text (will focus)
	return false;
}

void CAddressView::Focus()
{
	if (IsWindowVisible() && mCurrent)
		// Switch target to the text (will focus)
		mCurrent->Focus();
}
