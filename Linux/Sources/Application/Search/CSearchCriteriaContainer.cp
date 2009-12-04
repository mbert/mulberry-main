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


// CSearchCriteriaContainer.cp : implementation of the CSearchCriteriaContainer class
//

#include "CSearchCriteriaContainer.h"

#include "CMulberryApp.h"
#include "CSearchCriteriaLocal.h"
#include "CSearchCriteriaSIEVE.h"
#include "CSearchItem.h"
#include "CWindow.h"

#include <JXEngravedRect.h>
#include <JXTextButton.h>

#include <algorithm>
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CSearchCriteriaContainer

/////////////////////////////////////////////////////////////////////////////
// CSearchCriteriaContainer construction/destruction

CSearchCriteriaContainer::CSearchCriteriaContainer(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mFilterType = CFilterItem::eLocal;
	mTopLevel = false;
	mRules = false;
}

CSearchCriteriaContainer::~CSearchCriteriaContainer()
{
	// Always delete criteria items
	for(CCriteriaBaseList::iterator iter = mCriteriaItems.begin(); iter != mCriteriaItems.end(); iter++)
		delete *iter;
}

void CSearchCriteriaContainer::OnCreate()
{
// begin JXLayout1

    mBorder =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 496,45);
    assert( mBorder != NULL );

    mMoreBtn =
        new JXTextButton("More Choices", mBorder,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,15, 105,25);
    assert( mMoreBtn != NULL );

    mFewerBtn =
        new JXTextButton("Fewer Choices", mBorder,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 125,15, 105,25);
    assert( mFewerBtn != NULL );

    mClearBtn =
        new JXTextButton("Reset", mBorder,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 416,15, 70,25);
    assert( mClearBtn != NULL );

// end JXLayout1

	// Listen to UI items
	ListenTo(mMoreBtn);
	ListenTo(mFewerBtn);
	ListenTo(mClearBtn);

	if (mTopLevel)
	{
		mBorder->Hide();
		
		JRect size = mMoreBtn->GetFrame();
		AdjustSize(0, - size.height() - 4);
	}
}

// Respond to clicks in the icon buttons
void CSearchCriteriaContainer::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mMoreBtn)
		{
			OnMore();
			return;
		}
		else if (sender == mFewerBtn)
		{
			OnFewer();
			return;
		}
		else if (sender == mClearBtn)
		{
			OnClear();
			return;
		}
	}

	JXWidgetSet::Receive(sender, message);
}

// Respond to list changes
void CSearchCriteriaContainer::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CSearchCriteria::eBroadcast_SearchCriteriaResized:
		// Change the size of this one
		AdjustSize(0, *reinterpret_cast<unsigned long*>(param));
		RecalcLayout();

		// Inform listeners of size change
		Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, param);
		break;
	default:;
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
	while(mCriteriaItems.size() > 1)
		RemoveCriteria();

	// Reset the first one
	CSearchCriteria* criteria = static_cast<CSearchCriteria*>(mCriteriaItems[0]);
	criteria->SetSearchItem(NULL);
}

#pragma mark ____________________________Criteria Panels

void CSearchCriteriaContainer::InitGroup(CFilterItem::EType type, const CSearchItem* spec)
{
	mFilterType = type;

	// Strip off group
	if ((spec != NULL) && (spec->GetType() == CSearchItem::eGroup))
		InitCriteria(static_cast<const CSearchItem*>(spec->GetData()));
	else
		InitCriteria(NULL);
}

