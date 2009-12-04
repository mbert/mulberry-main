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

#include "CYearView.h"

#include "CCommands.h"
#include "CCalendarUtils.h"
#include "CLog.h"
#include "CStaticText.h"
#include "CTableScrollbarSet.h"
#ifdef _TODO
#include "CYearPrintout.h"
#endif

#include "CCalendarStoreManager.h"

#include "TPopupMenu.h"
#include "JXMultiImageButton.h"

#include <cassert>

// ---------------------------------------------------------------------------
//	CYearView														  [public]
/**
	Default constructor */

CYearView::CYearView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
	CCalendarViewBase(enclosure, hSizing, vSizing, x, y, w, h)
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

void CYearView::OnCreate()
{
// begin JXLayout1

    mYearMinus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 24,22);
    assert( mYearMinus != NULL );

    mYear =
        new CStaticText("2005", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 36,7, 47,18);
    assert( mYear != NULL );
    mYear->SetFontSize(10);

    mYearPlus =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 90,5, 24,22);
    assert( mYearPlus != NULL );

    mLayout =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 138,5, 190,20);
    assert( mLayout != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,170);
    assert( sbs != NULL );

// end JXLayout1
	// Get the UI objects

	mYearMinus->SetImage(IDI_PREVMSG);
	mYearPlus->SetImage(IDI_NEXTMSG);

	mYear->SetBreakCROnly(false);
	mYear->SetFontAlign(CStaticText::kAlignCenter);

	mLayoutValue = NCalendarView::e3x4;
	mLayout->SetMenuItems("1 row x 12 columns %r | 2 rows x 6 columns %r | 3 rows x 4 columns %r | 4 rows x 3 columns %r | 6 rows x 2 columns %r | 12 rows x 1 column %r");
	mLayout->SetValue(mLayoutValue + 1);
	
	sbs->NoTitles();
	mTable = new CYearTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,0, 105, 100);
	mTable->OnCreate();

	mTable->SetCalendarView(this);
	mTable->Add_Listener(this);

	ListenTo(mYearMinus);
	ListenTo(mYearPlus);
	ListenTo(mLayout);
}

CCalendarTableBase* CYearView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CYearView::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mYearMinus)
		{
			OnPreviousYear();
			return;
		}		
		else if (sender == mYearPlus)
		{
			OnNextYear();
			return;
		}		
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mLayout)
		{
			OnLayout();
			return;
		}
	}

	CCalendarViewBase::Receive(sender, message);
}

//	Respond to commands
bool CYearView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
#ifdef _TODO
	case CCommand::eFilePageSetup:
		return true;

	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
		return true;
#endif

	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnNewEvent();
		return true;

	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
		OnThisYear();
		return true;

	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
		OnPickDate();
		return true;

	default:;
	};

	return CCalendarViewBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CYearView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
		OnUpdateNewEvent(cmdui);
		return;

	case CCommand::eFilePageSetup:
	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CCalendarViewBase::UpdateCommand(cmd, cmdui);
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
	::snprintf(buf, 256, "%ld", (long)mDate.GetYear());
	mYear->SetText(buf);
}

void CYearView::SetLayout(NCalendarView::EYearLayout layout)
{
	mLayoutValue = layout;
	mLayout->SetValue(mLayoutValue + 1);
}

void CYearView::OnLayout()
{
	mLayoutValue = static_cast<NCalendarView::EYearLayout>(mLayout->GetValue() - 1);
	ResetDate();
}

void CYearView::ResetFont(const SFontInfo& finfo)
{
}

void CYearView::DoPrint()
{
#ifdef _TODO
#endif
}
