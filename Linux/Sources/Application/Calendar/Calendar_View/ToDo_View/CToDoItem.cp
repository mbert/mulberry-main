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

#include "CToDoItem.h"

#include "CCommands.h"
#include "CCalendarTableBase.h"
#include "CCalendarUtils.h"
#include "CCalendarViewBase.h"
#include "CDrawUtils.h"
#include "CMulberryCommon.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentExpanded.h"

#include <JXColormap.h>
#include <JXTextCheckbox.h>
#include <JXWindowPainter.h>
#include <jASCIIConstants.h>

#include <strstream>

const JCoordinate cItemHeight = 32;
const JCoordinate cCheckboxLeftOffset = 2;
const JCoordinate cCheckboxTopOffset = 4;
const JCoordinate cCheckboxWidth = 16;
const JCoordinate cCheckboxHeight = 12;
const JCoordinate cCheckboxSize = 16;
const JCoordinate cTextTopOffset = 2;

// ---------------------------------------------------------------------------
//	CToDoItem														  [public]
/**
	Default constructor */

CToDoItem::CToDoItem(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CCommander(enclosure), JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h), CContextMenu(this)
{
	mCompleted = NULL;
	mType = eToDo;
	mTable = NULL;
	mIsSelected = false;
	mIsCompleted = false;
	mColour = 0;
}


// ---------------------------------------------------------------------------
//	~CToDoItem														  [public]
/**
	Destructor */

CToDoItem::~CToDoItem()
{
}

CToDoItem* CToDoItem::Create(JXContainer* enclosure, const JRect& frame)
{
	CToDoItem* result = new CToDoItem(enclosure, kFixedLeft, kFixedTop, frame.left, frame.top, frame.width(), frame.height());
	try
	{
		result->OnCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

#pragma mark -

void CToDoItem::OnCreate()
{
	JRect frame = GetAperture();

	// Create the checkbox - top-left corner of control
	frame.left += cCheckboxLeftOffset;
	frame.top += cCheckboxTopOffset;
	frame.right = frame.left + cCheckboxWidth;
	frame.bottom = frame.top + cCheckboxHeight;

	mCompleted = new JXTextCheckbox("", this, kFixedLeft, kFixedTop, frame.left, frame.top, frame.width(), frame.height());
	ListenTo(mCompleted);

	CreateContextMenu(CMainMenu::eContextCalendarToDoItem);
}

// Respond to clicks
void CToDoItem::Receive(JBroadcaster* sender, const Message& message)
{
	// Check for context menu
	if (!ReceiveMenu(sender, message))
	{
		if (message.Is(JXCheckbox::kPushed))
		{
			if (sender == mCompleted)
			{
				OnCompleted();
			}
		}
	}
}

//	Respond to commands
bool CToDoItem::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eCalendarEditItem:
		OnEditToDo();
		return true;

	case CCommand::eCalendarDuplicateItem:
		OnDuplicateToDo();
		return true;

	case CCommand::eCalendarDeleteItem:
		OnDeleteToDo();
		return true;

	case CCommand::eCalendarCompleted:
		OnChangeCompleted();
		return true;

	default:;
	};

	return CCommander::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CToDoItem::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eCalendarEditItem:
	case CCommand::eCalendarDuplicateItem:
	case CCommand::eCalendarDeleteItem:
		OnUpdateRealToDo(cmdui);
		return;

	case CCommand::eCalendarCompleted:
		OnUpdateCompletedToDo(cmdui);
		return;

	default:;
	}

	CCommander::UpdateCommand(cmd, cmdui);
}

void CToDoItem::OnUpdateRealToDo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mType == eToDo);
}

void CToDoItem::OnUpdateCompletedToDo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mType == eToDo);
	pCmdUI->SetCheck(mIsCompleted);
}

void CToDoItem::SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table)
{
	mVToDo = todo;
	mType = eToDo;
	mTable = table;
	mSummary = todo->GetMaster<iCal::CICalendarVToDo>()->GetSummary();
	mStatus = todo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Setup a help tag
	SetupTagText();

	// Set the completed status (do without handling the change of value)
	{
		StStopListening _no_listen(this);
		switch(todo->GetMaster<iCal::CICalendarVToDo>()->GetStatus())
		{
		default:
			mCompleted->SetState(kFalse);
			break;
		case iCal::eStatus_VToDo_Completed:
			mIsCompleted = true;
			mCompleted->SetState(kTrue);
			break;
		case iCal::eStatus_VToDo_Cancelled:
			mCompleted->SetVisible(kFalse);
			break;
		}
	}
	
	// Determine colour
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
	if (cal)
	{
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}
}

