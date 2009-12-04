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


// Source for CSearchCriteria class

#include "CSearchCriteria.h"

#include "CMulberryCommon.h"
#include "CSearchBase.h"
#include "CSearchCriteriaContainer.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


BEGIN_MESSAGE_MAP(CSearchCriteria, CCriteriaBase)
END_MESSAGE_MAP()

// Default constructor
CSearchCriteria::CSearchCriteria()
{
	mRules = false;
	mGroupItems = NULL;
}

// Default destructor
CSearchCriteria::~CSearchCriteria()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CSearchCriteria::Create(const CRect& rect, CWnd* pParentWnd)
{
	return CSearchCriteria::CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, rect, pParentWnd, IDC_STATIC);
}

CCriteriaBaseList& CSearchCriteria::GetList()
{
	return static_cast<CSearchCriteriaContainer*>(GetParent())->mCriteriaItems;
}

void CSearchCriteria::SwitchWith(CCriteriaBase* other)
{
	CSearchCriteria* other_criteria = static_cast<CSearchCriteria*>(other);
	CSearchItem* this_action = GetSearchItem();
	bool this_or = IsOr();
	SetSearchItem(other_criteria->GetSearchItem());
	SetOr(other_criteria->IsOr());
	other_criteria->SetSearchItem(this_action);
	other_criteria->SetOr(this_or);
}
	
// Respond to list changes
void CSearchCriteria::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CSearchCriteriaContainer::eBroadcast_SearchCriteriaContainerResized:
		// Change the size of this one
		ResizeWindowBy(this, 0, *reinterpret_cast<unsigned long*>(param), true);

		// Inform listeners of size change
		Broadcast_Message(eBroadcast_SearchCriteriaResized, param);
		break;
	default:;
	}
}

const int cCriteriaPanelHeight = 46;
const int cCriteriaPanelWidth = 460;
const int cCriteriaHOffset = 16;
const int cCriteriaHWidthAdjust = 20;
const int cCriteriaVOffset = 0;
const int cCriteriaVHeightAdjust = 8;

void CSearchCriteria::MakeGroup(CFilterItem::EType type)
{
	if (mGroupItems != NULL)
		return;

	// Get current size
	CRect size;
	GetWindowRect(size);

	mGroupItems = new CSearchCriteriaContainer;
	CRect gsize;
	gsize.left = 0;
	gsize.right = cCriteriaPanelWidth;
	gsize.top = 0;
	gsize.bottom = cCriteriaPanelHeight;
	mGroupItems->Create(gsize, this);
	mGroupItems->InitGroup(type, NULL);
	mGroupItems->SetRules(mRules);
	mGroupItems->Add_Listener(this);
	AddAlignment(new CWndAlignment(mGroupItems, CWndAlignment::eAlign_TopWidth));

	// Get size after init'ing the group
	mGroupItems->GetWindowRect(gsize);

	// Resize groups so that width first inside criteria
	::ResizeWindowBy(mGroupItems, size.Width() - cCriteriaHWidthAdjust - gsize.Width(), 0, false);
	
	// Shrink the size of this one
	::ResizeWindowBy(this, 0, gsize.Height() + cCriteriaVHeightAdjust, true);

	// mGroupItems new sub-panel
	::MoveWindowBy(mGroupItems, cCriteriaHOffset, size.Height() + cCriteriaVOffset, false);
	mGroupItems->ShowWindow(SW_SHOW);

	// Inform listeners of size change
	unsigned long change = gsize.Height() + cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}

void CSearchCriteria::RemoveGroup()
{
	if (mGroupItems == NULL)
		return;

	// Get size to shrink
	CRect size;
	mGroupItems->GetWindowRect(size);
	
	// Now delete the pane
	mGroupItems->DestroyWindow();
	delete mGroupItems;
	mGroupItems = NULL;
	
	// Shrink the size of this one
	ResizeWindowBy(this, 0, -size.Height() - cCriteriaVHeightAdjust, true);

	// Inform listeners of size change
	unsigned long change = -size.Height() - cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}
