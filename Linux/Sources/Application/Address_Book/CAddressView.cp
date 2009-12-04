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


// Source for CAddressView class

#include "CAddressView.h"

#include "CAddressPreview.h"
#include "CGroupPreview.h"
#include "CPreferences.h"

#include <algorithm>

// Static members

cdmutexprotect<CAddressView::CAddressViewList> CAddressView::sAddressViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressView::CAddressView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: CBaseView(enclosure, hSizing, vSizing, x, y, w, h)
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

// Get details of sub-panes
void CAddressView::OnCreate()
{
	// Do inherited
	CBaseView::OnCreate();

	InitPreviews();
	SetOpen();
}

// Get details of sub-panes
void CAddressView::InitPreviews(void)
{
	// Read the address view resource
	mAddressPreview = new CAddressPreview(this, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 300);
	mAddressPreview->OnCreate();
	mAddressPreview->FitToEnclosure(kTrue, kTrue);
	mAddressPreview->Hide();

	// Read the group view resource
	mGroupPreview = new CGroupPreview(this, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 300);
	mGroupPreview->OnCreate();
	mGroupPreview->FitToEnclosure(kTrue, kTrue);
	mGroupPreview->Hide();
	
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
	mAddressPreview->SetAddressBook(NULL);
	mAddressPreview->SetAddress(NULL);
	mGroupPreview->SetAddressBook(NULL);
	mGroupPreview->SetGroup(NULL);
}

void CAddressView::SetAddress(CAddressBook* adbk, CAdbkAddress* addr, bool refresh)
{
	// Close any existing items if not refreshing
	if (mCurrent && !refresh)
		mCurrent->Close();
	
	// Hide the group and empty it
	mGroupPreview->Hide();
	mGroupPreview->Deactivate();
	mGroupPreview->SetGroup(NULL);
	
	// Set the address and show
	mAddressPreview->SetAddressBook(adbk);
	mAddressPreview->SetAddress(addr);
	mAddressPreview->Activate();
	mAddressPreview->Show();
	mCurrent = mAddressPreview;

	// Focus on the preview if requested
	if (addr && GetViewOptions().GetSelectAddressPane())
		Focus();

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

// Address removed by someone else
void CAddressView::ClearAddress()
{
	mAddressPreview->ClearAddress();
	mAddressPreview->Deactivate();
}

CAdbkAddress* CAddressView::GetAddress() const
{
	return mAddressPreview->GetAddress();
}

void CAddressView::SetGroup(CAddressBook* adbk, CGroup* grp, bool refresh)
{
	// Close any existing items if not refreshing
	if (mCurrent && !refresh)
		mCurrent->Close();
	
	// Hide the address and empty it
	mAddressPreview->Hide();
	mAddressPreview->Deactivate();
	mAddressPreview->SetAddress(NULL);
	
	// Set the address and show
	mGroupPreview->SetAddressBook(adbk);
	mGroupPreview->SetGroup(grp);
	mGroupPreview->Activate();
	mGroupPreview->Show();
	mCurrent = mGroupPreview;

	// Focus on the preview if requested
	if (grp && GetViewOptions().GetSelectAddressPane())
		Focus();

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

// Address removed by someone else
void CAddressView::ClearGroup()
{
	mGroupPreview->ClearGroup();
	mGroupPreview->Deactivate();
}

CGroup* CAddressView::GetGroup() const
{
	return mGroupPreview->GetGroup();
}

bool CAddressView::HasFocus() const
{
	// Switch target to the text (will focus)
	return false;
}

void CAddressView::Focus()
{
	if (IsVisible() && mCurrent)
		// Switch target to the text (will focus)
		mCurrent->Focus();
}

// Get options for this view
const CAddressViewOptions& CAddressView::GetViewOptions() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions();
}
