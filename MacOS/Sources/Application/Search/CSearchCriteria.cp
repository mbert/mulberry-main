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

#include "CSearchCriteriaContainer.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSearchCriteria::CSearchCriteria()
{
	mRules = false;
	mGroupItems = NULL;
}

// Constructor from stream
CSearchCriteria::CSearchCriteria(LStream *inStream)
		: CCriteriaBase(inStream)
{
	mRules = false;
	mGroupItems = NULL;
}

// Default destructor
CSearchCriteria::~CSearchCriteria()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

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
		ResizeFrameBy(0, *reinterpret_cast<unsigned long*>(param), true);

		// Inform listeners of size change
		Broadcast_Message(eBroadcast_SearchCriteriaResized, param);
		break;
	default:;
	}
}

const int cCriteriaHOffset = 16;
const int cCriteriaHWidthAdjust = 20;
const int cCriteriaVOffset = 2;
const int cCriteriaVHeightAdjust = 8;

void CSearchCriteria::MakeGroup(CFilterItem::EType type)
{
	if (mGroupItems != NULL)
		return;

	// Get current size
	SDimension16 size;
	GetFrameSize(size);
	
	// Locate the parent window
	LView* parent = GetSuperView();
	while((parent != NULL) && (dynamic_cast<LWindow*>(parent) == NULL))
		parent = parent->GetSuperView();
	LWindow* wnd = dynamic_cast<LWindow*>(parent);

	// Reanimate a new search criteria panel
	LCommander* defCommander;
	wnd->GetSubCommanders().FetchItemAt(1, defCommander);
	wnd->SetDefaultCommander(defCommander);
	wnd->SetDefaultView(wnd);

	mGroupItems = (CSearchCriteriaContainer*) UReanimator::ReadObjects('PPob', paneid_SearchCriteriaContainer);
	mGroupItems->FinishCreate();
	mGroupItems->InitGroup(type, NULL);
	mGroupItems->SetRules(mRules);
	mGroupItems->Add_Listener(this);

	// Put inside panel
	mGroupItems->PutInside(this);

	// Get size to increase
	SDimension16 gsize;
	mGroupItems->GetFrameSize(gsize);
	
	// Resize groups so that width first inside criteria
	mGroupItems->ResizeFrameBy(size.width - cCriteriaHWidthAdjust - gsize.width, 0, false);
	
	// Shrink the size of this one
	ResizeFrameBy(0, gsize.height + cCriteriaVHeightAdjust, true);

	// Position new sub-panel
	mGroupItems->PlaceInSuperFrameAt(cCriteriaHOffset, size.height + cCriteriaVOffset, false);
	mGroupItems->Show();

	// Inform listeners of size change
	unsigned long change = gsize.height + cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}

void CSearchCriteria::RemoveGroup()
{
	if (mGroupItems == NULL)
		return;

	// Get size to shrink
	SDimension16 size;
	mGroupItems->GetFrameSize(size);
	
	// Now delete the pane
	delete mGroupItems;
	mGroupItems = NULL;
	
	// Shrink the size of this one
	ResizeFrameBy(0, -size.height - cCriteriaVHeightAdjust, true);

	// Inform listeners of size change
	unsigned long change = -size.height - cCriteriaVHeightAdjust;
	Broadcast_Message(eBroadcast_SearchCriteriaResized, &change);
}
