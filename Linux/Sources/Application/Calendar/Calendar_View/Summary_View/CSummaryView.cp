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

#include "CSummaryView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CCommands.h"
#include "CCalendarUtils.h"
//#include "CSummaryPrintout.h"
#include "CSummaryTable.h"
#include "CSummaryTitleTable.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include "JXMultiImageButton.h"

#include <cassert>

// ---------------------------------------------------------------------------
//	CSummaryView														  [public]
/**
	Default constructor */

CSummaryView::CSummaryView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
	CCalendarViewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mSummaryType = NCalendarView::eList;
	mSummaryRange = NCalendarView::eMonths;
}


// ---------------------------------------------------------------------------
//	~CSummaryView														  [public]
/**
	Destructor */

CSummaryView::~CSummaryView()
{
}

#pragma mark -

const int cTitleHeight = 16;

void CSummaryView::OnCreate()
{
	// Get the UI objects
// begin JXLayout1

    mTimezonePopup =
        new CTimezonePopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,5, 130,20);
    assert( mTimezonePopup != NULL );

    mRangePopup =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 145,5, 130,20);
    assert( mRangePopup != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,170);
    assert( sbs != NULL );

// end JXLayout1

	mTimezonePopup->OnCreate();
	mTimezonePopup->NoFloating();

	mRangePopup->SetMenuItems("Today %r | Several Days %r | This Week %r | Several Weeks %r | This Month %r | Several Months %r | This Year %r | Several Years %r");

	mTitles = new CSummaryTitleTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kFixedTop,
														0,0, 105, cTitleHeight);

	mTable = new CSummaryTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,cTitleHeight, 105, 100);
	mTable->OnCreate();
	mTitles->OnCreate();
	mTitles->SyncTable(mTable, true);

	mTable->SetCalendarView(this);
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
	mTable->Add_Listener(this);
	
	// Get text traits resource
	ResetFont(CPreferences::sPrefs->mListTextFontInfo.GetValue());

	// Listen to some controls
	ListenTo(mTimezonePopup);
	ListenTo(mRangePopup);
}

CCalendarTableBase* CSummaryView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CSummaryView::Receive(JBroadcaster* sender, const Message& message) 
{
	if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mTimezonePopup)
		{
			OnTimezone();
			return;
		}
		else if (sender == mRangePopup)
		{
			OnRange();
			return;
		}
	}
}

//	Respond to commands
bool CSummaryView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnNewEvent();
		return true;

	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
		OnToday();
		return true;

	default:;
	};

	return CCalendarViewBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CSummaryView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnUpdateNewEvent(cmdui);
		return;

	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CCalendarViewBase::UpdateCommand(cmd, cmdui);
}

void CSummaryView::OnUpdateNewEvent(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CSummaryView::SetDate(const iCal::CICalendarDateTime& date)
{
	mDate = date;
	//mTimezonePopup->SetTimezone(date.GetTimezone());
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	
	// Date is always based on today
	mDate.SetNow();

	ResetDate();
}

void CSummaryView::OnTimezone()
{
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CSummaryView::SetType(NCalendarView::ESummaryType type)
{
	mSummaryType = type;
	
	// Only one right now so nothing more to do
}

void CSummaryView::SetRange(NCalendarView::ESummaryRanges range)
{
	mSummaryRange = range;
	mRangePopup->SetValue(mSummaryRange + 1);
	ResetDate();
}

void CSummaryView::OnRange()
{
	mSummaryRange = static_cast<NCalendarView::ESummaryRanges>(mRangePopup->GetValue() - 1);
	ResetDate();
}

// Reset text traits from prefs
void CSummaryView::ResetFont(const SFontInfo& finfo)
{
	mTitles->ResetFont(finfo);
	mTable->ResetFont(finfo);
	mTitles->SyncTable(mTable, true);
}

void CSummaryView::DoPrint()
{
#ifdef _TODO
#endif
}
