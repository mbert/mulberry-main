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


// Source for CSearchCriteriaContainer class

#include "CSearchCriteriaContainer.h"

#include "CSearchCriteria.h"
#include "CSearchCriteriaLocal.h"
#include "CSearchCriteriaSIEVE.h"
#include "CSearchItem.h"

#include <LPushButton.h>

// Default constructor
CSearchCriteriaContainer::CSearchCriteriaContainer()
{
	mFilterType = CFilterItem::eLocal;
	mTopLevel = false;
	mRules = false;
}

// Constructor from stream
CSearchCriteriaContainer::CSearchCriteriaContainer(LStream *inStream)
		: LView(inStream)
{
	mFilterType = CFilterItem::eLocal;
	mTopLevel = false;
	mRules = false;
}

// Default destructor
CSearchCriteriaContainer::~CSearchCriteriaContainer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CSearchCriteriaContainer::FinishCreateSelf()
{
	// Do inherited
	LView::FinishCreateSelf();

	// Locate the parent
	LView* parent = GetSuperView();
	while((parent != NULL) && (dynamic_cast<LWindow*>(parent) == NULL))
		parent = parent->GetSuperView();
	mContainer = dynamic_cast<LWindow*>(parent);

	// Get all controls
	mSeparator = (LPushButton*) FindPaneByID(paneid_SearchCriteriaContainerSeparator);
	mMoreBtn = (LPushButton*) FindPaneByID(paneid_SearchCriteriaContainerMore);
	mFewerBtn = (LPushButton*) FindPaneByID(paneid_SearchCriteriaContainerFewer);
	mClearBtn = (LPushButton*) FindPaneByID(paneid_SearchCriteriaContainerClear);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSearchCriteriaContainerBtns);
	
	// Hide top-level items and resize
	if (mTopLevel)
	{
		mSeparator->Hide();
		mMoreBtn->Hide();
		mFewerBtn->Hide();
		mClearBtn->Hide();
		
		SDimension16 size;
		mMoreBtn->GetFrameSize(size);
		ResizeFrameBy(0, -size.height, false);
	}
}

// Respond to clicks in the icon buttons
void CSearchCriteriaContainer::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_SearchCriteriaContainerMore:
		OnMore();
		break;

	case msg_SearchCriteriaContainerFewer:
		OnFewer();
		break;

	case msg_SearchCriteriaContainerClear:
		OnClear();
		break;
	}
}

// Respond to list changes
void CSearchCriteriaContainer::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CSearchCriteria::eBroadcast_SearchCriteriaResized:
		// Change the size of this one
		ResizeFrameBy(0, *reinterpret_cast<unsigned long*>(param), true);
		RecalcLayout();

		// Inform listeners of size change
		Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, param);
		break;
	default:;
	}
}

// Activate search item
void CSearchCriteriaContainer::DoActivate()
{
	// Iterate over all criteria and first first that wants to activate
	long num = mCriteria.GetCount();
	for(long i = 1; i <= num; i++)
	{
		if (static_cast<CSearchCriteria*>(mCriteria[i])->DoActivate())
			break;
	}
}


#pragma mark ____________________________Commands

void CSearchCriteriaContainer::OnMore()
{
	AddCriteria();
}

void CSearchCriteriaContainer::OnFewer()
{
	RemoveCriteria();
}

void CSearchCriteriaContainer::OnClear()
{
	// Remove all but the first one
	while(mCriteria.GetCount() >1)
		RemoveCriteria();

	// Reset the first one
	CSearchCriteria* criteria = static_cast<CSearchCriteria*>(mCriteria[1]);
	criteria->SetSearchItem(NULL);
}

#pragma mark ____________________________Criteria Panels

void CSearchCriteriaContainer::InitGroup(CFilterItem::EType type, const CSearchItem* spec)
{
	mFilterType = type;

	// Strip off group
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
	{
		const CSearchItem* item = static_cast<const CSearchItem*>(spec->GetData());
		InitCriteria(item);
	}
	else
		InitCriteria(NULL);
}

void CSearchCriteriaContainer::InitCriteria(const CSearchItem* spec)
{
	RemoveAllCriteria();

	// Check for And/Or
	if (spec && (spec->GetType() == CSearchItem::eAnd))
	{
		const CSearchItemList* list = static_cast<const CSearchItemList*>(spec->GetData());
		for(CSearchItemList::const_iterator iter = list->begin(); iter != list->end(); iter++)
			AddCriteria(*iter, false);
	}
	else if (spec && (spec->GetType() == CSearchItem::eOr))
	{
		const CSearchItemList* list = static_cast<const CSearchItemList*>(spec->GetData());
		for(CSearchItemList::const_iterator iter1 = list->begin(); iter1 != list->end(); iter1++)
		{
			// Check for and list
			if ((*iter1)->GetType() == CSearchItem::eAnd)
			{
				const CSearchItemList* list = static_cast<const CSearchItemList*>((*iter1)->GetData());
				for(CSearchItemList::const_iterator iter2 = list->begin(); iter2 != list->end(); iter2++)
					AddCriteria(*iter2, iter2 == list->begin());
			}
			else
				AddCriteria(*iter1);
		}
	}
	else
		AddCriteria(spec);
}

