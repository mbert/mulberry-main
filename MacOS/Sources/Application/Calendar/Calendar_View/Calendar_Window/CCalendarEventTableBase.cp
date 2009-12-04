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
#include "CCalendarViewBase.h"
#include "CCommands.h"
#include "CDataAttachment.h"
#include "CErrorDialog.h"
#include "CPreferences.h"
#include "CNewEventDialog.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"

#include "CICalendarVEvent.h"
#include "CITIPProcessor.h"

#include <algorithm>

// ---------------------------------------------------------------------------
//	CCalendarEventTableBase														  [public]
/**
	Default constructor */

CCalendarEventTableBase::CCalendarEventTableBase(LStream *inStream) :
	CCalendarTableBase(inStream)
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

//	Respond to commands
Boolean CCalendarEventTableBase::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	// Pass these to selected item if present
	case cmd_NewLetter:
	case cmd_ToolbarNewLetter:
	case cmd_ToolbarNewLetterOption:
		OnNewDraft(inCommand == cmd_ToolbarNewLetterOption);
		break;
		
	case cmd_EditCalendarItem:
		OnEditEvent();
		break;

	case cmd_DuplicateCalendarItem:
		OnDuplicateEvent();
		break;

	case cmd_DeleteCalendarItem:
		OnDeleteEvent();
		break;

	case cmd_InviteAttendees:
		OnInviteAttendees();
		break;

	case cmd_ProcessInvitation:
		OnProcessInvitation();
		break;
		
	default:
		cmdHandled = CCalendarTableBase::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarEventTableBase::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_NewLetter:
	case cmd_ToolbarNewLetter:
	case cmd_ToolbarNewLetterOption:
		outEnabled = true;
		break;

	case cmd_EditCalendarItem:
	case cmd_DuplicateCalendarItem:
		// Must have event selected
		outEnabled = mSelectedEvents.size() != 0;
		break;

	case cmd_DeleteCalendarItem:
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

			outEnabled = enabled;
		}
		break;

	case cmd_InviteAttendees:
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

			outEnabled = enabled;
		}
		else
			outEnabled = false;
		break;

	case cmd_ProcessInvitation:
		// Can only send one at a time
		if (mSelectedEvents.size() == 1)
		{
			// Look at all events
			bool enabled = true;
			
			// Get the unqiue original components
			iCal::CICalendarComponentRecurs vevents;
			GetSelectedMasterEvents(vevents);
			
			// Edit each one
			for(iCal::CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
			{
				// See if event is in an Inbox
				iCal::CICalendarRef calref = static_cast<iCal::CICalendarVEvent*>(*iter)->GetCalendar();
				iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
				const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(cal);
				if ((cal == NULL) || cal->IsReadOnly() || (node == NULL) || !(node->IsInbox() || node->CanSchedule()))
				{
					enabled = false;
					break;
				}
			}

			outEnabled = enabled;
		}
		else
			outEnabled = false;
		break;

	default:
		CCalendarTableBase::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

Boolean CCalendarEventTableBase::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Return => edit
	switch(inKeyEvent.message & charCodeMask)
	{
	case char_Tab:
		SelectEvent(!(inKeyEvent.modifiers & shiftKey));
		break;

	case char_Enter:
	case char_Return:
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
		}
		break;
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if (((inKeyEvent.message & keyCodeMask) != vkey_Escape) && (mSelectedEvents.size() != 0))
		{
			OnDeleteEvent();
			return true;
		}
		break;
	default:;
	}
	
	return CCalendarTableBase::HandleKeyPress(inKeyEvent);
}

// Click
void CCalendarEventTableBase::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Make sure the table is the target
	SwitchTarget(this);

	// Remove selection from any selected items
	UnselectAllEvents();

	CCalendarTableBase::ClickSelf(inMouseDown);
}

void CCalendarEventTableBase::ClickEvent(CCalendarEventBase* item, bool shift)
{
	// Make sure the table is the target
	SwitchTarget(this);

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
