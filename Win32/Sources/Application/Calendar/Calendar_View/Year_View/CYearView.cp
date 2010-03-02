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

#include "CYearView.h"

#include "CCalendarView.h"
#include "CFontCache.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CStatusWindow.h"
#include "CUnicodeUtils.h"
#ifdef _TODO
#include "CYearPrintout.h"
#endif

#include "CCalendarStoreManager.h"

BEGIN_MESSAGE_MAP(CYearView, CCalendarViewBase)
	ON_WM_CREATE()

	ON_COMMAND(ID_FILE_PRINT, DoPrint)
	
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDM_CALENDAR_NEWEVENT, OnNewEvent)

	ON_COMMAND(IDM_CALENDAR_GOTO_TODAY, OnThisYear)
	ON_COMMAND(IDM_CALENDAR_GOTO_DATE, OnPickDate)

	ON_COMMAND(IDC_YEARVIEW_PREV, OnPreviousYear)
	ON_COMMAND(IDC_YEARVIEW_NEXT, OnNextYear)
	ON_COMMAND(IDC_YEARVIEW_LAYOUT, OnLayout)
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARMESSAGEPRINTBTN, DoPrint)
	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnUpdateNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWEVENT, OnNewEvent)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_TODAY, OnThisYear)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_GOTO, OnPickDate)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CYearView														  [public]
/**
	Default constructor */

CYearView::CYearView(CCommander* cmdr) :
	CCalendarViewBase(cmdr),
	mLayout(true)
{
	mLayoutValue = NCalendarView::e3x4;
}


// ---------------------------------------------------------------------------
//	~CYearView														  [public]
/**
	Destructor */

CYearView::~CYearView()
{
}

#pragma mark -

const int cCaptionHeight = 16;
const int cPopupHeight = 22;
const int cBtnSize = 16;

const int cPrevBtnHOffset = 8;
const int cBtnVOffset = 7;

const int cCaptionHOffset = cPrevBtnHOffset + cBtnSize + cPrevBtnHOffset;
const int cCaptionWidth = 32;

const int cNextBtnHOffset = cCaptionHOffset + cCaptionWidth + cPrevBtnHOffset;

const int cLayoutPopupWidth = 140;
const int cLayoutPopupHOffset = cNextBtnHOffset + cBtnSize + cPrevBtnHOffset;
const int cLayoutPopupVOffset = 4;

const int cCaptionVOffset = cLayoutPopupVOffset + 5;

const int cTopHeight = cLayoutPopupVOffset + cPopupHeight + 4;

int CYearView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Get the UI objects
	mYearMinus.Create(_T(""), CRect(cPrevBtnHOffset, cBtnVOffset, cPrevBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize), this, IDC_YEARVIEW_PREV, IDC_STATIC, IDI_PREVMSG);
	mYearPlus.Create(_T(""), CRect(cNextBtnHOffset, cBtnVOffset, cNextBtnHOffset + cBtnSize, cBtnVOffset + cBtnSize), this, IDC_YEARVIEW_NEXT, IDC_STATIC, IDI_NEXTMSG);

	mYear.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight), this, IDC_STATIC);
	mYear.SetFont(CMulberryApp::sAppFont);

	mLayout.Create(_T(""), CRect(cLayoutPopupHOffset, cLayoutPopupVOffset, cLayoutPopupHOffset + cLayoutPopupWidth, cLayoutPopupVOffset + cPopupHeight), this, IDC_YEARVIEW_LAYOUT, IDC_STATIC, IDI_POPUPBTN);
	mLayout.SetMenu(IDR_POPUP_YEARLAYOUT);
	mLayout.SetFont(CMulberryApp::sAppFont);
	mLayoutValue = NCalendarView::e3x4;
	mLayout.SetValue(IDM_YEARLAYOUT_3_4);

	mTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
					CRect(0, cTopHeight, width, height), this, IDC_YEARVIEW_TABLE);
	mTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	AddAlignment(new CWndAlignment(&mTable, CWndAlignment::eAlign_WidthHeight));
	mTable.ResetFont(CFontCache::GetListFont());
	mTable.SetSuperCommander(this);
	mTable.SetCalendarView(this);
	mTable.Add_Listener(this);
	
	return 0;
}

CCalendarTableBase* CYearView::GetTable() const
{
	return const_cast<CYearTable*>(&mTable);
}

void CYearView::OnUpdateNewEvent(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CYearView::SetCaptions()
{
	// Set static text
	char buf[256];
	snprintf(buf, 256, "%ld", mDate.GetYear());
	CUnicodeUtils::SetWindowTextUTF8(&mYear, buf);
}

void CYearView::SetLayout(NCalendarView::EYearLayout layout)
{
	mLayoutValue = layout;
	mLayout.SetValue(mLayoutValue + IDM_YEARLAYOUT_1_12);
}

void CYearView::OnLayout()
{
	mLayoutValue = static_cast<NCalendarView::EYearLayout>(mLayout.GetValue() - IDM_YEARLAYOUT_1_12);
	ResetDate();
}

void CYearView::ResetFont(CFont* font)
{
}

void CYearView::DoPrint()
{
#ifdef _TODO
	std::auto_ptr<CYearPrintout> pout;

	try
	{
		pout.reset(static_cast<CYearPrintout*>(LPrintout::CreatePrintout(CYearPrintout::pane_ID)));
		pout->SetPrintSpec(*GetPrintSpec());
		pout->SetDetails(mDate.GetYear(), mLayoutValue);

		pout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
#endif
}
