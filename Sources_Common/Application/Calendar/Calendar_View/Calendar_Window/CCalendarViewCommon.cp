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

#include "CCalendarView.h"

#include "CCalendarStoreView.h"
#include "CCalendarWindow.h"
#include "CDayWeekView.h"
#include "CEventPreview.h"
#include "CFreeBusyView.h"
#include "CMonthView.h"
#include "CPreferences.h"
#include "CToDoView.h"
#include "CYearView.h"

#include "CICalendarManager.h"
#include "CICalendarVEvent.h"
#include "CCalendarStoreManager.h"

#include <algorithm>

// Check for window
bool CCalendarView::ViewExists(const CCalendarView* view)
{
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	CCalendarViewList::iterator found = std::find(sCalendarViews->begin(), sCalendarViews->end(), view);
	return found != sCalendarViews->end();
}

CCalendarWindow* CCalendarView::GetCalendarWindow(void) const
{
	return !Is3Pane() ? static_cast<CCalendarWindow*>(GetOwningWindow()) : NULL;
}

void CCalendarView::SetCalendar(iCal::CICalendar* calendar)
{
	mCalendar = calendar;
	if (mCalendar != NULL)
		mCalendar->Add_Listener(this);
	
	// Force reset of entire view
	FullReset();
}

void CCalendarView::SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date)
{
	ResetView(NCalendarView::eViewFreeBusy, NULL);
	static_cast<CFreeBusyView*>(mCurrentView)->SetFreeBusy(calref, id, organizer, attendees, date);
}

void CCalendarView::SetPreview(CEventPreview* preview)
{
	mPreview = preview;
}

void CCalendarView::SetUsePreview(bool use_view)
{
	// Only if different
	if (mUsePreview == use_view)
		return;

	// Clear preview pane if its being hidden
	if (mUsePreview && (GetPreview() != NULL))
	{
		GetPreview()->SetComponent(iCal::CICalendarComponentExpandedShared());
	}

	mUsePreview = use_view;
}

void CCalendarView::ResetAll()
{
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	for(CCalendarViewList::iterator iter = sCalendarViews->begin(); iter != sCalendarViews->end(); iter++)
		(*iter)->FullReset();
}

void CCalendarView::EventsChangedAll()
{
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	for(CCalendarViewList::iterator iter = sCalendarViews->begin(); iter != sCalendarViews->end(); iter++)
		(*iter)->EventsChanged();
}

void CCalendarView::EventChangedAll(iCal::CICalendarVEvent* vevent)
{
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	for(CCalendarViewList::iterator iter = sCalendarViews->begin(); iter != sCalendarViews->end(); iter++)
		(*iter)->EventChanged(vevent);
}

void CCalendarView::ToDosChangedAll()
{
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	for(CCalendarViewList::iterator iter = sCalendarViews->begin(); iter != sCalendarViews->end(); iter++)
		(*iter)->ToDosChanged();
}

cdstring CCalendarView::GetTitle() const
{
	if (mCurrentView)
		return mCurrentView->GetTitle();
	else
		return cdstring::null_str;
}

void CCalendarView::InitView()
{
	// Reset window state
	ResetState(false);
	
	// Now reset all the calendar/task views
	FullReset();
}

void CCalendarView::FullReset()
{
	if (mCurrentView)
		mCurrentView->ResetDate();
	if (mToDoView)
		mToDoView->ResetDate();
	
	// Also do full refresh to ensure title tables etc get redrawn with possibly new title
	FRAMEWORK_REFRESH_WINDOW(this);
}

void CCalendarView::EventsChanged()
{
	if (mCurrentView)
		mCurrentView->ResetDate();
}

void CCalendarView::EventChanged(iCal::CICalendarVEvent* vevent)
{
	// Only bother if calendar matches
	if (GetCalendar() && (GetCalendar()->GetRef() != vevent->GetCalendar()))
		return;

	if (mCurrentView)
		mCurrentView->SetDate(vevent->GetStart());
}

void CCalendarView::ToDosChanged()
{
	if (mToDoView)
		mToDoView->ResetDate();
}

