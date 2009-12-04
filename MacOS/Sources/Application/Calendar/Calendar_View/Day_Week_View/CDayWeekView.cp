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
#include "CContextMenu.h"
#include "CGUtils.h"
#include "CDayWeekPrintout.h"
#include "CDayWeekTable.h"
#include "CDayWeekTitleTable.h"
#include "CLog.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CStatusWindow.h"

#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <LBevelButton.h>
#include <LScrollerView.h>

#ifdef PROFILING
#include <UProfiler.h>
#endif

// ---------------------------------------------------------------------------
//	CDayWeekView														  [public]
/**
	Default constructor */

CDayWeekView::CDayWeekView(LStream *inStream) :
	CCalendarViewBase(inStream)
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

void CDayWeekView::FinishCreateSelf()
{
	// Get the UI objects
	mPrevWeek = dynamic_cast<LBevelButton*>(FindPaneByID(ePrevWeek_ID));
	mNextWeek = dynamic_cast<LBevelButton*>(FindPaneByID(eNextWeek_ID));
	mPrevDay = dynamic_cast<LBevelButton*>(FindPaneByID(ePrevDay_ID));
	mNextDay = dynamic_cast<LBevelButton*>(FindPaneByID(eNextDay_ID));

	mTimezonePopup = dynamic_cast<CTimezonePopup*>(FindPaneByID(eTimezone_ID));
	mTimezonePopup->NoFloating();

	mScalePopup = dynamic_cast<LPopupButton*>(FindPaneByID(eScale_ID));
	mRangePopup = dynamic_cast<LPopupButton*>(FindPaneByID(eRange_ID));

	mTitles = dynamic_cast<CDayWeekTitleTable*>(FindPaneByID(eTitleTable_ID));

	mScroller = dynamic_cast<LScrollerView*>(FindPaneByID(eScroller_ID));

	mTable = dynamic_cast<CDayWeekTable*>(FindPaneByID(eTable_ID));
	mTable->SetCalendarView(this);
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
	mTable->Add_Listener(this);
	mTitles->Add_Listener(this);
	
	CContextMenuAttachment::AddUniqueContext(this, 1832, mTable);

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

CCalendarTableBase* CDayWeekView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CDayWeekView::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case ePrevWeek_ID:
		OnPrevWeek();
		break;

	case eNextWeek_ID:
		OnNextWeek();
		break;

	case ePrevDay_ID:
		OnPrevDay();
		break;

	case eNextDay_ID:
		OnNextDay();
		break;

	case eTimezone_ID:
		OnTimezone();
		break;

	case eScale_ID:
		OnScale();
		break;

	case eRange_ID:
		OnRange();
		break;
	}
}

//	Respond to commands
Boolean CDayWeekView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_PageSetup:
		{
			StPrintSession			session(*GetPrintSpec());
			StDesktopDeactivator	deactivator;
			UPrinting::AskPageSetup(*GetPrintSpec());
		}
		break;

	case cmd_Print:
	case cmd_ToolbarMessagePrintBtn:
		//  Hide status window as Print Manager gets name of top window and does not know about floats
		{
			StPrintSession	session(*GetPrintSpec());
			StStatusWindowHide hide;
			bool do_print = false;
			{
				StDesktopDeactivator	deactivator;
				do_print = UPrinting::AskPrintJob(*GetPrintSpec());
			}
			if (do_print)
				DoPrint();
		}

		// Print job status window might mess up window order
		UDesktop::NormalizeWindowOrder();
		break;

	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
		OnNewEvent();
		break;

	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
		OnToday();
		break;

	case cmd_GotoDate:
	case cmd_ToolbarGoToDateBtn:
		OnPickDate();
		break;

	// Pass these to selected item if present
	case cmd_ToolbarNewLetter:
	case cmd_ToolbarNewLetterOption:
#ifdef _TODO
		if (mSelectedEvent != NULL)
		{
			cmdHandled = mSelectedEvent->ObeyCommand(inCommand, ioParam);
			break;
		}
#endif
		
		// else fall through

	default:
		cmdHandled = CCalendarViewBase::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CDayWeekView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
	{
		// Only if subscribed nodes
		if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
		{
			const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
			outEnabled = (GetCalendar() != NULL) || (cals.size() > 0);
		}
		else
			outEnabled = false;
		break;
	}

	case cmd_PageSetup:
	case cmd_Print:
	case cmd_ToolbarMessagePrintBtn:
	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
	case cmd_GotoDate:
	case cmd_ToolbarGoToDateBtn:
		// Always enabled
		outEnabled = true;
		break;

	default:
		CCalendarViewBase::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
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
	mPrevWeek->SetVisible(mType == eDay);
	mNextWeek->SetVisible(mType == eDay);
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

void CDayWeekView::DoPrint()
{
	std::auto_ptr<CDayWeekPrintout> pout;

	try
	{
		pout.reset(static_cast<CDayWeekPrintout*>(LPrintout::CreatePrintout(CDayWeekPrintout::pane_ID)));
		pout->SetPrintSpec(*GetPrintSpec());
		pout->SetDetails(mDate, mType, mDayWeekRange, mCalendarView);

		pout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}
