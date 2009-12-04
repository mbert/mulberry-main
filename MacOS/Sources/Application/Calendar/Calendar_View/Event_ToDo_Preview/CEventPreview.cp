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


// Source for CEventPreview class

#include "CEventPreview.h"

#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CTextDisplay.h"
#include "CXStringResources.h"

#include "cdstring.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentRecur.h"
#include "CICalendarLocale.h"
#include "CICalendarVEvent.h"
#include "CICalendarVToDo.h"

#include <strstream>

#include <LPushButton.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEventPreview::CEventPreview()
{
}

// Constructor from stream
CEventPreview::CEventPreview(LStream *inStream)
		: CBaseView(inStream)
{
}

// Default destructor
CEventPreview::~CEventPreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEventPreview::FinishCreateSelf(void)
{
	// Do inherited
	CBaseView::FinishCreateSelf();

	// Get items
	mDetails = (CTextDisplay*) FindPaneByID(paneid_EventPreviewDetails);

	mEditBtn = (LPushButton*) FindPaneByID(paneid_EventPreviewEdit);
	mEditBtn->AddListener(this);
	mEditBtn->Disable();
}

// Respond to clicks in the icon buttons
void CEventPreview::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_EventPreviewEdit:
		Edit();
		break;
	}
}

bool CEventPreview::HasFocus() const
{
	// Switch target to the text (will focus)
	return LCommander::GetTarget() == mDetails;
}

void CEventPreview::Focus()
{
	LCommander::SwitchTarget(mDetails);
}

void CEventPreview::SetComponent(const iCal::CICalendarComponentExpandedShared& comp)
{
	if (mComp.get() != comp.get())
	{
		if (comp.get() != NULL)
		{
			mEditBtn->Enable();
			mDetails->Enable();
			
			mSummary = comp->GetOwner()->GetSummary();
			
			// Determine type
			switch(comp->GetOwner()->GetType())
			{
			case iCal::CICalendarComponent::eVEVENT:
				SetEvent(comp);
				break;
			case iCal::CICalendarComponent::eVTODO:
				SetToDo(comp);
				break;
			default:;
			}
		}
		else
		{
			mEditBtn->Disable();
			mDetails->Disable();
			
			mSummary = cdstring::null_str;
			
			mDetails->SetText(cdstring::null_str);
		}

		mComp = comp;

		// Broadcast change to listeners
		Broadcast_Message(eBroadcast_ViewChanged, this);
	}
}

void CEventPreview::SetEvent(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("EventTip::Summary") << comp->GetMaster<iCal::CICalendarVEvent>()->GetSummary() << std::endl;
	if (comp->GetInstanceStart().IsDateOnly())
	{
		ostr << rsrc::GetString("EventTip::All Day Event") << std::endl;
	}
	else
	{
		ostr << rsrc::GetString("EventTip::Starts on") << comp->GetInstanceStart().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime()) << std::endl;
		ostr << rsrc::GetString("EventTip::Ends on") << comp->GetInstanceEnd().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime()) << std::endl;
	}
	if (!comp->GetMaster<iCal::CICalendarVEvent>()->GetLocation().empty())
		ostr << rsrc::GetString("EventTip::Location") << comp->GetMaster<iCal::CICalendarVEvent>()->GetLocation() << std::endl;
	if (!comp->GetMaster<iCal::CICalendarVEvent>()->GetDescription().empty())
		ostr << rsrc::GetString("EventTip::Description") << comp->GetMaster<iCal::CICalendarVEvent>()->GetDescription() << std::endl;
	
	// Add calendar name if more than one calendar in use
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() > 1)
	{
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(comp->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
		if (cal != NULL)
			ostr << std::endl << rsrc::GetString("EventTip::Calendar") << cal->GetName() << std::endl;
	}

	ostr << std::ends;
	cdstring temp;
	temp.steal(ostr.str());
	
	mDetails->SetText(temp);
}

void CEventPreview::SetToDo(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("ToDoTip::Summary") << comp->GetMaster<iCal::CICalendarVToDo>()->GetSummary() << std::endl;

	if (!comp->GetMaster<iCal::CICalendarVToDo>()->HasEnd())
	{
		ostr << rsrc::GetString("ToDoTip::No due date") << std::endl;
	}
	else
	{
		ostr << rsrc::GetString("ToDoTip::Due on") << comp->GetInstanceEnd().GetAdjustedTime().GetLocaleDate(iCal::CICalendarDateTime::eAbbrevDate) << std::endl;
	}

	// Add calendar name if more than one calendar in use
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() > 1)
	{
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar( comp->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
		if (cal != NULL)
			ostr << std::endl << rsrc::GetString("ToDoTip::Calendar") << cal->GetName() << std::endl;
	}

	ostr << std::ends;
	cdstring temp;
	temp.steal(ostr.str());
	
	mDetails->SetText(temp);
}

bool CEventPreview::HasData() const
{
	return (mComp.get() != NULL);
}

void CEventPreview::Edit()
{
	// Determine type
	switch(mComp->GetOwner()->GetType())
	{
	case iCal::CICalendarComponent::eVEVENT:
		CNewEventDialog::StartEdit(*mComp->GetMaster<iCal::CICalendarVEvent>(), mComp.get());
		break;
	case iCal::CICalendarComponent::eVTODO:
		CNewToDoDialog::StartEdit(*mComp->GetMaster<iCal::CICalendarVToDo>(), mComp.get());
		break;
	default:;
	}
}
