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
#include "CMulberryCommon.h"
#include "CSearchCriteriaLocal.h"
#include "CSearchCriteriaSIEVE.h"
#include "CSearchItem.h"

/////////////////////////////////////////////////////////////////////////////
// CSearchCriteriaContainer

BEGIN_MESSAGE_MAP(CSearchCriteriaContainer, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDC_SEARCH_MORE, OnMore)
	ON_COMMAND(IDC_SEARCH_FEWER, OnFewer)
	ON_COMMAND(IDC_SEARCH_CLEAR, OnClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchCriteriaContainer construction/destruction

CSearchCriteriaContainer::CSearchCriteriaContainer()
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

// Get details of sub-panes
BOOL CSearchCriteriaContainer::Create(const CRect& rect, CWnd* pParentWnd)
{
	return CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, rect, pParentWnd, IDC_STATIC);
}

const int cMoreBtnWidth = 80;
const int cMoreBtnHeight = 24;
const int cMoreBtnHOffset = 8;
const int cMoreBtnVOffset = 4;

const int cFewerBtnWidth = 80;
const int cFewerBtnHeight = 24;
const int cFewerBtnHOffset = cMoreBtnHOffset + cMoreBtnWidth + 8;
const int cFewerBtnVOffset = cMoreBtnVOffset;

const int cClearBtnWidth = 50;
const int cClearBtnHeight = 24;
const int cClearBtnHOffset = cMoreBtnHOffset + cClearBtnWidth + 8;
const int cClearBtnVOffset = cMoreBtnVOffset;

int CSearchCriteriaContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// need this to ensure tabbing into criteria panels
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);
	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Search styles popup
	CRect r;
	r = CRect(0, 0, width, height);
	CString s;
	mBorder.Create(s, WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME, r, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mBorder, CWndAlignment::eAlign_WidthHeight));

	s.LoadString(IDS_SEARCH_MOREBTN);
	r = CRect(cMoreBtnHOffset, height - cMoreBtnVOffset - cMoreBtnHeight, cMoreBtnHOffset + cMoreBtnWidth + 2*large_offset, height - cMoreBtnVOffset);
	mMoreBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, this, IDC_SEARCH_MORE);
	mMoreBtn.SetFont(CMulberryApp::sAppFont);
	AddAlignment(new CWndAlignment(&mMoreBtn, CWndAlignment::eAlign_BottomLeft));

	s.LoadString(IDS_SEARCH_FEWERBTN);
	r = CRect(cFewerBtnHOffset + 2*large_offset, height - cFewerBtnVOffset - cFewerBtnHeight, cFewerBtnHOffset + cFewerBtnWidth + 4*large_offset, height - cFewerBtnVOffset);
	mFewerBtn.Create(s, WS_CHILD | WS_TABSTOP, r, this, IDC_SEARCH_FEWER);
	mFewerBtn.SetFont(CMulberryApp::sAppFont);
	AddAlignment(new CWndAlignment(&mFewerBtn, CWndAlignment::eAlign_BottomLeft));

	s.LoadString(IDS_SEARCH_CLEARBTN);
	r = CRect(width - cClearBtnHOffset - 2*large_offset, height - cClearBtnVOffset - cClearBtnHeight, width - cClearBtnHOffset + cClearBtnWidth - large_offset, height - cClearBtnVOffset);
	mClearBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, this, IDC_SEARCH_CLEAR);
	mClearBtn.SetFont(CMulberryApp::sAppFont);
	AddAlignment(new CWndAlignment(&mClearBtn, CWndAlignment::eAlign_BottomRight));

	if (mTopLevel)
	{
		mBorder.ShowWindow(SW_HIDE);
		mMoreBtn.ShowWindow(SW_HIDE);
		mFewerBtn.ShowWindow(SW_HIDE);
		mClearBtn.ShowWindow(SW_HIDE);
		
		::ResizeWindowBy(this, 0, - cMoreBtnVOffset - cMoreBtnHeight, false);
	}

	return 0;
}

