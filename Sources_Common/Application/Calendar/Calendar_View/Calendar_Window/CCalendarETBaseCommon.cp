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

#include "CCalendarEventTableBase.h"

#include "CActionManager.h"
#include "CAddressList.h"
#include "CCalendarViewBase.h"
#include "CDataAttachment.h"
#include "CErrorDialog.h"
#include "CPreferences.h"
#include "CNewEventDialog.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include "CICalendarVEvent.h"
#include "CITIPProcessor.h"

#include <algorithm>

void CCalendarEventTableBase::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CCalendarEventBase::eBroadcast_ClickEvent:
	{
		// Handle selection of event
		CCalendarEventBase::SClickEvent_Message* click = static_cast<CCalendarEventBase::SClickEvent_Message*>(param);
		ClickEvent(click->GetItem(), click->GetShift());
		break;
	}
	case CCalendarEventBase::eBroadcast_EditEvent:
		// Handle edit of one event
		OnEditOneEvent(static_cast<CCalendarEventBase*>(param));
		break;
	}
}

void CCalendarEventTableBase::SelectEvent(CCalendarEventBase* item)
{
	// Check whether unqiue selection
	bool first_selection = (mSelectedEvents.size() == 0);

	// Add the new one
	item->Select(true);
	mSelectedEvents.push_back(item);
	
	// Also select links
	CCalendarEventBase* prev = item->GetPreviousLink();
	while(prev)
	{
		prev->Select(true);
		mSelectedEvents.push_back(prev);
		prev = prev->GetPreviousLink();
	}
	
	CCalendarEventBase* next = item->GetNextLink();
	while(next)
	{
		next->Select(true);
		mSelectedEvents.push_back(next);
		next = next->GetNextLink();
	}
	
	// Preview if only one
	if (first_selection)
	{
		// Preview this one
		if (GetCalendarView() != NULL)
			GetCalendarView()->PreviewComponent(item->GetVEvent());
	}
}

void CCalendarEventTableBase::UnselectEvent(CCalendarEventBase* item)
{
	// Add the new one
	item->Select(false);
	mSelectedEvents.erase(std::remove(mSelectedEvents.begin(), mSelectedEvents.end(), item), mSelectedEvents.end());
	
	// Also select links
	CCalendarEventBase* prev = item->GetPreviousLink();
	while(prev)
	{
		prev->Select(false);
		mSelectedEvents.erase(std::remove(mSelectedEvents.begin(), mSelectedEvents.end(), prev), mSelectedEvents.end());
		prev = prev->GetPreviousLink();
	}
	
	CCalendarEventBase* next = item->GetNextLink();
	while(next)
	{
		next->Select(false);
		mSelectedEvents.erase(std::remove(mSelectedEvents.begin(), mSelectedEvents.end(), next), mSelectedEvents.end());
		next = next->GetNextLink();
	}
	
	// Always remove preview
	if (GetCalendarView() != NULL)
		GetCalendarView()->PreviewComponent(iCal::CICalendarComponentExpandedShared());
}

void CCalendarEventTableBase::UnselectAllEvents()
{
	// Remove selection from any selected items
	for(CCalendarEventBaseList::iterator iter = mSelectedEvents.begin(); iter != mSelectedEvents.end(); iter++)
	{
		(*iter)->Select(false);
	}
	mSelectedEvents.clear();

	// Always remove preview
	if (GetCalendarView() != NULL)
		GetCalendarView()->PreviewComponent(iCal::CICalendarComponentExpandedShared());
}

void CCalendarEventTableBase::ClearSelectedEvents()
{
	mSelectedEvents.clear();

	// Always remove preview
	if (GetCalendarView() != NULL)
		GetCalendarView()->PreviewComponent(iCal::CICalendarComponentExpandedShared());
}

// Base class does nothing
void CCalendarEventTableBase::SelectEvent(bool next)
{
	
}

