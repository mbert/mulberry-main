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

#include "CMonthView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CCommands.h"
#include "CCalendarUtils.h"
#include "CLog.h"
#ifdef _TODO
#include "CMonthPrintout.h"
#endif
#include "CMonthTable.h"
#include "CMonthTitleTable.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTableScrollbarSet.h"
#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include "JXMultiImageButton.h"

#include <cassert>

// ---------------------------------------------------------------------------
//	CMonthView														  [public]
/**
	Default constructor */

CMonthView::CMonthView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
	CCalendarViewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mTable = NULL;
}


// ---------------------------------------------------------------------------
//	~CMonthView														  [public]
/**
	Destructor */

CMonthView::~CMonthView()
{
}

#pragma mark -

const int cTitleHeight = 16;

void CMonthView::OnCreate()
{
	// Get the UI objects
// begin JXLayout1

    mMonthMinus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 24,22);
    assert( mMonthMinus != NULL );

    mMonth =
        new CStaticText("January", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 31,7, 72,18);
    assert( mMonth != NULL );
    mMonth->SetFontSize(10);

    mMonthPlus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 105,5, 24,22);
    assert( mMonthPlus != NULL );

    mYearMinus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 137,5, 24,22);
    assert( mYearMinus != NULL );

    mYear =
        new CStaticText("2005", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 168,7, 47,18);
    assert( mYear != NULL );
    mYear->SetFontSize(10);

    mYearPlus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 222,5, 24,22);
    assert( mYearPlus != NULL );

    mTimezonePopup =
        new CTimezonePopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 263,5, 130,20);
    assert( mTimezonePopup != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,170);
    assert( sbs != NULL );

// end JXLayout1

	mMonthMinus->SetImage(IDI_PREVMSG);
	mMonthPlus->SetImage(IDI_NEXTMSG);
	mYearMinus->SetImage(IDI_PREVMSG);
	mYearPlus->SetImage(IDI_NEXTMSG);
	
	mMonth->SetBreakCROnly(false);
	mMonth->SetFontAlign(CStaticText::kAlignCenter);
	mYear->SetBreakCROnly(false);
	mYear->SetFontAlign(CStaticText::kAlignCenter);

	mTimezonePopup->OnCreate();
	mTimezonePopup->NoFloating();

	mTitles = new CMonthTitleTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kFixedTop,
														0,0, 105, cTitleHeight);

	mTable = new CMonthTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,cTitleHeight, 105, 100);
	mTable->OnCreate();
	mTitles->OnCreate();
	mTitles->SyncTable(mTable, true);

	mTable->SetCalendarView(this);
	mTable->Add_Listener(this);

	// Listen to some controls
	ListenTo(mMonthMinus);
	ListenTo(mMonthPlus);
	ListenTo(mYearMinus);
	ListenTo(mYearPlus);
	ListenTo(mTimezonePopup);
}

CCalendarTableBase* CMonthView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CMonthView::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mMonthMinus)
		{
			OnPreviousMonth();
			return;
		}		
		else if (sender == mMonthPlus)
		{
			OnNextMonth();
			return;
		}		
		else if (sender == mYearMinus)
		{
			OnPreviousYear();
			return;
		}		
		else if (sender == mYearPlus)
		{
			OnNextYear();
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
	}

	CCalendarViewBase::Receive(sender, message);
}

//	Respond to commands
bool CMonthView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
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

	default:;
	};

	return CCalendarViewBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CMonthView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
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

void CMonthView::OnUpdateNewEvent(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CMonthView::SetDate(const iCal::CICalendarDateTime& date)
{
	mDate = date;
	//mTimezonePopup->SetTimezone(date.GetTimezone());
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	ResetDate();
}

void CMonthView::SetCaptions()
{
	// Set static text
	mMonth->SetText(mDate.GetMonthText(false));
	
	char buf[256];
	::snprintf(buf, 256, "%ld", (long)mDate.GetYear());
	mYear->SetText(buf);
}

void CMonthView::OnTimezone()
{
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CMonthView::ResetFont(const SFontInfo& finfo)
{
	// Change fonts
	//mTable->ResetFont(finfo);

	// May need to manually adjust titles and position of table
	mTitles->ResetFont(finfo);
	mTitles->SyncTable(mTable, true);
}

void CMonthView::DoPrint()
{
#ifdef _TODO
#endif
}