const int cCriteriaHOffset = 4;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 4;

void CSearchCriteriaContainer::AddCriteria(const CSearchItem* spec, bool use_or)
{
	// Reanimate a new search criteria panel
	LCommander* defCommander;
	mContainer->GetSubCommanders().FetchItemAt(1, defCommander);
	mContainer->SetDefaultCommander(defCommander);
	mContainer->SetDefaultView(mContainer);

	PaneIDT paneid = 0;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		paneid = paneid_SearchCriteria;
		break;
	case CFilterItem::eSIEVE:
		paneid = paneid_SearchCriteriaS;
		break;
	}
	CSearchCriteria* criteria = (CSearchCriteria*) UReanimator::ReadObjects('PPob', paneid);
	criteria->FinishCreate();
	criteria->SetRules(mRules);
	if (spec)
		criteria->SetSearchItem(spec);

	// Get last view in criteria bottom
	SPoint32 new_pos = {mTopLevel ? 0 : cCriteriaHOffset, cCriteriaVInitOffset};
	if (mCriteria.GetCount() > 0)
	{
		// Previous bottom is no longer the bottom
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteria[mCriteria.GetCount()]);
		prev->SetBottom(false);

		// Change current to add Or/And menu
		criteria->ShowOrAnd(true);
		criteria->SetOr(use_or);
		criteria->SetBottom(true);

		// Now get position
		SDimension16 prev_size;
		prev->GetFrameSize(prev_size);
		SPoint32 prev_pos32;
		prev->GetFrameLocation(prev_pos32);
		Point prev_pos = {prev_pos32.v, prev_pos32.h};
		PortToLocalPoint(prev_pos);
		new_pos.v = prev_pos.v + prev_size.height;
	}
	else
	{
		criteria->ShowOrAnd(false);
		criteria->SetTop(true);
		criteria->SetBottom(true);
	}

	// Put inside panel
	mCriteria.AddItem(criteria);
	criteria->PutInside(this);

	// Now adjust sizes
	SDimension16 size;
	criteria->GetFrameSize(size);
	
	// Resize groups so that width first inside criteria
	SDimension16 gsize;
	GetFrameSize(gsize);
	criteria->ResizeFrameBy(gsize.width - cCriteriaHWidthAdjust - (mTopLevel ? 0 : cCriteriaHOffset) - size.width, 0, false);
	
	// Do size change
	ResizeFrameBy(0, size.height, true);
	unsigned long change = size.height;
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);

	// Position new sub-panel
	criteria->PlaceInSuperFrameAt(new_pos.h, new_pos.v, false);
	criteria->Show();
	criteria->Add_Listener(this);

	// Do button state
	if (!mTopLevel && (mCriteria.GetCount() > 1))
		mFewerBtn->Show();
}

void CSearchCriteriaContainer::RemoveCriteria()
{
	// Get last view in criteria
	LPane* criteria = mCriteria[mCriteria.GetCount()];
	criteria->PutInside(NULL);

	// Now adjust sizes
	SDimension16 size;
	criteria->GetFrameSize(size);
	
	// Do size change
	ResizeFrameBy(0, -size.height, true);
	unsigned long change = -size.height;
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);

	// Now delete the pane
	delete criteria;
	mCriteria.RemoveItemsAt(1, (ArrayIndexT) mCriteria.GetCount());

	// Make sure new bottom is set
	if (mCriteria.GetCount() > 1)
	{
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteria[mCriteria.GetCount()]);
		prev->SetBottom(true);
	}

	// Do button state
	if (!mTopLevel && (mCriteria.GetCount() < 2))
		mFewerBtn->Hide();
}

void CSearchCriteriaContainer::RemoveAllCriteria()
{
	while(mCriteria.GetCount() > 0)
		RemoveCriteria();
}