void CCalendarEventTableBase::GetSelectedEvents(iCal::CICalendarExpandedComponents& items)
{
	// Get the unique selected expanded components
	for(CCalendarEventBaseList::iterator iter = mSelectedEvents.begin(); iter != mSelectedEvents.end(); iter++)
	{
		// If the event has no previous link then we add it
		if (((*iter)->GetPreviousLink() == NULL) && ((*iter)->GetVEvent().get() != NULL))
		{
			items.push_back((*iter)->GetVEvent());
		}
	}
}

void CCalendarEventTableBase::GetSelectedMasterEvents(iCal::CICalendarComponentRecurs& vevents)
{
	// Get the unique selected expanded components
	iCal::CICalendarExpandedComponents items;
	GetSelectedEvents(items);

	// Now get the unqiue original components
	for(iCal::CICalendarExpandedComponents::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		const iCal::CICalendarVEvent* vevent = (*iter)->GetTrueMaster<iCal::CICalendarVEvent>();
		iCal::CICalendarComponentRecurs::const_iterator found = std::find(vevents.begin(), vevents.end(), vevent);
		
		if (found == vevents.end())
		{
			vevents.push_back(const_cast<iCal::CICalendarVEvent*>(vevent));
		}
	}

	// Sort the components by dtstart
	std::sort(vevents.begin(), vevents.end(), iCal::CICalendarComponentRecur::sort_by_dtstart);
}

// Base class does nothing
void CCalendarEventTableBase::SelectDay()
{
	
}

void CCalendarEventTableBase::OnNewDraft(bool option)
{
	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);
	
	if (vevents.empty())
	{
		// Do ordinary draft
		CActionManager::NewDraft();
		return;
	}

	// Determine identity from calendar of first event
	const CIdentity* id = NULL;
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(vevents.front()->GetCalendar());
	if (cal != NULL)
		id = calstore::CCalendarStoreManager::sCalendarStoreManager->GetTiedIdentity(cal);

	// Use email address of primary identity as the organiser if no other identity found
	if (id == NULL)
		id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Generate iTIP PUBLISH component for this event
	iCal::CITIPProcessor::PublishEvents(vevents, id);
}

void CCalendarEventTableBase::OnEditEvent()
{
	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);

	// Edit each one
	for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		CNewEventDialog::StartEdit(*static_cast<iCal::CICalendarVEvent*>(*iter), NULL);
	}
}

void CCalendarEventTableBase::OnEditOneEvent(CCalendarEventBase* event)
{
	if (event->GetVEvent().get() == NULL)
		return;

#if 0
	// IMPORTANT the VEVENT object may get deleted if its is a recurrence item,
	// so we must cache the master VEVENT here and use this
	iCal::CICalendarVEvent* master = event->GetVEvent()->GetTrueMaster<iCal::CICalendarVEvent>();

	CNewEventDialog::StartEdit(*master);
#else
	CNewEventDialog::StartEdit(*event->GetVEvent()->GetMaster<iCal::CICalendarVEvent>(), event->GetVEvent().get());
#endif
}

void CCalendarEventTableBase::OnDuplicateEvent()
{
	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);

	// Duplicate each one
	for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		CNewEventDialog::StartDuplicate(*static_cast<iCal::CICalendarVEvent*>(*iter));
	}
}

