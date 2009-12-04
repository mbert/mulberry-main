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

#include "CMulberryApp.h"
#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CStringUtils.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "cdstring.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentRecur.h"
#include "CICalendarLocale.h"
#include "CICalendarVEvent.h"
#include "CICalendarVToDo.h"

#include <strstream.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEventPreview::CEventPreview()
{
}

// Default destructor
CEventPreview::~CEventPreview()
{
}

BEGIN_MESSAGE_MAP(CEventPreview, CBaseView)
	//{{AFX_MSG_MAP(CEventPreview)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_EVENTPREVIEW_EDIT, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

int CEventPreview::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	const int cButtonHeight = 16;
	const int cButtonWidth = 48;
	const int cButtonHOffset = 8;
	const int cButtonVOffset = 4;
	const int cButtonAreaHeight = cButtonHeight + 2 * cButtonVOffset;

	// Text
	CRect r(0, 0, width, height - cButtonAreaHeight);
	mDetails.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL, r, this, IDC_STATIC);
	mDetails.SetFont(CMulberryApp::sAppFont);
	mDetails.SetReadOnly(true);
	mDetails.EnableWindow(false);
	AddAlignment(new CWndAlignment(&mDetails, CWndAlignment::eAlign_WidthHeight));

	r = CRect(width - cButtonHOffset - cButtonWidth, height - cButtonVOffset - cButtonHeight, width - cButtonHOffset, height - cButtonVOffset);
	mEditBtn.Create(rsrc::GetString("UI::Calendar::EventEdit").win_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, this, IDC_EVENTPREVIEW_EDIT);
	mEditBtn.SetFont(CMulberryApp::sAppFont);
	mEditBtn.EnableWindow(false);
	AddAlignment(new CWndAlignment(&mEditBtn, CWndAlignment::eAlign_BottomRight));
	
	return 0;
}


bool CEventPreview::HasFocus() const
{
	// Switch target to the text (will focus)
	return false;
}

void CEventPreview::Focus()
{
	if (IsWindowVisible())
		mDetails.SetFocus();
}

void CEventPreview::SetComponent(const iCal::CICalendarComponentExpandedShared& comp)
{
	if (mComp.get() != comp.get())
	{
		if (comp.get() != NULL)
		{
			mEditBtn.EnableWindow(true);
			mDetails.EnableWindow(true);
			
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
			mEditBtn.EnableWindow(false);
			mDetails.EnableWindow(false);
			
			mSummary = cdstring::null_str;
			
			mDetails.SetText(cdstring::null_str);
		}

		mComp = comp;

		// Broadcast change to listeners
		Broadcast_Message(eBroadcast_ViewChanged, this);
	}
}

void CEventPreview::SetEvent(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Setup a help tag;
	ostrstream ostr;
	ostr << rsrc::GetString("EventTip::Summary") << comp->GetMaster<iCal::CICalendarVEvent>()->GetSummary() << endl;
	if (comp->GetInstanceStart().IsDateOnly())
	{
		ostr << rsrc::GetString("EventTip::All Day Event") << endl;
	}
	else
	{
		ostr << rsrc::GetString("EventTip::Starts on") << comp->GetInstanceStart().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime()) << endl;
		ostr << rsrc::GetString("EventTip::Ends on") << comp->GetInstanceEnd().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime()) << endl;
	}
	if (!comp->GetMaster<iCal::CICalendarVEvent>()->GetLocation().empty())
		ostr << rsrc::GetString("EventTip::Location") << comp->GetMaster<iCal::CICalendarVEvent>()->GetLocation() << endl;
	if (!comp->GetMaster<iCal::CICalendarVEvent>()->GetDescription().empty())
		ostr << rsrc::GetString("EventTip::Description") << comp->GetMaster<iCal::CICalendarVEvent>()->GetDescription() << endl;
	
	// Add calendar name if more than one calendar in use
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() > 1)
	{
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(comp->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
		if (cal != NULL)
			ostr << endl << rsrc::GetString("EventTip::Calendar") << cal->GetName() << endl;
	}

	ostr << ends;
	cdstring temp;
	temp.steal(ostr.str());
	
	mDetails.SetText(temp);
}

void CEventPreview::SetToDo(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Setup a help tag;
	ostrstream ostr;
	ostr << rsrc::GetString("ToDoTip::Summary") << comp->GetMaster<iCal::CICalendarVToDo>()->GetSummary() << endl;

	if (!comp->GetMaster<iCal::CICalendarVToDo>()->HasEnd())
	{
		ostr << rsrc::GetString("ToDoTip::No due date") << endl;
	}
	else
	{
		ostr << rsrc::GetString("ToDoTip::Due on") << comp->GetInstanceEnd().GetAdjustedTime().GetLocaleDate(iCal::CICalendarDateTime::eAbbrevDate) << endl;
	}

	// Add calendar name if more than one calendar in use
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() > 1)
	{
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar( comp->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
		if (cal != NULL)
			ostr << endl << rsrc::GetString("ToDoTip::Calendar") << cal->GetName() << endl;
	}

	ostr << ends;
	cdstring temp;
	temp.steal(ostr.str());
	
	mDetails.SetText(temp);
}

bool CEventPreview::HasData() const
{
	return (mComp.get() != NULL);
}

void CEventPreview::OnEdit()
{
	Edit();
}

void CEventPreview::Edit()
{
	// Determine type
	switch(mComp->GetOwner()->GetType())
	{
	case iCal::CICalendarComponent::eVEVENT:
		CNewEventDialog::StartEdit(*mComp->GetMaster<iCal::CICalendarVEvent>());
		break;
	case iCal::CICalendarComponent::eVTODO:
		CNewToDoDialog::StartEdit(*mComp->GetMaster<iCal::CICalendarVToDo>());
		break;
	default:;
	}
}