void CToDoItem::SetDetails(EType type)
{
	mVToDo.reset();
	mType = type;
	mTable = NULL;
	
	mSummary = rsrc::GetIndexedString("CToDoItem::When", type);
	
	// Hide checkbox
	mCompleted->SetVisible(kFalse);
	
	// Turn off tooltip
	//EnableTooltips(false);
}

void CToDoItem::HandleKeyPress(const int key, const JXKeyModifiers& modifiers)
{
	// Return => edit
	switch(key)
	{
	case kJReturnKey:
		// Only if real to do
		if (mType == eToDo)
			OnEditToDo();
		return;
	}
}

// Click
void CToDoItem::HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Only if real to do
	if (mType == eToDo)
	{
		bool was_selected = mIsSelected;
		
		// Look for toggle of state
		if (!modifiers.shift() && !modifiers.control())
			SelectToDo(!was_selected);

		// Broadcast double-click if in month area
		if ((clickCount > 1) && (button == kJXLeftButton))	
		{
			OnEditToDo();
		}
		
		if ((clickCount == 1) && (button == kJXRightButton))
			ContextEvent(pt, buttonStates, modifiers);
	}
}

void CToDoItem::HandleFocusEvent()
{
	JXWidgetSet::HandleFocusEvent();

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(true);
}

void CToDoItem::HandleUnfocusEvent()
{
	JXWidgetSet::HandleUnfocusEvent();

	// Only if real to do
	if (mType == eToDo)
		SelectToDo(false);
}

void CToDoItem::Draw(JXWindowPainter& p, const JRect& rect)
{
	JRect actual_rect(rect);
	JColorIndex cindex;
	if (mType == eToDo)
	{
		// Use unsaturated colour for selected item
		float red = CCalendarUtils::GetRed(mColour);
		float green = CCalendarUtils::GetGreen(mColour);
		float blue = CCalendarUtils::GetBlue(mColour);
		if (not mIsSelected)
			CCalendarUtils::LightenColours(red, green, blue);
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(red, green, blue), &cindex);
		p.SetPenColor(cindex);
		p.SetFilling(true);
		p.JPainter::Rect(actual_rect);
		p.SetFilling(false);
	}
	else
	{
		CDrawUtils::DrawSimpleBorder(p, actual_rect, false, true);
		p.SetPenColor(GetColormap()->GetGray50Color());
		actual_rect.Shrink(0, 3);
	}
	
	// Draw summary text
	actual_rect.Shrink(3, 0);
	JRect box(actual_rect);
	if (mType == eToDo)
		box.left += cCheckboxLeftOffset + cCheckboxSize;
	box.bottom = box.top + cItemHeight / 2;
	float red = CCalendarUtils::GetRed(mColour);
	float green = CCalendarUtils::GetGreen(mColour);
	float blue = CCalendarUtils::GetBlue(mColour);
	if (mIsSelected)
	{
		cindex = (red + green + blue > 2.5) ? p.GetColormap()->GetBlackColor() : p.GetColormap()->GetWhiteColor();
	}
	else
	{
		CCalendarUtils::DarkenColours(red, green, blue);
		JRGB fill = CCalendarUtils::GetRGBColor(red, green, blue);
		GetColormap()->JColormap::AllocateStaticColor(fill, &cindex);
	}
	p.SetPenColor(cindex);
	::DrawClippedStringUTF8(&p, mSummary, JPoint(box.left, box.top), box, (mType == eToDo) ? eDrawString_Left : eDrawString_Center);

	// Draw status text
	box = actual_rect;
	box.top += cItemHeight / 2;
	::DrawClippedStringUTF8(&p, mStatus, JPoint(box.left, box.top), box, (mType == eToDo) ? eDrawString_Left : eDrawString_Center);
}

void CToDoItem::OnChangeCompleted()
{
	// Toggle checkbox to trigger action
	mCompleted->ToggleState();
}

void CToDoItem::OnCompleted()
{
	// Only if real to do
	if (mType != eToDo)
		return;

	// Change the completed status on the event but do not force an update to the view right now
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->EditStatus(mCompleted->IsChecked() ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	mVToDo->GetMaster<iCal::CICalendarVToDo>()->Changed();
	mIsCompleted = mCompleted->IsChecked();
	mStatus = mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetStatusText();
	
	// Redo tooltip for change
	SetupTagText();
	
	// Refresh as status may have changed
	FRAMEWORK_REFRESH_WINDOW(this);
}

