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

#include "CDayWeekView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CCommands.h"
#include "CCalendarUtils.h"
#ifdef _TODO
#include "CDayWeekPrintout.h"
#endif
#include "CDayWeekTable.h"
#include "CDayWeekTitleTable.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include "JXMultiImageButton.h"

#include <cassert>

// ---------------------------------------------------------------------------
//	CDayWeekView														  [public]
/**
	Default constructor */

CDayWeekView::CDayWeekView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
	CCalendarViewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mType = eDay;
	mDayWeekRange = CDayWeekViewTimeRange::e24Hrs;
	mDayWeekScale = 0;
	
	mTitles = NULL;
	mTable = NULL;
	mSelectedEvent = NULL;
}


// ---------------------------------------------------------------------------
//	~CDayWeekView														  [public]
/**
	Destructor */

CDayWeekView::~CDayWeekView()
{
}

#pragma mark -

const int cTitleHeight = 16;

void CDayWeekView::OnCreate()
{
	// Get the UI objects
// begin JXLayout1

    mPrevWeek =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 24,22);
    assert( mPrevWeek != NULL );

    mNextWeek =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,5, 24,22);
    assert( mNextWeek != NULL );

    mPrevDay =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 30,5, 24,22);
    assert( mPrevDay != NULL );

    mNextDay =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 55,5, 24,22);
    assert( mNextDay != NULL );

    mTimezonePopup =
        new CTimezonePopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 110,5, 130,20);
    assert( mTimezonePopup != NULL );

    mScalePopup =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 245,5, 120,20);
    assert( mScalePopup != NULL );

    mRangePopup =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 370,5, 120,20);
    assert( mRangePopup != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,170);
    assert( sbs != NULL );

// end JXLayout1

	mPrevWeek->SetImage(IDI_PREVMSG);
	mNextWeek->SetImage(IDI_NEXTMSG);
	mPrevDay->SetImage(IDI_PREVMSG);
	mNextDay->SetImage(IDI_NEXTMSG);

	mTimezonePopup->OnCreate();
	mTimezonePopup->NoFloating();

	mScalePopup->SetMenuItems("Fit to Height %r | Full Height %r | 1/2 Height %r | 1/3 Height %r | 1/4 Height %r");

	mRangePopup->SetMenuItems("Full 24 Hours %r | Morning %r | Afternoon %r | Evening %r | Awake Period %r | Work Period %r");

	mTitles = new CDayWeekTitleTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kFixedTop,
														0,0, 105, cTitleHeight);

	mTable = new CDayWeekTable(sbs,sbs->GetScrollEnclosure(),
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
	mTitles->Add_Listener(this);

	// Listen to some controls
	ListenTo(mPrevWeek);
	ListenTo(mNextWeek);
	ListenTo(mPrevDay);
	ListenTo(mNextDay);
	ListenTo(mTimezonePopup);
	ListenTo(mScalePopup);
	ListenTo(mRangePopup);
}

CCalendarTableBase* CDayWeekView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CDayWeekView::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mPrevWeek)
		{
			OnPrevWeek();
			return;
		}		
		else if (sender == mNextWeek)
		{
			OnNextWeek();
			return;
		}		
		else if (sender == mPrevDay)
		{
			OnPrevDay();
			return;
		}		
		else if (sender == mNextDay)
		{
			OnNextDay();
			return;
		}		
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mTimezonePopup)
		{
			OnTimezone();
			return;
		}
		else if (sender == mScalePopup)
		{
			OnScale();
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
bool CDayWeekView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
#ifdef _TODO
	case CCommand::eFilePageSetup:
		return true;

	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
		return true;
#endif

	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnNewEvent();
		return true;

	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
		OnToday();
		return true;

	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
		OnPickDate();
		return true;

	// Pass these to selected item if present
	case CCommand::eToolbarNewLetter:
	case CCommand::eToolbarNewLetterOption:
#ifdef _TODO
		if (mSelectedEvent != NULL)
		{
			return mSelectedEvent->ObeyCommand(cmd, menu);
		}
#endif
		
		// else fall through

	default:;
	};

	return CCalendarViewBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CDayWeekView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnUpdateNewEvent(cmdui);
		return;

	case CCommand::eFilePageSetup:
	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CCalendarViewBase::UpdateCommand(cmd, cmdui);
}

void CDayWeekView::OnUpdateNewEvent(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CDayWeekView::SetDate(const iCal::CICalendarDateTime& date)
{
	mDate = date;
	//mTimezonePopup->SetTimezone(date.GetTimezone());
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	ResetDate();
}

void CDayWeekView::SetType(EDayWeekType type)
{
	mType = type;
	mPrevWeek->SetVisible(mType == eDay ? kTrue : kFalse);
	mNextWeek->SetVisible(mType == eDay ? kTrue : kFalse);
}

void CDayWeekView::OnTimezone()
{
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CDayWeekView::SetScale(uint32_t scale)
{
	mDayWeekScale = scale;
	mScalePopup->SetValue(mDayWeekScale + 1);
}

void CDayWeekView::OnScale()
{
	mDayWeekScale = mScalePopup->GetValue() - 1;
	mTable->ScaleRows(mDayWeekScale);
	Refresh();
}

void CDayWeekView::SetRange(CDayWeekViewTimeRange::ERanges range)
{
	mDayWeekRange = range;
	mRangePopup->SetValue(mDayWeekRange + 1);
}

void CDayWeekView::OnRange()
{
	mDayWeekRange = static_cast<CDayWeekViewTimeRange::ERanges>(mRangePopup->GetValue() - 1);
	ResetDate();
	Refresh();
}

void CDayWeekView::ResetFont(const SFontInfo& finfo)
{
	// Change fonts
	//mTable->ResetFont(finfo);

	// May need to manually adjust titles and position of table
	mTitles->ResetFont(finfo);
	mTitles->SyncTable(mTable, true);
}

void CDayWeekView::DoPrint()
{
#ifdef _TODO
#endif
}
