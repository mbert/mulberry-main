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


//	CACLTable.cp

#include "CACLTable.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPropAdbkACL.h"
#include "CPropCalendarACL.h"
#include "CPropMailboxACL.h"
#include "CResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

CACLTable::CACLTable(LStream *inStream)
	: CTableDrag(inStream)

{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
	mReadWrite = false;
	SetOneColumnSelect(1);
}


CACLTable::~CACLTable()
{
}

// Get details of sub-panes
void CACLTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
}

void CACLTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Get item
	CACL& rights = (mMbox ? (CACL&) mMboxACLs->at(inCell.row - 1) : (mAdbk ? (CACL&) mAdbkACLs->at(inCell.row - 1) : (CACL&) mCalACLs->at(inCell.row - 1)));

	// Look for calendar unchangeable rights
	bool can_change = true;
	if (mCalendar)
	{
		can_change = static_cast<CCalendarACL&>(rights).CanChange();
	}

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;
	ResIDT			icon;
	ResIDT			icon_on = (mReadWrite && can_change) ? ICNx_DiamondTicked : ICNx_ACLSet;
	ResIDT			icon_off = (mReadWrite && can_change) ? ICNx_Diamond : ICNx_ACLUnset;

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	switch(inCell.col)
	{

	case 1:
		// Get name of item
		theTxt = rights.GetDisplayUID();
		::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
		short width = inLocalRect.right - inLocalRect.left;
		::DrawClippedStringUTF8(theTxt, width, eDrawString_Left);
		break;

	case 2:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Lookup : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Lookup : (ResIDT)CCalendarACL::eCalACL_ReadFreeBusy)) ? icon_on : icon_off;
		break;

	case 3:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Read : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Read : (ResIDT)CCalendarACL::eCalACL_Read)) ? icon_on : icon_off;
		break;

	case 4:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Seen : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Write : (ResIDT)CCalendarACL::eCalACL_Write)) ? icon_on : icon_off;
		break;

	case 5:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Write : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Create : (ResIDT)CCalendarACL::eCalACL_Schedule)) ? icon_on : icon_off;
		break;

	case 6:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Insert : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Delete : (ResIDT)CCalendarACL::eCalACL_Create)) ? icon_on : icon_off;
		break;

	case 7:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Post : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Admin : (ResIDT)CCalendarACL::eCalACL_Delete)) ? icon_on : icon_off;
		break;

	case 8:
		// Determine icon
		icon = rights.HasRight(mMbox ? (ResIDT)CMboxACL::eMboxACL_Create : (ResIDT)CCalendarACL::eCalACL_Admin) ? icon_on : icon_off;
		break;

	case 9:
		// Determine icon
		icon = rights.HasRight((ResIDT)CMboxACL::eMboxACL_Delete) ? icon_on : icon_off;
		break;

	case 10:
		// Determine icon
		icon = rights.HasRight((ResIDT)CMboxACL::eMboxACL_Admin) ? icon_on : icon_off;
		break;

	default:
		break;
	}

	if (inCell.col > 1)
	{
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		::Ploticns(&iconRect, atNone, IsActive() ? ttNone : ttDisabled, icon);
	}
}

// Draw or undraw active hiliting for a Cell
void CACLTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	if (mMboxACLs)
		CTableDrag::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CACLTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	if (mMboxACLs)
		CTableDrag::HiliteCellInactively(inCell, inHilite);
}

// Option click does not select
Boolean CACLTable::ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	// Special for button columns
	if ((inCell.col == 1) || !(inMouseDown.macEvent.modifiers & optionKey))
		return CTableDrag::ClickSelect(inCell, inMouseDown);
	else
		return true;
}