void CSearchCriteriaContainer::InitCriteria(const CSearchItem* spec)
{
	RemoveAllCriteria();

	// Check for an Or
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
const int cCriteriaVInitOffset = 5;
const int cCriteriaVOffset = 0;
const int cCriteriaHeight = 25;
const int cCriteriaWidth = 496;

void CSearchCriteriaContainer::AddCriteria(const CSearchItem* spec, bool use_or)
{
	// Create a new search criteria panel
	JRect r;
	r.left = 0;
	r.right = cCriteriaWidth;
	r.top = 0;
	r.bottom = cCriteriaHeight;
	CSearchCriteria* criteria = NULL;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		criteria = new CSearchCriteriaLocal(this, JXWidget::kHElastic, JXWidget::kFixedTop, r.left, r.top, cCriteriaWidth, cCriteriaHeight);
		break;
	case CFilterItem::eSIEVE:
		criteria = new CSearchCriteriaSIEVE(this, JXWidget::kHElastic, JXWidget::kFixedTop, r.left, r.top, cCriteriaWidth, cCriteriaHeight);
		break;
	}
	
	// Create actual items
	criteria->OnCreate();
	criteria->SetRules(mRules);
	
	// Set any input spec
	if (spec)
		criteria->SetSearchItem(spec);

	// Get last view in criteria bottom
	JPoint new_pos(mTopLevel ? 0 : cCriteriaHOffset, cCriteriaVInitOffset);
	if (mCriteriaItems.size())
	{
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteriaItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		criteria->ShowOrAnd(true);
		criteria->SetOr(use_or);
		criteria->SetBottom(true);

		// Now get position
		JRect prev_rect = prev->GetFrame();
		new_pos.y = prev_rect.bottom;
	}
	else
	{
		criteria->ShowOrAnd(false);
		criteria->SetTop(true);
		criteria->SetBottom(true);
	}

	// Add to list
	mCriteriaItems.push_back(criteria);

	// Now adjust sizes
	JRect size = criteria->GetFrame();

	// Adjust size to actual width of parent
	JRect gsize = GetAperture();
	criteria->AdjustSize(gsize.width() - cCriteriaHWidthAdjust - (mTopLevel ? 0 : cCriteriaHOffset) - size.width(), 0);
	
	// Do size change
	AdjustSize(0, size.height());
	unsigned long change = size.height();
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);
	
	criteria->Place(new_pos.x, new_pos.y);
	criteria->Show();
	criteria->Add_Listener(this);

	// Do button state
	if (!mTopLevel && (mCriteriaItems.size() > 1))
		mFewerBtn->Show();
}

void CSearchCriteriaContainer::RemoveCriteria(unsigned long num)
{
	JCoordinate adjust_height = 0;
	for(unsigned long i = 1; i <= num; i++)
	{
		// Get last view in criteria
		JXWidget* criteria = mCriteriaItems.back();
		mCriteriaItems.pop_back();
		criteria->Hide();

		// Get size for adjustment
		JRect size = criteria->GetFrame();
		adjust_height += size.height();

		// Now delete the pane
		delete criteria;
	}

	// Do size change
	AdjustSize(0, -adjust_height);
	unsigned long change = -adjust_height;
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);

	// Set up/down button state
	if (mCriteriaItems.size())
	{
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteriaItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (!mTopLevel && (mCriteriaItems.size() < 2))
		mFewerBtn->Hide();
}

void CSearchCriteriaContainer::RemoveAllCriteria()
{
	RemoveCriteria(mCriteriaItems.size());
}

void CSearchCriteriaContainer::RecalcLayout()
{
	// Reposition each criteria
	long num = mCriteriaItems.size();
	if (num)
	{
		JRect rect = mCriteriaItems[0]->GetFrame();
		unsigned long pos = rect.top;
		for(long i = 0; i < num; i++)
		{
			JXWidget* view = mCriteriaItems[i];
			
			// Adjust location of view to new position
			rect = view->GetFrame();
			view->Move(0, pos - rect.top);

			// Set position for next view
			pos += rect.height();
		}
	}
}

void CSearchCriteriaContainer::SelectNextCriteria(CSearchCriteria* previous)
{
	// Only possible if more than one
	if (mCriteriaItems.size() > 1)
	{
		// Find current
		CCriteriaBaseList::iterator found = std::find(mCriteriaItems.begin(), mCriteriaItems.end(), previous);
		if (found != mCriteriaItems.end())
		{
			// Bump to next one (may need to go back to first if at end)
			found++;
			if (found == mCriteriaItems.end())
				found = mCriteriaItems.begin();
			
			// Now focus on new one
			//(*found)->SetFocus();
		}
	}
	//else if (mCriteriaItems.size())
	//	mCriteriaItems.front()->SetFocus();
}

#pragma mark ____________________________Build Search

CSearchItem* CSearchCriteriaContainer::ConstructSearch() const
{
	CSearchItem* result = NULL;

	// Count items
	long num = mCriteriaItems.size();
	if (num == 1)
	{
		result = static_cast<const CSearchCriteria*>(mCriteriaItems[0])->GetSearchItem();
	}
	else
	{
		// Create flat list of all items
		CSearchItemList flat_list;
		for(long i = 0; i < num; i++)
		{
			// Insert OR/AND operator for items after the first one
			if (i > 0)
				flat_list.push_back(new CSearchItem(static_cast<const CSearchCriteria*>(mCriteriaItems[i])->IsOr() ? CSearchItem::eOr : CSearchItem::eAnd, new CSearchItemList));
			flat_list.push_back(static_cast<const CSearchCriteria*>(mCriteriaItems[i])->GetSearchItem());
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