void CCalendarView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewActivate, this);
		break;
	case CTableDrag::eBroadcast_Deactivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewDeactivate, this);
		break;
	case CTableDrag::eBroadcast_SelectionChanged:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewSelectionChanged, this);
		break;

	case CCalendarViewBase::eBroadcast_ViewChanged:
		// Broadcast change to listeners
		Broadcast_Message(eBroadcast_ViewChanged, this);
		break;

	case CCalendarViewBase::eBroadcast_DblClkToMonth:
	{
		iCal::CICalendarDateTime* dt = static_cast<iCal::CICalendarDateTime*>(param);
		ResetView(NCalendarView::eViewMonth, dt);
		break;
	}
	case CCalendarViewBase::eBroadcast_DblClkToDay:
	{
		// Only both if not already set to single day view
		if (mViewType != NCalendarView::eViewDay)
		{
			iCal::CICalendarDateTime* dt = static_cast<iCal::CICalendarDateTime*>(param);
			ResetView(NCalendarView::eViewDay, dt);
		}
		break;
	}

	case iCal::CICalendar::eBroadcast_Closed:
	{
		// Clear out single calendar
		if (mCalendar != NULL)
			mCalendar = NULL;
		
		// Force reset of entire view
		FullReset();
		break;
	}

	case calstore::CCalendarStoreManager::eBroadcast_SubscribeNode:
	case calstore::CCalendarStoreManager::eBroadcast_UnsubscribeNode:
		// If this is the view showing subscribed items do a full reset
		if (GetCalendar() == NULL)
			FullReset();
		break;

	case calstore::CCalendarStoreManager::eBroadcast_ChangedNode:
		{
			calstore::CCalendarStoreNode* node = static_cast<calstore::CCalendarStoreNode*>(param);
			if (node != NULL)
			{
				// Check for subscribed
				if (GetCalendar() == NULL)
				{
					// Do full reset of subscribed view
					if (node->IsSubscribed())
						FullReset();
				}
				else
				{
					// Look for specific calendar and reset that
					iCal::CICalendar* cal = node->GetCalendar();
					if (cal == GetCalendar())
						FullReset();
				}
			}
		}
		break;
	}
}

void CCalendarView::SetDisplay(uint32_t index, bool show)
{
#ifdef _TODO
	if (calstore::CCalendarStoreManager::sCalendarStoreManager == NULL)
		return;

	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (index >= cals.size())
		return;
	
	calstore::CCalendarStoreNode* node = cals.at(index);
	if (node->IsActive() ^ show)
	{
		if (show)
			calstore::CCalendarStoreManager::sCalendarStoreManager->ActivateNode(node);
		else
			calstore::CCalendarStoreManager::sCalendarStoreManager->DeactivateNode(node);
		
		// Refresh views
		if (mCurrentView != NULL)
			mCurrentView->ResetDate();
		if (mToDoView != NULL)
			mToDoView->ResetDate();
	}
#endif
}

void CCalendarView::OnDayBtn()
{
	ResetView(NCalendarView::eViewDay);
}

void CCalendarView::OnWorkWeekBtn()
{
	ResetView(NCalendarView::eViewWorkWeek);
}

void CCalendarView::OnWeekBtn()
{
	ResetView(NCalendarView::eViewWeek);
}

void CCalendarView::OnMonthBtn()
{
	ResetView(NCalendarView::eViewMonth);
}

void CCalendarView::OnYearBtn()
{
	ResetView(NCalendarView::eViewYear);
}

void CCalendarView::OnSummaryBtn()
{
	ResetView(NCalendarView::eViewSummary);
}

void CCalendarView::OnToDoBtn()
{
	ShowToDo(!mShowToDo);
}

void CCalendarView::OnNewToDoBtn()
{
	// Make sure to do view is visible
	if (!mShowToDo)
		ShowToDo(true);
	
	// Now get view to create it
	static_cast<CToDoView*>(mToDoView)->OnNewToDo();
}

void CCalendarView::OnFileSave()
{
	// Write out modified calendars
	calstore::CCalendarStoreManager::sCalendarStoreManager->SaveAllCalendars();
}

void CCalendarView::OnCheckCalendar()
{
	// If no selection, check all subscribed calendars
	if (mSingleCalendar)
	{
		calstore::CCalendarStoreNode* node = const_cast<calstore::CCalendarStoreNode*>(calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(GetCalendar()));
		node->GetProtocol()->CheckCalendar(*node, *node->GetCalendar());
	}
	else
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			calstore::CCalendarStoreNode* node = const_cast<calstore::CCalendarStoreNode*>(calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(*iter));
			node->GetProtocol()->CheckCalendar(*node, *node->GetCalendar());
		}
	}

	// Reset all views
	CCalendarView::ResetAll();
}
