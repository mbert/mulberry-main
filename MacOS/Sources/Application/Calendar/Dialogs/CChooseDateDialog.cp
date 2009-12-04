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

#include "CBalloonDialog.h"
#include "CMonthIndicator.h"
#include "CMonthPopup.h"
#include "CMulberryApp.h"
#include "CYearPopup.h"

#include <LBevelButton.h>

// ---------------------------------------------------------------------------
//	CChooseDateDialog														  [public]
/**
	Default constructor */

CChooseDateDialog::CChooseDateDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mDone = false;
}


// ---------------------------------------------------------------------------
//	~CChooseDateDialog														  [public]
/**
	Destructor */

CChooseDateDialog::~CChooseDateDialog()
{
}

#pragma mark -

void CChooseDateDialog::FinishCreateSelf()
{
	LDialogBox::FinishCreateSelf();

	// Get UI items
	mYearMinus = dynamic_cast<LBevelButton*>(FindPaneByID(ePrevYear_ID));
	mYearPopup = dynamic_cast<CYearPopup*>(FindPaneByID(eYearPopup_ID));
	mYearPlus = dynamic_cast<LBevelButton*>(FindPaneByID(eNextYear_ID));

	mMonthMinus = dynamic_cast<LBevelButton*>(FindPaneByID(ePrevMonth_ID));
	mMonthPopup = dynamic_cast<CMonthPopup*>(FindPaneByID(eMonthPopup_ID));
	mMonthPlus = dynamic_cast<LBevelButton*>(FindPaneByID(eNextMonth_ID));

	mTable = dynamic_cast<CMonthIndicator*>(FindPaneByID(eTable_ID));
	mTable->Add_Listener(this);

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
	
	// Focus on table
	SwitchTarget(mTable);
}

void CChooseDateDialog::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CMonthIndicator::eBroadcast_DblClkDay:
		// Terminate dialog with the date
		mDateResult = *static_cast<iCal::CICalendarDateTime*>(param);
		mDone = true;
		break;
	}
}

// Handle OK button
void CChooseDateDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case ePrevYear_ID:
		OnPrevYear();
		break;

	case eNextYear_ID:
		OnNextYear();
		break;

	case ePrevMonth_ID:
		OnPrevMonth();
		break;

	case eNextMonth_ID:
		OnNextMonth();
		break;

	case eYearPopup_ID:
		OnYearPopup();
		break;

	case eMonthPopup_ID:
		OnMonthPopup();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CChooseDateDialog::SetDate(const iCal::CICalendarDateTime& dt)
{
	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt);
	
	// Select the cell with the chosen date
	mTable->SelectDate(dt);
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
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year - 1, month, 1);

	mYearPopup->SetYear(dt.GetYear());
	mTable->ResetTable(dt);
}

void CChooseDateDialog::OnNextYear()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year + 1, month, 1);

	mYearPopup->SetYear(dt.GetYear());
	mTable->ResetTable(dt);
}

void CChooseDateDialog::OnPrevMonth()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(-1);

	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt);
}

void CChooseDateDialog::OnNextMonth()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(1);

	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt);
}

void CChooseDateDialog::OnYearPopup()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable->ResetTable(dt);
}

void CChooseDateDialog::OnMonthPopup()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable->ResetTable(dt);
}

bool CChooseDateDialog::PoseDialog(iCal::CICalendarDateTime& dt)
{
	// Create the dialog
	CBalloonDialog theHandler(pane_ID, CMulberryApp::sApp);

	CChooseDateDialog* dlog = static_cast<CChooseDateDialog*>(theHandler.GetDialog());
	dlog->SetDate(dt);
	
	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (dlog->mDone)
		{
			dlog->GetDate(dt);
			return true;
		}
		else if (hitMessage == msg_Cancel)
		{
			return false;
		}
	}
}
