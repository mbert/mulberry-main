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

#include "CFreeBusyView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#ifdef _TODO
#include "CFreeBusyPrintout.h"
#endif
#include "CFontCache.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CStatusWindow.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

BEGIN_MESSAGE_MAP(CFreeBusyView, CCalendarViewBase)
	ON_WM_CREATE()

	ON_COMMAND(ID_FILE_PRINT, DoPrint)
	
	ON_COMMAND(IDM_CALENDAR_GOTO_TODAY, OnToday)
	ON_COMMAND(IDM_CALENDAR_GOTO_DATE, OnPickDate)

	ON_COMMAND(IDC_FREEBUSYVIEW_DAYPREV, OnPrevDay)
	ON_COMMAND(IDC_FREEBUSYVIEW_DAYNEXT, OnNextDay)
	ON_COMMAND(IDC_FREEBUSYVIEW_TIMEZONE_POPUP, OnTimezone)
	ON_COMMAND(IDC_FREEBUSYVIEW_SCALE_POPUP, OnScale)
	ON_COMMAND(IDC_FREEBUSYVIEW_RANGE_POPUP, OnRange)
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARMESSAGEPRINTBTN, DoPrint)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CFreeBusyView														  [public]
/**
	Default constructor */

CFreeBusyView::CFreeBusyView(CCommander* cmdr) :
	CCalendarViewBase(cmdr),
	mScalePopup(true), mRangePopup(true)
{
	mFreeBusyRange = CDayWeekViewTimeRange::e24Hrs;
	mFreeBusyScale = 0;
	
	mSelectedEvent = NULL;
}


// ---------------------------------------------------------------------------
//	~CFreeBusyView														  [public]
/**
	Destructor */

CFreeBusyView::~CFreeBusyView()
{
}

#pragma mark -

const int cCaptionHeight = 16;
const int cPopupHeight = 22;
const int cBtnSize = 16;
const int cBtnSpace = 8;

const int cWeekPrevBtnHOffset = cBtnSpace;
const int cBtnVOffset = 7;

const int cDayPrevBtnHOffset = cWeekPrevBtnHOffset + cBtnSize;
const int cDayNextBtnHOffset = cDayPrevBtnHOffset + cBtnSize;
const int cWeekNextBtnHOffset = cDayNextBtnHOffset + cBtnSize;

const int cTimezonePopupWidth = 150;
const int cTimezonePopupHOffset = cWeekNextBtnHOffset + cBtnSize + cBtnSpace;
const int cTimezonePopupVOffset = 4;

const int cScalePopupWidth = 90;
const int cScalePopupHOffset = cTimezonePopupHOffset + cTimezonePopupWidth + cBtnSpace;
const int cScalePopupVOffset = 4;

const int cRangePopupWidth = 90;
const int cRangePopupHOffset = cScalePopupHOffset + cScalePopupWidth + cBtnSpace;
const int cRangePopupVOffset = 4;

const int cCaptionVOffset = cTimezonePopupVOffset + 5;

const int cTopHeight = cTimezonePopupVOffset + cPopupHeight + 4;

const int cTitleHeight = 16;