// Resize sub-views
void CSearchCriteriaContainer::OnSize(UINT nType, int cx, int cy)
{
	// Do inherited
	CWnd::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

// Respond to list changes
void CSearchCriteriaContainer::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CSearchCriteria::eBroadcast_SearchCriteriaResized:
		// Change the size of this one
		::ResizeWindowBy(this, 0, *reinterpret_cast<unsigned long*>(param), true);
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
	while(mCriteriaItems.size() >1)
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

bool CSearchCriteriaContainer::SetInitialFocus()
{
	for(CCriteriaBaseList::iterator iter = mCriteriaItems.begin(); iter != mCriteriaItems.end(); iter++)
	{
		if (static_cast<CSearchCriteria*>(*iter)->SetInitialFocus())
			return true;
	}
	
	return false;
}

const int cCriteriaWidth = 460;
const int cCriteriaHeight = 26;
const int cCriteriaHOffset = 4;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 12;

void CSearchCriteriaContainer::AddCriteria(const CSearchItem* spec, bool use_or)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;
	const int cCriteriaVInitOffset1 = cCriteriaVInitOffset + small_offset;
	const int cCriteriaHeight1 = cCriteriaHeight + small_offset;

	// Create a new search criteria panel
	CRect r;
	r.left = 0;
	r.right = cCriteriaWidth;
	r.top = 0;
	r.bottom = cCriteriaHeight1;
	CSearchCriteria* criteria = NULL;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		criteria = new CSearchCriteriaLocal;
		break;
	case CFilterItem::eSIEVE:
		criteria = new CSearchCriteriaSIEVE;
		break;
	}
	criteria->Create(r, this);
	criteria->SetRules(mRules);
	AddAlignment(new CWndAlignment(criteria, CWndAlignment::eAlign_TopWidth));
	if (spec)
		criteria->SetSearchItem(spec);

	// Get last view in criteria bottom
	POINT new_pos = {mTopLevel ? 0 : cCriteriaHOffset, cCriteriaVInitOffset1};
	if (mCriteriaItems.size())
	{
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteriaItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		criteria->ShowOrAnd(true);
		criteria->SetOr(use_or);
		criteria->SetBottom(true);

		// Now get position
		CRect prev_rect;
		prev->GetWindowRect(prev_rect);
		ScreenToClient(prev_rect);
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
	CRect size;
	criteria->GetWindowRect(size);

	// Resize groups so that width first inside criteria
	CRect gsize;
	GetWindowRect(gsize);
	::ResizeWindowBy(criteria, gsize.Width() - cCriteriaHWidthAdjust - (mTopLevel ? 0 : cCriteriaHOffset) - size.Width(), 0, false);

	// Do size change
	::ResizeWindowBy(this, 0, size.Height(), true);
	unsigned long change = size.Height();
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);
	
	// Position new sub-panel, show and focus it
	criteria->SetWindowPos(nil, new_pos.x, new_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	criteria->ShowWindow(SW_SHOW);
	criteria->SetFocus();
	criteria->Add_Listener(this);
	
	// Do button state
	if (!mTopLevel && (mCriteriaItems.size() > 1))
		mFewerBtn.ShowWindow(SW_SHOW);
}

void CSearchCriteriaContainer::RemoveCriteria()
{
	// Get last view in criteria
	CWnd* criteria = mCriteriaItems.back();
	criteria->ShowWindow(SW_HIDE);

	// Now adjust sizes
	CRect size;
	criteria->GetWindowRect(size);

	// Do size change
	::ResizeWindowBy(this, 0, -size.Height(), true);
	unsigned long change = -size.Height();
	Broadcast_Message(eBroadcast_SearchCriteriaContainerResized, &change);

	// Now delete the pane
	criteria->DestroyWindow();
	delete criteria;
	mCriteriaItems.pop_back();
	
	if (mCriteriaItems.size())
	{
		CSearchCriteria* prev = static_cast<CSearchCriteria*>(mCriteriaItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (!mTopLevel && (mCriteriaItems.size() < 2))
		mFewerBtn.ShowWindow(SW_HIDE);
}

void CSearchCriteriaContainer::RemoveAllCriteria()
{
	while(mCriteriaItems.size() > 0)
		RemoveCriteria();
}

void CSearchCriteriaContainer::RecalcLayout()
{
	// Reposition each criteria
	long num = mCriteriaItems.size();
	if (num)
	{
		CRect rect;
		mCriteriaItems[0]->GetWindowRect(rect);
		unsigned long pos = rect.top;
		for(long i = 0; i < num; i++)
		{
			CWnd* view = mCriteriaItems[i];
			
			// Adjust location of view to new position
			view->GetWindowRect(rect);
			::MoveWindowBy(view, 0, pos - rect.top, false);

			// Set position for next view
			pos += rect.Height();
		}
	}
}

void CSearchCriteriaContainer::SelectNextCriteria(CSearchCriteria* previous)
{
	// Only possible if more than one
	if (mCriteriaItems.size() > 1)
	{
		// Find current
		CCriteriaBaseList::iterator found = ::find(mCriteriaItems.begin(), mCriteriaItems.end(), previous);
		if (found != mCriteriaItems.end())
		{
			// Bump to next one (may need to go back to first if at end)
			found++;
			if (found == mCriteriaItems.end())
				found = mCriteriaItems.begin();
			
			// Now focus on new one
			(*found)->SetFocus();
		}
	}
	else if (mCriteriaItems.size())
		mCriteriaItems.front()->SetFocus();
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
