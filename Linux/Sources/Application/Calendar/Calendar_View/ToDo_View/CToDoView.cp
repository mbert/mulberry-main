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

#include "CToDoView.h"

#include "CCalendarView.h"
#include "CCommands.h"
#include "CTableScrollbarSet.h"
#include "CToDoItem.h"
#include "CToDoTable.h"

#include "CCalendarStoreManager.h"

#include "TPopupMenu.h"

#include <JXStaticText.h>

#include <cassert>

// ---------------------------------------------------------------------------
//	CToDoView														  [public]
/**
	Default constructor */

CToDoView::CToDoView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
	CCalendarViewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}


// ---------------------------------------------------------------------------
//	~CToDoView														  [public]
/**
	Destructor */

CToDoView::~CToDoView()
{
}

#pragma mark -

void CToDoView::OnCreate()
{
	// Get the UI objects
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Show:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 8,5, 53,16);
    assert( obj1 != NULL );

    mShowPopup =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 50,5, 190,20);
    assert( mShowPopup != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 500,170);
    assert( sbs != NULL );

// end JXLayout1

	mShowPopup->SetMenuItems("All Due %r | All %r | Due Today %r | Due this Week %r | Overdue %r");
	mShowValue = eShowAllDue;
	mShowPopup->SetValue(mShowValue);
	
	sbs->NoTitles();
	mTable = new CToDoTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,0, 105, 100);
	mTable->OnCreate();
	mTable->SetCalendarView(this);
	mTable->Add_Listener(this);
	
	// Listen to some controls
	ListenTo(mShowPopup);
}

CCalendarTableBase* CToDoView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CToDoView::Receive(JBroadcaster* sender, const Message& message) 
{
	if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mShowPopup)
		{
			OnShowPopup();
			return;
		}
	}
}

//	Respond to commands
bool CToDoView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
		OnNewToDo();
		return true;

	default:;
	}

	return CCalendarViewBase::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CToDoView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
		OnUpdateNewToDo(cmdui);
		return;

	default:;
	}

	CCalendarViewBase::UpdateCommand(cmd, cmdui);
}

void CToDoView::OnUpdateNewToDo(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((GetCalendar() != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CToDoView::OnShowPopup()
{
	mShowValue = mShowPopup->GetValue();
	ResetDate();
}

void CToDoView::ResetFont(const SFontInfo& finfo)
{
}

