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


// Default constructor
CSearchCriteria::CSearchCriteria(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CCriteriaBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mRules = false;
	mGroupItems = NULL;
}

// Default destructor
CSearchCriteria::~CSearchCriteria()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

CCriteriaBaseList& CSearchCriteria::GetList()
{
	return const_cast<CSearchCriteriaContainer*>(static_cast<const CSearchCriteriaContainer*>(GetEnclosure()))->mCriteriaItems;
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
		AdjustSize(0, *reinterpret_cast<unsigned long*>(param));

		// Inform listeners of size change
		Broadcast_Message(eBroadcast_SearchCriteriaResized, param);
		break;
	default:;
	}
}

const int cCriteriaPanelHeight = 46;
const int cCriteriaPanelWidth = 496;
const int cCriteriaHOffset = 16;
const int cCriteriaHWidthAdjust = 20;
const int cCriteriaVOffset = 0;
const int cCriteriaVHeightAdjust = 8;

void CSearchCriteria::MakeGroup(CFilterItem::EType type)
{
	if (mGroupItems != NULL)
		return;

	// Get current size
	JRect size = GetAperture();

	JRect gsize;
	gsize.left = 0;
	gsize.right = cCriteriaPanelWidth;
	gsize.top = 0;
	gsize.bottom = cCriteriaPanelHeight;
	mGroupItems = new CSearchCriteriaContainer(this, JXWidget::kHElastic, JXWidget::kFixedTop, gsize.left, gsize.top, gsize.width(), gsize.height());
	mGroupItems->OnCreate();
	mGroupItems->InitGroup(type, NULL);
	mGroupItems->SetRules(mRules);
	mGroupItems->Add_Listener(this);

	// Get size after init'ing the group
	gsize = mGroupItems->GetFrame();

	// Resize groups so that width first inside criteria
	mGroupItems->AdjustSize(size.width() - cCriteriaHWidthAdjust - gsize.width(), 0);
	
	// Shrink the size of this one
	AdjustSize(0, gsize.height() + cCriteriaVHeightAdjust);

	// mGroupItems new sub-panel
	mGroupItems->Move(cCriteriaHOffset, size.height() + cCriteriaVOffset);
	mGroupItems->Show();

	// Inform listeners of size change
	unsigned long change = gsize.height() + cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}

void CSearchCriteria::RemoveGroup()
{
	if (mGroupItems == NULL)
		return;

	// Get size to shrink
	JRect size = mGroupItems->GetFrame();
	
	// Now delete the pane
	delete mGroupItems;
	mGroupItems = NULL;
	
	// Shrink the size of this one
	AdjustSize(0, -size.height() - cCriteriaVHeightAdjust);

	// Inform listeners of size change
	unsigned long change = -size.height() - cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}