void CCalendarEventTableBase::OnDeleteEvent()
{
	// Get the unqiue original components
	iCal::CICalendarExpandedComponents vevents;
	GetSelectedEvents(vevents);
	iCal::CICalendarComponentRecurs masters;
	GetSelectedMasterEvents(masters);

	if (vevents.empty() || masters.empty())
		return;

	// Check to see if any expanded events are recurring
	bool is_recur = false;
	for(iCal::CICalendarExpandedComponents::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		if ((*iter)->Recurring())
		{
			is_recur = true;
			break;
		}
	}

	// Do alert about delete
	if (is_recur)
	{
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::DeleteAllEvents",
																		"ErrorDialog::Btn::DeleteThisEvent",
																		"ErrorDialog::Btn::DeleteThisFutureEvent",
																		"ErrorDialog::Btn::Cancel",
																		"ErrorDialog::Text::DeleteRecurEvent", 4);
		
		// Cancel
		if (result == CErrorDialog::eBtn4)
			return;
		
		// When removing all events, iterate over the master list to the expanded list,
		// as remove an expanded item can delete other expanded ones
		if (result == CErrorDialog::eBtn1)	// Delete All Events
		{
			// Remove each one
			for(iCal::CICalendarComponentRecurs::iterator iter = masters.begin(); iter != masters.end(); iter++)
			{
				// Must have calendar
				iCal::CICalendar* calendar = iCal::CICalendar::GetICalendar((*iter)->GetCalendar());
				if (calendar == NULL)
					continue;
				
				// Now remove the master
				calendar->RemoveVEvent(static_cast<iCal::CICalendarVEvent*>(*iter));
			}
		}
		else
		{
			// Remove each one
			for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
			{
				// Must have calendar
				iCal::CICalendar* calendar = iCal::CICalendar::GetICalendar((*iter)->GetTrueMaster<iCal::CICalendarVEvent>()->GetCalendar());
				if (calendar == NULL)
					continue;
			
				// Do chosen delete action
				switch(result)
				{
				case CErrorDialog::eBtn2:	// Delete This Event
					calendar->RemoveRecurringVEvent(*iter, iCal::CICalendar::eRemoveOnlyThis);
					break;
				case CErrorDialog::eBtn3:	// Delete This and Future Event
					calendar->RemoveRecurringVEvent(*iter, iCal::CICalendar::eRemoveThisFuture);
					break;
				default:;
				}
			}
		}
	}
	else
	{
		if (CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
										"ErrorDialog::Btn::Yes",
										"ErrorDialog::Btn::No",
										NULL,
										NULL,
										"ErrorDialog::Text::DeleteEvent") == CErrorDialog::eCancel)
			return;
			
		// Remove each one
		for(iCal::CICalendarComponentRecurs::iterator iter = masters.begin(); iter != masters.end(); iter++)
		{
			// Must have calendar
			iCal::CICalendar* calendar = iCal::CICalendar::GetICalendar((*iter)->GetCalendar());
			if (calendar == NULL)
				continue;
			
			// Now remove the master
			calendar->RemoveVEvent(static_cast<iCal::CICalendarVEvent*>(*iter));
		}
	}
	

	// Do direct call to ListenTo_Message as we cannot broadcast when this object could be deleted
	Broadcast_Message(eBroadcast_CalendarChanged, NULL);
}

void CCalendarEventTableBase::OnInviteAttendees()
{
	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);

	// MUST have only one selected item as iTIP requires that	
	if (vevents.size() != 1)
	{
		return;
	}

	// Get mailto addressees for this component
	CCalendarAddressList addrs;
	cdstring idname;
	iCal::CITIPProcessor::GetRequestDetails(*vevents.front(), addrs, idname);
	const CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(idname);
	
	// Must have some addressees
	if (addrs.size() == 0)
		return;
	
	// Prompt user for each one
	
	// Generate iTIP REQUEST component for this event
	iCal::CITIPProcessor::SendRequest(&addrs, vevents.front(), id);
}

void CCalendarEventTableBase::OnProcessInvitation()
{
	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);
	
	// MUST have only one selected item as iTIP requires that	
	if (vevents.size() != 1)
	{
		return;
	}

	// Make sure we have an X-METHOD property
	if (!vevents.front()->HasProperty(iCal::cICalProperty_X_PRIVATE_METHOD))
	{
		return;
	}

	// Handle iTIP component for this event
	iCal::CITIPProcessor::ProcessCalDAVComponent(vevents.front());
	
	// Do direct call to ListenTo_Message as we cannot broadcast when this object could be deleted
	// We need to do this just in case the selected item got deleted whilst processing
	Broadcast_Message(eBroadcast_CalendarChanged, NULL);
}

