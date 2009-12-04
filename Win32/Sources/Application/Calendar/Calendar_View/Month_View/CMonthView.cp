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
#include "CFontCache.h"
#include "CLog.h"
#include "CMulberryApp.h"
#ifdef _TODO
#include "CMonthPrintout.h"
#endif
#include "CNewEventDialog.h"
#include "CPreferences.h"
#include "CStatusWindow.h"
#include "CUnicodeUtils.h"

#include "CTimezonePopup.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

BEGIN_MESSAGE_MAP(CMonthView, CCalendarViewBase)
	ON_WM_CREATE()

	ON_COMMAND(ID_FILE_PRINT, DoPrint)
	
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDM_CALENDAR_NEWEVENT, OnNewEvent)

	ON_COMMAND(IDM_CALENDAR_GOTO_TODAY, OnToday)
	ON_COMMAND(IDM_CALENDAR_GOTO_DATE, OnPickDate)

	ON_COMMAND(IDC_MONTHVIEW_MONTHMINUS, OnPreviousMonth)
	ON_COMMAND(IDC_MONTHVIEW_MONTHPLUS, OnNextMonth)
	ON_COMMAND(IDC_MONTHVIEW_YEARMINUS, OnPreviousYear)
	ON_COMMAND(IDC_MONTHVIEW_YEARPLUS, OnNextYear)
	ON_COMMAND(IDC_MONTHVIEW_TIMEZONE_POPUP, OnTimezone)
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARMESSAGEPRINTBTN, DoPrint)
	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_TODAY, OnToday)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_GOTO, OnPickDate)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CMonthView														  [public]
/**
	Default constructor */

CMonthView::CMonthView(CCommander* cmdr) :
	CCalendarViewBase(cmdr)
{
}


// ---------------------------------------------------------------------------
//	~CMonthView														  [public]
/**
	Destructor */

CMonthView::~CMonthView()
{
}

#pragma mark -

const int cCaptionHeight = 16;
const int cPopupHeight = 22;
const int cBtnSize = 16;
const int cBtnSpace = 8;

const int cMonthPrevBtnHOffset = cBtnSpace;
const int cBtnVOffset = 7;

const int cMonthHOffset = cMonthPrevBtnHOffset + cBtnSize + cBtnSpace;
const int cMonthWidth = 60;

const int cMonthNextBtnHOffset = cMonthHOffset + cMonthWidth + cBtnSpace;

const int cYearPrevBtnHOffset = cMonthNextBtnHOffset + cBtnSize + cBtnSpace;

const int cYearHOffset = cYearPrevBtnHOffset + cBtnSize + cBtnSpace;
const int cYearWidth = 32;

const int cYearNextBtnHOffset = cYearHOffset + cYearWidth + cBtnSpace;

const int cTimezonePopupWidth = 150;
const int cTimezonePopupHOffset = cYearNextBtnHOffset + cBtnSize + cBtnSpace;
const int cTimezonePopupVOffset = 4;

const int cCaptionVOffset = cTimezonePopupVOffset + 5;

const int cTopHeight = cTimezonePopupVOffset + cPopupHeight + 4;

const int cTitleHeight = 16;

int CMonthView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Get the UI objects
	CRect r = CRect(cMonthPrevBtnHOffset, cBtnVOffset, cMonthPrevBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mMonthMinus.Create(_T(""), r, this, IDC_MONTHVIEW_MONTHMINUS, IDC_STATIC, IDI_PREVMSG);
	r = CRect(cMonthNextBtnHOffset, cBtnVOffset, cMonthNextBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mMonthPlus.Create(_T(""), r, this, IDC_MONTHVIEW_MONTHPLUS, IDC_STATIC, IDI_NEXTMSG);

	r = CRect(cYearPrevBtnHOffset, cBtnVOffset, cYearPrevBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mYearMinus.Create(_T(""), r, this, IDC_MONTHVIEW_YEARMINUS, IDC_STATIC, IDI_PREVMSG);
	r = CRect(cYearNextBtnHOffset, cBtnVOffset, cYearNextBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize);
	mYearPlus.Create(_T(""), r, this, IDC_MONTHVIEW_YEARPLUS, IDC_STATIC, IDI_NEXTMSG);

	r = CRect(cTimezonePopupHOffset, cTimezonePopupVOffset, cTimezonePopupHOffset + cTimezonePopupWidth, cTimezonePopupVOffset + cPopupHeight);
	mTimezonePopup.Create(_T(""), r, this, IDC_MONTHVIEW_TIMEZONE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mTimezonePopup.SetFont(CMulberryApp::sAppFont);
	mTimezonePopup.NoFloating();
	mTimezonePopup.SetValue(IDM_TIMEZONE_UTC);

	r = CRect(cMonthHOffset, cCaptionVOffset, cMonthHOffset + cMonthWidth, cCaptionVOffset + cCaptionHeight);
	mMonth.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_CENTER, r, this, IDC_STATIC);
	mMonth.SetFont(CMulberryApp::sAppFont);

	r = CRect(cYearHOffset, cCaptionVOffset, cYearHOffset + cYearWidth, cCaptionVOffset + cCaptionHeight);
	mYear.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_CENTER, r, this, IDC_STATIC);
	mYear.SetFont(CMulberryApp::sAppFont);
	
	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(0, cTopHeight + cTitleHeight, width, height), this, IDC_MONTHVIEW_TABLE);
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

	return 0;
}

CCalendarTableBase* CMonthView::GetTable() const
{
	return const_cast<CMonthTable*>(&mTable);
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
	mTimezonePopup.SetTimezone(date.GetTimezone());
	ResetDate();
}

void CMonthView::SetCaptions()
{
	// Set static text
	CUnicodeUtils::SetWindowTextUTF8(&mMonth, mDate.GetMonthText(false));
	
	char buf[256];
	snprintf(buf, 256, "%ld", mDate.GetYear());
	CUnicodeUtils::SetWindowTextUTF8(&mYear, buf);
}

void CMonthView::OnTimezone()
{
	mTimezonePopup.GetTimezone(mDate.GetTimezone());
	iCal::CICalendarManager::sICalendarManager->SetDefaultTimezone(mDate.GetTimezone());
	CPreferences::sPrefs->mTimezone.SetValue(mDate.GetTimezone().GetTimezoneID());
	ResetDate();
}

void CMonthView::DoPrint()
{
#ifdef _TODO
	auto_ptr<CMonthPrintout> pout;

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
#endif
}