int CFreeBusyView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 24 : 0;
	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Get the UI objects

	CRect r = CRect(cDayPrevBtnHOffset, cBtnVOffset, cDayPrevBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mPrevDay.Create(_T(""), r, this, IDC_FREEBUSYVIEW_DAYPREV, IDC_STATIC, IDI_PREVMSG);

	r = CRect(cDayNextBtnHOffset, cBtnVOffset, cDayNextBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mNextDay.Create(_T(""), r, this, IDC_FREEBUSYVIEW_DAYNEXT, IDC_STATIC, IDI_NEXTMSG);

	r = CRect(cTimezonePopupHOffset, cTimezonePopupVOffset, cTimezonePopupHOffset + cTimezonePopupWidth, cTimezonePopupVOffset + cPopupHeight);
	mTimezonePopup.Create(_T(""), r, this, IDC_FREEBUSYVIEW_TIMEZONE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mTimezonePopup.SetFont(CMulberryApp::sAppFont);
	mTimezonePopup.NoFloating();
	mTimezonePopup.SetValue(IDM_TIMEZONE_UTC);

	r = CRect(cScalePopupHOffset, cScalePopupVOffset, cScalePopupHOffset + cScalePopupWidth + large_offset, cScalePopupVOffset + cPopupHeight);
	mScalePopup.Create(_T(""), r, this, IDC_FREEBUSYVIEW_SCALE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mScalePopup.SetMenu(IDR_POPUP_DAYVIEW_SCALE);
	mScalePopup.SetFont(CMulberryApp::sAppFont);
	mScalePopup.SetValue(IDM_DAYVIEW_SCALE_FIT);

	r = CRect(cRangePopupHOffset + large_offset, cRangePopupVOffset, cRangePopupHOffset + cRangePopupWidth + 2 * large_offset, cRangePopupVOffset + cPopupHeight);
	mRangePopup.Create(_T(""), r, this, IDC_FREEBUSYVIEW_RANGE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mRangePopup.SetMenu(IDR_POPUP_DAYVIEW_RANGE);
	mRangePopup.SetFont(CMulberryApp::sAppFont);
	mRangePopup.SetValue(IDM_DAYVIEW_RANGE_FULL);

	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
					CRect(0, cTopHeight + cTitleHeight, width, height), this, IDC_FREEBUSYVIEW_TABLE);
	mTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	AddAlignment(new CWndAlignment(&mTable, CWndAlignment::eAlign_WidthHeight));
	mTable.ResetFont(CFontCache::GetListFont());
	mTable.SetContextMenuID(IDR_POPUP_CONTEXT_EVENTTABLE);
	mTable.SetContextView(static_cast<CView*>(mCalendarView->GetOwningWindow()));
	mTable.SetSuperCommander(this);
	mTable.SetCalendarView(this);
	mTable.Add_Listener(this);
	
	// Get titles
	mTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, cTopHeight, width, cTopHeight + cTitleHeight), this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mTitles, CWndAlignment::eAlign_TopWidth));
	mTitles.SetFont(CFontCache::GetListFont());
	mTable.SetTitles(&mTitles);
	mTitles.SetTable(&mTable);
	mTitles.Add_Listener(this);
	
	return 0;
}

CCalendarTableBase* CFreeBusyView::GetTable() const
{
	return const_cast<CFreeBusyTable*>(&mTable);
}

void CFreeBusyView::OnUpdateNewEvent(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CFreeBusyView::SetDate(const iCal::CICalendarDateTime& date)
{
	mDate = date;
	//mTimezonePopup.SetTimezone(date.GetTimezone());
	mTimezonePopup.GetTimezone(mDate.GetTimezone());
	ResetDate();
}

void CFreeBusyView::OnTimezone()
{
	mTimezonePopup.GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CFreeBusyView::SetScale(uint32_t scale)
{
	mFreeBusyScale = scale;
	mScalePopup.SetValue(mFreeBusyScale + IDM_DAYVIEW_SCALE_FIT);
}

void CFreeBusyView::OnScale()
{
	mFreeBusyScale = mScalePopup.GetValue() - IDM_DAYVIEW_SCALE_FIT;
	mTable.ScaleColumns(mFreeBusyScale);
	RedrawWindow();
}

void CFreeBusyView::SetRange(CDayWeekViewTimeRange::ERanges range)
{
	mFreeBusyRange = range;
	mRangePopup.SetValue(mFreeBusyRange + IDM_DAYVIEW_RANGE_FULL);
}

void CFreeBusyView::OnRange()
{
	mFreeBusyRange = static_cast<CDayWeekViewTimeRange::ERanges>(mRangePopup.GetValue() - IDM_DAYVIEW_RANGE_FULL);
	ResetDate();
	RedrawWindow();
}

void CFreeBusyView::ResetFont(CFont* font)
{
	// Change fonts
	//mTable.ResetFont(font);

	// May need to manually adjust titles and position of table
	mTitles.ResetFont(font);
	mTitles.SyncTable(&mTable, true);
}

void CFreeBusyView::DoPrint()
{
#ifdef _TODO
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
#endif
}