// Handle button hit
void CACLTable::ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	// Only if admin allowed and not click-through
	if (!mReadWrite || inMouseDown.delaySelect)
		return;

	long right;
	bool set;

	// Get item
	CACL& rights = (mMbox ? (CACL&) mMboxACLs->at(inCell.row - 1) : (mAdbk ? (CACL&) mAdbkACLs->at(inCell.row - 1) : (CACL&) mCalACLs->at(inCell.row - 1)));

	// Look for calendar unchangeable rights
	if (mCalendar)
	{
		if (!static_cast<CCalendarACL&>(rights).CanChange())
			return;
	}

	switch(inCell.col)
	{
	case 2:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Lookup : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Lookup : (ResIDT)CCalendarACL::eCalACL_ReadFreeBusy));
		set = rights.HasRight(right) ? false : true;
		break;
	case 3:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Read : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Read : (ResIDT)CCalendarACL::eCalACL_Read));
		set = rights.HasRight(right) ? false : true;
		break;
	case 4:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Seen : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Write : (ResIDT)CCalendarACL::eCalACL_Write));
		set = rights.HasRight(right) ? false : true;
		break;
	case 5:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Write : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Create : (ResIDT)CCalendarACL::eCalACL_Schedule));
		set = rights.HasRight(right) ? false : true;
		break;
	case 6:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Insert : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Delete : (ResIDT)CCalendarACL::eCalACL_Create));
		set = rights.HasRight(right) ? false : true;
		break;
	case 7:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Post : (mAdbk ? (ResIDT)CAdbkACL::eAdbkACL_Admin : (ResIDT)CCalendarACL::eCalACL_Delete));
		set = rights.HasRight(right) ? false : true;
		break;
	case 8:
		right = (mMbox ? (ResIDT)CMboxACL::eMboxACL_Create : (ResIDT)CCalendarACL::eCalACL_Admin);
		set = rights.HasRight(right) ? false : true;
		break;
	case 9:
		right = (ResIDT)CMboxACL::eMboxACL_Delete;
		set = rights.HasRight(right) ? false : true;
		break;
	case 10:
		right = (ResIDT)CMboxACL::eMboxACL_Admin;
		set = rights.HasRight(right) ? false : true;
		break;
	default:
		return;
	}

	// Send message to parent
	LView* super = GetSuperView();
	while(super && (mMbox ? (dynamic_cast<CPropMailboxACL*>(super) == NULL) : (mAdbk ? (dynamic_cast<CPropAdbkACL*>(super) == NULL) : (dynamic_cast<CPropCalendarACL*>(super) == NULL))))
		super = super->GetSuperView();
	if (super)
	{
		if (mMbox)
			static_cast<CPropMailboxACL*>(super)->DoChangeACL((CMboxACL::EMboxACL) right, set);
		else if (mAdbk)
			static_cast<CPropAdbkACL*>(super)->DoChangeACL((CAdbkACL::EAdbkACL) right, set);
		else if (mCalendar)
			static_cast<CPropCalendarACL*>(super)->DoChangeACL((CCalendarACL::ECalACL) right, set);
	}
}

void CACLTable::SetList(CCalendarACLList* aList, bool read_write)
{
	mCalACLs = aList;
	mMbox = false;
	mAdbk = false;
	mCalendar = true;
	mReadWrite = read_write;

	// Create columns
	if (!mCols)
	{
		InsertCols(8, 1, nil, 0, false);
		AdaptToNewSurroundings();
	}

	// Create entries after removing all rows
	RemoveRows(mRows, 1, false);
	if (mCalACLs)
		InsertRows(mCalACLs->size(), 0, nil, 0, true);
	
	Refresh();
}

void CACLTable::SetList(CAdbkACLList* aList, bool read_write)
{
	mAdbkACLs = aList;
	mMbox = false;
	mAdbk = true;
	mCalendar = false;
	mReadWrite = read_write;

	// Create columns
	if (!mCols)
	{
		InsertCols(7, 1, nil, 0, false);
		AdaptToNewSurroundings();
	}

	// Create entries after removing all rows
	RemoveRows(mRows, 1, false);
	if (mAdbkACLs)
		InsertRows(mAdbkACLs->size(), 0, nil, 0, true);
	
	Refresh();
}

void CACLTable::SetList(CMboxACLList* aList, bool read_write)
{
	mMboxACLs = aList;
	mMbox = true;
	mAdbk = false;
	mCalendar = false;
	mReadWrite = read_write;

	// Create columns
	if (!mCols)
	{
		InsertCols(10, 1, nil, 0, false);
		AdaptToNewSurroundings();
	}

	// Create entries after removing all rows
	RemoveRows(mRows, 1, false);
	if (mMboxACLs)
		InsertRows(mMboxACLs->size(), 0, nil, 0, true);
	
	Refresh();
}

// Adjust column widths
void CACLTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CTableDrag::AdaptToNewSurroundings();

	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	if (mMbox)
	{
		// Name column has variable width
		SetColWidth(mFrameSize.width - 171, 1, 1);

		// Remaining columns have fixed width
		SetColWidth(19, 2, 10);
	}
	else if (mAdbk)
	{
		// Name column has variable width
		SetColWidth(mFrameSize.width - 114, 1, 1);

		// Remaining columns have fixed width
		SetColWidth(19, 2, 7);
	}
	else if (mCalendar)
	{
		// Name column has variable width
		SetColWidth(mFrameSize.width - 133, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(19, 2, 8);
	}

}
