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
#include "CContextMenu.h"
#include "CGUtils.h"
#include "CToDoItem.h"
#include "CToDoTable.h"

#include "CCalendarStoreManager.h"

#include <LBevelButton.h>
#include <LPopupButton.h>
#include <LScrollerView.h>

// ---------------------------------------------------------------------------
//	CToDoView														  [public]
/**
	Default constructor */

CToDoView::CToDoView(LStream *inStream) :
	CCalendarViewBase(inStream)
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

void CToDoView::FinishCreateSelf()
{
	// Get the UI objects
	mShowPopup = dynamic_cast<LPopupButton*>(FindPaneByID(eShowPopup_ID));
	mShowValue = eShowAllDue;
	
	mScroller = dynamic_cast<LScrollerView*>(FindPaneByID(eScroller_ID));

	mTable = dynamic_cast<CToDoTable*>(FindPaneByID(eTable_ID));
	mTable->SetCalendarView(this);
	mTable->Add_Listener(this);
	
	CContextMenuAttachment::AddUniqueContext(this, 1833, mTable);
	
	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

CCalendarTableBase* CToDoView::GetTable() const
{
	return mTable;
}

// Respond to clicks in the icon buttons
void CToDoView::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eShowPopup_ID:
		OnShowPopup();
		break;
	}
}

//	Respond to commands
Boolean CToDoView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
		OnNewToDo();
		break;

	default:
		cmdHandled = CCalendarViewBase::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CToDoView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
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

	default:
		CCalendarViewBase::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

void CToDoView::OnShowPopup()
{
	mShowValue = mShowPopup->GetValue();
	ResetDate();
}
