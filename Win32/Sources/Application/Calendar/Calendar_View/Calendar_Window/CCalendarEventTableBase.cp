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

#include "CCalendarEventTableBase.h"

#include "CActionManager.h"
#include "CCalendarViewBase.h"
#include "CDataAttachment.h"
#include "CErrorDialog.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include "CICalendarVEvent.h"
#include "CITIPProcessor.h"

#include <algorithm>

BEGIN_MESSAGE_MAP(CCalendarEventTableBase, CCalendarTableBase)
	ON_UPDATE_COMMAND_UI(IDM_FILE_NEW_DRAFT, OnUpdateAlways)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnNewDraft)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_EDITITEM, OnUpdateSelectedEvent)
	ON_COMMAND(IDM_CALENDAR_EDITITEM, OnEditEvent)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_DUPLICATEITEM, OnUpdateSelectedEvent)
	ON_COMMAND(IDM_CALENDAR_DUPLICATEITEM, OnDuplicateEvent)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_DELETEITEM, OnUpdateDeleteEvent)
	ON_COMMAND(IDM_CALENDAR_DELETEITEM, OnDeleteEvent)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_INVITEATTENDEES, OnUpdateInviteAttendees)
	ON_COMMAND(IDM_CALENDAR_INVITEATTENDEES, OnInviteAttendees)

	ON_WM_LBUTTONDOWN()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnNewDraftOption)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CCalendarEventTableBase														  [public]
/**
	Default constructor */

CCalendarEventTableBase::CCalendarEventTableBase()
{
}


// ---------------------------------------------------------------------------
//	~CCalendarEventTableBase														  [public]
/**
	Destructor */

CCalendarEventTableBase::~CCalendarEventTableBase()
{
}

#pragma mark -

void CCalendarEventTableBase::OnUpdateSelectedEvent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mSelectedEvents.size() != 0);
}

void CCalendarEventTableBase::OnUpdateDeleteEvent(CCmdUI* pCmdUI)
{
	// Look at all events - any read-onloy prevent delete
	bool enabled = (mSelectedEvents.size() != 0);

	// Get the unqiue original components
	iCal::CICalendarComponentRecurs vevents;
	GetSelectedMasterEvents(vevents);

	// Edit each one
	for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		iCal::CICalendarRef calref = static_cast<iCal::CICalendarVEvent*>(*iter)->GetCalendar();
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
		if ((cal == NULL) || cal->IsReadOnly())
		{
			enabled = false;
			break;
		}
	}

	pCmdUI->Enable(enabled);
}

void CCalendarEventTableBase::OnUpdateInviteAttendees(CCmdUI* pCmdUI)
{
	// Can only send one at a time
	if (mSelectedEvents.size() == 1)
	{
		// Look at all events
		bool enabled = false;

		// Get the unqiue original components
		iCal::CICalendarComponentRecurs vevents;
		GetSelectedMasterEvents(vevents);

		// Edit each one
		for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
		{
			// See whether iTIP processing is needed for any ATTENDEEs in the component
			if (iCal::CITIPProcessor::NeedsITIPRequest(**iter))
				enabled = true;
		}

		pCmdUI->Enable(enabled);
	}
	else
		pCmdUI->Enable(false);
}

bool CCalendarEventTableBase::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Return => edit
	switch(nChar)
	{
	case VK_TAB:
		SelectEvent(!(::GetKeyState(VK_SHIFT) < 0));
		break;

	case VK_RETURN:
		// Edit event if selected
		if (mSelectedEvents.size() != 0)
		{
			OnEditEvent();
			return true;			
		}
		// Switch to next view
		else if (IsSelectionValid())
		{
			SelectDay();
			return true;
		}
		break;
	case VK_BACK:
	case VK_DELETE:
		OnDeleteEvent();
		return true;
	default:;
	}
	
	return CCalendarTableBase::HandleKeyDown(nChar, nRepCnt, nFlags);
}

// Click
void CCalendarEventTableBase::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Remove selection from any selected items
	UnselectAllEvents();

	CCalendarTableBase::OnLButtonDown(nFlags, point);
}

void CCalendarEventTableBase::ClickEvent(CCalendarEventBase* item, bool shift)
{
	// Make sure the table is the target
	SetFocus();
	SetTarget(this);

	// Always remove table cell selection
	UnselectAllCells();
	
	// If not shifting always remove other selections
	if (shift)
	{
		// Add or remove the chosen item without affecting others
		if (item->IsSelected())
			UnselectEvent(item);
		else
			SelectEvent(item);
	}
	else
	{
		// Remove existing selection
		UnselectAllEvents();

		// Add the new one
		SelectEvent(item);
	}
}

void CCalendarEventTableBase::OnNewDraft()
{
	OnNewDraft(false);
}

void CCalendarEventTableBase::OnNewDraftOption()
{
	OnNewDraft(true);
}
