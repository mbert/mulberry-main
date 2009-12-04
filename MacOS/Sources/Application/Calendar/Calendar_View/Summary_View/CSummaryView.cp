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
#include "CContextMenu.h"
#include "CGUtils.h"
#include "CSummaryPrintout.h"
#include "CSummaryTable.h"
#include "CSummaryTitleTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CStatusWindow.h"

#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <LBevelButton.h>
#include <LScrollerView.h>

#ifdef PROFILING
#include <UProfiler.h>
#endif

// ---------------------------------------------------------------------------
//	CSummaryView														  [public]
/**
	Default constructor */

CSummaryView::CSummaryView(LStream *inStream) :
	CCalendarViewBase(inStream)
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

void CSummaryView::FinishCreateSelf()
{
	// Get the UI objects
	mTimezonePopup = dynamic_cast<CTimezonePopup*>(FindPaneByID(eTimezone_ID));
	mTimezonePopup->NoFloating();

	mRangePopup = dynamic_cast<LPopupButton*>(FindPaneByID(eRange_ID));

	mTitles = dynamic_cast<CSummaryTitleTable*>(FindPaneByID(eTitleTable_ID));

	mScroller = dynamic_cast<LScrollerView*>(FindPaneByID(eScroller_ID));

	mTable = dynamic_cast<CSummaryTable*>(FindPaneByID(eTable_ID));
	mTable->SetCalendarView(this);
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
	mTable->Add_Listener(this);
	
	CContextMenuAttachment::AddUniqueContext(this, 1832, mTable);
	
	// Get text traits resource
	ResetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

CCalendarTableBase* CSummaryView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CSummaryView::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eTimezone_ID:
		OnTimezone();
		break;

	case eRange_ID:
		OnRange();
		break;
	}
}

//	Respond to commands
Boolean CSummaryView::ObeyCommand(CommandT inCommand,void *ioParam)
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

	default:
		cmdHandled = CCalendarViewBase::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CSummaryView::FindCommandStatus(
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
		// Always enabled
		outEnabled = true;
		break;

	default:
		CCalendarViewBase::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
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
void CSummaryView::ResetTextTraits(const TextTraitsRecord& list_traits)
{
	mTable->SetTextTraits(list_traits);
	
	short old_height = mTitles->GetRowHeight(1);

	// Change fonts
	mTitles->SetTextTraits(list_traits);

	// Get new row height
	short diff_height = mTitles->GetRowHeight(1) - old_height;

	// Change panes if row height changed
	if (diff_height)
	{
		// Offset and resize table
		mTable->GetSuperView()->MoveBy(0, diff_height, false);
		mTable->GetSuperView()->ResizeFrameBy(0, -diff_height, true);

		// Do header and button
		mTitles->ResizeFrameBy(0, diff_height, true);
	}
}

void CSummaryView::DoPrint()
{
	std::auto_ptr<CSummaryPrintout> pout;

	try
	{
		pout.reset(static_cast<CSummaryPrintout*>(LPrintout::CreatePrintout(CSummaryPrintout::pane_ID)));
		pout->SetPrintSpec(*GetPrintSpec());
		pout->SetDetails(mDate, mSummaryType, mSummaryRange, mCalendarView);

		pout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}
