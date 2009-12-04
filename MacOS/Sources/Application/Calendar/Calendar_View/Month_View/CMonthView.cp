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
#include "CContextMenu.h"
#include "CGUtils.h"
#include "CLog.h"
#include "CMonthPrintout.h"
#include "CMonthTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CStatusWindow.h"

#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <LBevelButton.h>

#ifdef PROFILING
#include <UProfiler.h>
#endif

// ---------------------------------------------------------------------------
//	CMonthView														  [public]
/**
	Default constructor */

CMonthView::CMonthView(LStream *inStream) :
	CCalendarViewBase(inStream)
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

void CMonthView::FinishCreateSelf()
{
	// Get the UI objects
	mMonthMinus = dynamic_cast<LBevelButton*>(FindPaneByID(eMonthMinus_ID));
	mMonthPlus = dynamic_cast<LBevelButton*>(FindPaneByID(eMonthPlus_ID));
	mYearMinus = dynamic_cast<LBevelButton*>(FindPaneByID(eYearMinus_ID));
	mYearPlus = dynamic_cast<LBevelButton*>(FindPaneByID(eYearPlus_ID));

	mTimezonePopup = dynamic_cast<CTimezonePopup*>(FindPaneByID(eTimezone_ID));
	mTimezonePopup->NoFloating();

	mMonth = dynamic_cast<CStaticText*>(FindPaneByID(eMonth_ID));
	mYear = dynamic_cast<CStaticText*>(FindPaneByID(eYear_ID));
	
	mTable = dynamic_cast<CMonthTable*>(FindPaneByID(eTable_ID));
	mTable->SetCalendarView(this);
	mTable->Add_Listener(this);
	
	CContextMenuAttachment::AddUniqueContext(this, 1832, mTable);

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

CCalendarTableBase* CMonthView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CMonthView::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eMonthMinus_ID:
		OnPreviousMonth();
		break;

	case eMonthPlus_ID:
		OnNextMonth();
		break;

	case eYearMinus_ID:
		OnPreviousYear();
		break;

	case eYearPlus_ID:
		OnNextYear();
		break;

	case eTimezone_ID:
		OnTimezone();
		break;
	}
}

//	Respond to commands
Boolean CMonthView::ObeyCommand(CommandT inCommand,void *ioParam)
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

	default:
		cmdHandled = CCalendarViewBase::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMonthView::FindCommandStatus(
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
	MyCFString month(mDate.GetMonthText(false), kCFStringEncodingUTF8);
	mMonth->SetCFDescriptor(month);
	
	char buf[256];
	std::snprintf(buf, 256, "%ld", mDate.GetYear());
	MyCFString year(buf, kCFStringEncodingUTF8);
	mYear->SetCFDescriptor(year);
}

void CMonthView::OnTimezone()
{
	mTimezonePopup->GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CMonthView::DoPrint()
{
	std::auto_ptr<CMonthPrintout> pout;

	try
	{
		pout.reset(static_cast<CMonthPrintout*>(LPrintout::CreatePrintout(CMonthPrintout::pane_ID)));
		pout->SetPrintSpec(*GetPrintSpec());
		pout->SetDetails(mDate, mCalendarView);

		pout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}
