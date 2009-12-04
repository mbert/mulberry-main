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

#include "CMonthIndicator.h"
#include "CMonthPopup.h"
#include "CMulberryApp.h"
#include "CYearPopup.h"

#include <JXWindow.h>
#include "JXMultiImageButton.h"
#include <JXUpRect.h>
#include <JXTextButton.h>

#include <jXGlobals.h>

#include <cassert>

// ---------------------------------------------------------------------------
//	CChooseDateDialog														  [public]
/**
	Default constructor */

CChooseDateDialog::CChooseDateDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
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

const int cTableWidth = 170;

void CChooseDateDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 200,300, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 200,300);
    assert( obj1 != NULL );

    mYearMinus =
        new JXMultiImageButton(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,12, 24,22);
    assert( mYearMinus != NULL );

    mYearPopup =
        new CYearPopup("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,10, 100,25);
    assert( mYearPopup != NULL );

    mYearPlus =
        new JXMultiImageButton(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 161,12, 24,22);
    assert( mYearPlus != NULL );

    mMonthMinus =
        new JXMultiImageButton(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,47, 24,22);
    assert( mMonthMinus != NULL );

    mMonthPopup =
        new CMonthPopup("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,45, 100,25);
    assert( mMonthPopup != NULL );

    mMonthPlus =
        new JXMultiImageButton(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 161,47, 24,22);
    assert( mMonthPlus != NULL );

    mTable =
        new CMonthIndicator(NULL,obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,80, 170,170);
    assert( mTable != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,260, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^M^[");

// end JXLayout
	window->SetTitle("Choose Date");
	SetButtons(mCancelBtn, mCancelBtn);

	mYearMinus->SetImage(IDI_PREVMSG);
	mYearPlus->SetImage(IDI_NEXTMSG);
	mMonthMinus->SetImage(IDI_PREVMSG);
	mMonthPlus->SetImage(IDI_NEXTMSG);

	mYearPopup->OnCreate();
	mMonthPopup->OnCreate();

	mTable->OnCreate();
	mTable->Add_Listener(this);

	ListenTo(mYearMinus);
	ListenTo(mYearPopup);
	ListenTo(mYearPlus);
	ListenTo(mMonthMinus);
	ListenTo(mMonthPopup);
	ListenTo(mMonthPlus);
}

void CChooseDateDialog::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CMonthIndicator::eBroadcast_DblClkDay:
		// Terminate dialog with the date
		mDateResult = *static_cast<iCal::CICalendarDateTime*>(param);
		mDone = true;
		EndDialog(true);
		break;
	}
}

// Handle OK button
void CChooseDateDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mYearMinus)
		{
			OnPrevYear();
			return;
		}
		else if (sender == mYearPlus)
		{
			OnNextYear();
			return;
		}
		else if (sender == mMonthMinus)
		{
			OnPrevMonth();
			return;
		}
		else if (sender == mMonthPlus)
		{
			OnNextMonth();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mYearPopup)
		{
			OnYearPopup();
			return;
		}
		else if (sender == mMonthPopup)
		{
			OnMonthPopup();
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

void CChooseDateDialog::SetDate(const iCal::CICalendarDateTime& dt)
{
	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt, cTableWidth);
	
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
	mTable->ResetTable(dt, cTableWidth);
}

void CChooseDateDialog::OnNextYear()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year + 1, month, 1);

	mYearPopup->SetYear(dt.GetYear());
	mTable->ResetTable(dt, cTableWidth);
}

void CChooseDateDialog::OnPrevMonth()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(-1);

	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt, cTableWidth);
}

void CChooseDateDialog::OnNextMonth()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);
	dt.OffsetMonth(1);

	mYearPopup->SetYear(dt.GetYear());
	mMonthPopup->SetMonth(dt.GetMonth());
	mTable->ResetTable(dt, cTableWidth);
}

void CChooseDateDialog::OnYearPopup()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable->ResetTable(dt, cTableWidth);
}

void CChooseDateDialog::OnMonthPopup()
{
	int32_t year = mYearPopup->GetYear();
	int32_t month = mMonthPopup->GetMonth();

	iCal::CICalendarDateTime dt(year, month, 1);

	mTable->ResetTable(dt, cTableWidth);
}

bool CChooseDateDialog::PoseDialog(iCal::CICalendarDateTime& dt)
{
	bool result = false;

	// Create the dialog
	CChooseDateDialog* dlog = new CChooseDateDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDate(dt);
	
	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		result = dlog->mDone;
		if (result)
			dlog->GetDate(dt);
		dlog->Close();
	}

	return result;
}
