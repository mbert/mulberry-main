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

#include "CSummaryView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CFontCache.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
//#include "CSummaryPrintout.h"
#include "CSummaryTable.h"
#include "CSummaryTitleTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include "cdustring.h"

BEGIN_MESSAGE_MAP(CSummaryView, CCalendarViewBase)
	ON_WM_CREATE()
	
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDM_CALENDAR_NEWEVENT, OnNewEvent)

	ON_COMMAND(IDM_CALENDAR_GOTO_TODAY, OnToday)

	ON_COMMAND(IDC_SUMMARYVIEW_TIMEZONE_POPUP, OnTimezone)
	ON_COMMAND(IDC_SUMMARYVIEW_RANGE_POPUP, OnRange)
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_TODAY, OnToday)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CSummaryView														  [public]
/**
	Default constructor */

CSummaryView::CSummaryView(CCommander* cmdr) :
	CCalendarViewBase(cmdr),
	mRangePopup(true)
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

const int cCaptionHeight = 16;
const int cPopupHeight = 22;

const int cTimezonePopupWidth = 150;
const int cTimezonePopupHOffset = 4;
const int cTimezonePopupVOffset = 4;

const int cRangePopupWidth = 130;
const int cRangePopupHOffset = cTimezonePopupHOffset + cTimezonePopupWidth + 8;
const int cRangePopupVOffset = cTimezonePopupVOffset;

const int cTopHeight = cRangePopupVOffset + cPopupHeight + 4;

const int cTitleHeight = 16;

int CSummaryView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Get the UI objects

	CRect r = CRect(cTimezonePopupHOffset, cTimezonePopupVOffset, cTimezonePopupHOffset + cTimezonePopupWidth, cTimezonePopupVOffset + cPopupHeight);
	mTimezonePopup.Create(_T(""), r, this, IDC_SUMMARYVIEW_TIMEZONE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mTimezonePopup.SetFont(CMulberryApp::sAppFont);
	mTimezonePopup.NoFloating();
	mTimezonePopup.SetValue(IDM_TIMEZONE_UTC);

	r = CRect(cRangePopupHOffset, cRangePopupVOffset, cRangePopupHOffset + cRangePopupWidth, cRangePopupVOffset + cPopupHeight);
	mRangePopup.Create(_T(""), r, this, IDC_SUMMARYVIEW_RANGE_POPUP, IDC_STATIC, IDI_POPUPBTN);
	mRangePopup.SetMenu(IDR_POPUP_SUMMARYRANGE);
	mRangePopup.SetFont(CMulberryApp::sAppFont);
	mRangePopup.SetValue(IDM_SUMMARYRANGE_TODAY);

	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
					CRect(0, cTopHeight + cTitleHeight, width, height), this, IDC_SUMMARYVIEW_TABLE);
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
	
	ResetFont(CFontCache::GetListFont());

	return 0;
}

// Init columns and text
void CSummaryView::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();
	
	SColumnInfo newInfo;

	newInfo.column_type = 1;
	newInfo.column_width = 16;
	mColumnInfo.push_back(newInfo);

	newInfo.column_type = 2;
	newInfo.column_width = 16;
	mColumnInfo.push_back(newInfo);

	newInfo.column_type = 3;
	newInfo.column_width = 16;
	mColumnInfo.push_back(newInfo);

	newInfo.column_type = 4;
	newInfo.column_width = 16;
	mColumnInfo.push_back(newInfo);

	newInfo.column_type = 5;
	newInfo.column_width = 16;
	mColumnInfo.push_back(newInfo);
}

CCalendarTableBase* CSummaryView::GetTable() const
{
	return const_cast<CSummaryTable*>(&mTable);
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
	//mTimezonePopup.SetTimezone(date.GetTimezone());
	mTimezonePopup.GetTimezone(mDate.GetTimezone());
	
	// Date is always based on today
	mDate.SetNow();

	ResetDate();
}

void CSummaryView::OnTimezone()
{
	mTimezonePopup.GetTimezone(mDate.GetTimezone());
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
	mRangePopup.SetValue(mSummaryRange + IDM_SUMMARYRANGE_TODAY);
	ResetDate();
}

void CSummaryView::OnRange()
{
	mSummaryRange = static_cast<NCalendarView::ESummaryRanges>(mRangePopup.GetValue() - IDM_SUMMARYRANGE_TODAY);
	ResetDate();
}

// Reset text traits from prefs
void CSummaryView::ResetFont(CFont* font)
{
	// Change font in table and determine change in row height
	mTable.ResetFont(font);

	// May need to manually adjust titles and position of table
	UInt16 old_height = mTitles.GetRowHeight(1);
	mTitles.ResetFont(font);
	SInt16 delta = mTitles.GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(&mTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(&mTable, 0, -delta, false);
		::MoveWindowBy(&mTable, 0, delta, false);
		
		mTable.RedrawWindow();
		mTitles.RedrawWindow();
	}
}

void CSummaryView::DoPrint()
{
#ifdef _TODO
	std::auto_ptr<CSummaryPrintout> pout(PPx::XMLSerializer::ResourceToObjects<CSummaryPrintout>(CFSTR("CSummaryPrintout")));
	pout->SetPrintSpec(pspec);
	pout->SetDetails(mDate, mType, mSummaryRange, mCalendarView);

	AddSubView(pout.get());
	pout->SetVisible(true);

	pout->DoPrintJob();

	pout->RemoveFromSuperView();
#endif
}
