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

#include "CChooseDateDialog.h"

#include "CSDIFrame.h"

// ---------------------------------------------------------------------------
//	CChooseDateDialog														  [public]
/**
	Default constructor */

CChooseDateDialog::CChooseDateDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CChooseDateDialog::IDD, pParent)
{
}


// ---------------------------------------------------------------------------
//	~CChooseDateDialog														  [public]
/**
	Destructor */

CChooseDateDialog::~CChooseDateDialog()
{
}

#pragma mark -

void CChooseDateDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDateDialog)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		GetDate(mDateResult);
	}
	else
	{
		InitControls();
		SetDate(mDateResult);
	}
}

BEGIN_MESSAGE_MAP(CChooseDateDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CChooseDateDialog)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_PREVYEAR, OnPrevYear)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_YEAR, OnYearPopup)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_NEXTYEAR, OnNextYear)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_PREVMONTH, OnPrevMonth)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_MONTH, OnMonthPopup)
	ON_COMMAND(IDC_CALENDAR_CHOOSEDATE_NEXTMONTH, OnNextMonth)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

void CChooseDateDialog::InitControls()
{
	// Get UI items
	mYearMinus.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_PREVYEAR, this, IDI_PREVMSG);
	mYearPopup.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_YEAR, this);
	mYearPlus.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_NEXTYEAR, this, IDI_NEXTMSG);

	mMonthMinus.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_PREVMONTH, this, IDI_PREVMSG);
	mMonthPopup.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_MONTH, this);
	mMonthPlus.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_NEXTMONTH, this, IDI_NEXTMSG);

	mTable.SubclassDlgItem(IDC_CALENDAR_CHOOSEDATE_TABLE, this);
	mTable.Add_Listener(this);
}

void CChooseDateDialog::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CMonthIndicator::eBroadcast_DblClkDay:
		// Terminate dialog with the date
		mDateResult = *static_cast<iCal::CICalendarDateTime*>(param);
		EndDialog(IDOK);
		break;
	}
}

void CChooseDateDialog::SetDate(const iCal::CICalendarDateTime& dt)
{
	mYearPopup.SetYear(dt.GetYear());
	mMonthPopup.SetMonth(dt.GetMonth());
	mTable.ResetTable(dt);
	
	// Select the cell with the chosen date
	mTable.SelectDate(dt);
}

void CChooseDateDialog::GetDate(iCal::CICalendarDateTime& dt)
{
	// Change the date only - leave the time alone
	dt.SetYear(mDateResult.GetYear());
	dt.SetMonth(mDateResult.GetMonth());
	dt.SetDay(mDateResult.GetDay());
}

void CChooseDateDialog::OnPrevYear()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year - 1, month, 1);

	mYearPopup.SetYear(dt.GetYear());
	mTable.ResetTable(dt);
	RedrawWindow();
}

void CChooseDateDialog::OnNextYear()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year + 1, month, 1);

	mYearPopup.SetYear(dt.GetYear());
	mTable.ResetTable(dt);
	RedrawWindow();
}

void CChooseDateDialog::OnPrevMonth()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(-1);

	mYearPopup.SetYear(dt.GetYear());
	mMonthPopup.SetMonth(dt.GetMonth());
	mTable.ResetTable(dt);
	RedrawWindow();
}

void CChooseDateDialog::OnNextMonth()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(1);

	mYearPopup.SetYear(dt.GetYear());
	mMonthPopup.SetMonth(dt.GetMonth());
	mTable.ResetTable(dt);
	RedrawWindow();
}

void CChooseDateDialog::OnYearPopup()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable.ResetTable(dt);
	RedrawWindow();
}

void CChooseDateDialog::OnMonthPopup()
{
	int32_t year = mYearPopup.GetYear();
	int32_t month = mMonthPopup.GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable.ResetTable(dt);
	RedrawWindow();
}

bool CChooseDateDialog::PoseDialog(iCal::CICalendarDateTime& dt)
{
	// Create the dialog
	CChooseDateDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mDateResult = dt;
	
	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetDate(dt);
		return true;
	}

	return false;
}