void CSearchCriteriaContainer::RecalcLayout()
{
	SPoint32 superFrameLoc;
	GetFrameLocation(superFrameLoc);

	// Reposition each criteria
	long num = mCriteria.GetCount();
	unsigned long pos = cCriteriaVInitOffset;
	for(long i = 1; i <= num; i++)
	{
		LPane* view = mCriteria[i];
		
		// Adjust location of view to new position
		SPoint32 childFrameLoc;
		view->GetFrameLocation(childFrameLoc);
		view->MoveBy(0, pos - (childFrameLoc.v - superFrameLoc.v), false);

		// Set position for next view
		SDimension16 size;
		view->GetFrameSize(size);
		pos += size.height;
	}
}

#pragma mark ____________________________Build Search

CSearchItem* CSearchCriteriaContainer::ConstructSearch() const
{
	CSearchItem* result = NULL;

	// Count items
	long num = mCriteria.GetCount();
	if (num == 1)
	{
		result = static_cast<const CSearchCriteria*>(mCriteria[num])->GetSearchItem();
	}
	else
	{
		// Create flat list of all items
		CSearchItemList flat_list;
		for(long i = 1; i <= num; i++)
		{
			// Insert OR/AND operator for items after the first one
			if (i > 1)
				flat_list.push_back(new CSearchItem(static_cast<const CSearchCriteria*>(mCriteria[i])->IsOr() ? CSearchItem::eOr : CSearchItem::eAnd, new CSearchItemList));
			flat_list.push_back(static_cast<const CSearchCriteria*>(mCriteria[i])->GetSearchItem());
		}

		// Pass 1: Merge all AND pairs into one
		for(CSearchItemList::iterator iter = flat_list.begin(); iter != flat_list.end(); iter++)
		{
			// Look for an AND operation
			if ((*iter)->GetType() == CSearchItem::eAnd)
			{
				// Get item before and after the AND operator
				CSearchItemList::iterator prev = iter - 1;
				CSearchItemList::iterator next = iter + 1;
				
				// Check to see if previous is an AND in which case we merge into that
				if ((*prev)->GetType() == CSearchItem::eAnd)
				{
					// Add next item into previous which is an AND
					CSearchItemList* and_list = const_cast<CSearchItemList*>(static_cast<const CSearchItemList*>((*prev)->GetData()));
					and_list->push_back(new CSearchItem(**next));
					
					// Delete the current item and the next item
					flat_list.erase(iter, next + 1);
					
					// Adjust iter to previous item so it will point to the new item after cycling through the loop
					iter = prev;
				}
				else
				{
					// Add previous and next to the AND list
					CSearchItemList* and_list = const_cast<CSearchItemList*>(static_cast<const CSearchItemList*>((*iter)->GetData()));
					and_list->push_back(new CSearchItem(**prev));
					and_list->push_back(new CSearchItem(**next));
					
					// Delete the next item
					flat_list.erase(next, next + 1);
					
					// Delete the prev item
					flat_list.erase(prev, prev + 1);
					
					// Adjust iter to previous item so it will point to the new item after cycling through the loop
					iter = prev;
				}
			}
		}

		// Pass 2: Merge all OR pairs into one
		for(CSearchItemList::iterator iter = flat_list.begin(); iter != flat_list.end(); iter++)
		{
			// Look for an OR operation
			if ((*iter)->GetType() == CSearchItem::eOr)
			{
				// Get item before and after the OR operator
				CSearchItemList::iterator prev = iter - 1;
				CSearchItemList::iterator next = iter + 1;
				
				// Check to see if previous is an OR in which case we merge into that
				if ((*prev)->GetType() == CSearchItem::eOr)
				{
					// Add next item into previous which is an OR
					CSearchItemList* and_list = const_cast<CSearchItemList*>(static_cast<const CSearchItemList*>((*prev)->GetData()));
					and_list->push_back(new CSearchItem(**next));
					
					// Delete the current item and the next item
					flat_list.erase(iter, next + 1);
					
					// Adjust iter to previous item so it will point to the new item after cycling through the loop
					iter = prev;
				}
				else
				{
					// Add previous and next to the AND list
					CSearchItemList* and_list = const_cast<CSearchItemList*>(static_cast<const CSearchItemList*>((*iter)->GetData()));
					and_list->push_back(new CSearchItem(**prev));
					and_list->push_back(new CSearchItem(**next));
					
					// Delete the next item
					flat_list.erase(next, next + 1);
					
					// Delete the prev item
					flat_list.erase(prev, prev + 1);
					
					// Adjust iter to previous item so it will point to the new item after cycling through the loop
					iter = prev;
				}
			}
		}

		// We should now have one item left! Return it.
		CSearchItem* generated = flat_list.front();
		*flat_list.begin() = NULL;
		
		result = generated;
	}
	
	// Now create the group item if not top level
	if (mTopLevel)
		return result;
	else
		return new CSearchItem(CSearchItem::eGroup, result);
}
