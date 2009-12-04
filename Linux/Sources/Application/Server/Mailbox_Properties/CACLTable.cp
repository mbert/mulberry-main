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

#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPropAdbkACL.h"
#include "CPropCalendarACL.h"
#include "CPropMailboxACL.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include "StPenState.h"

#include <JColormap.h>
#include <JXImage.h>

CACLTable::CACLTable(
				JXScrollbarSet* scrollbarSet, 
				JXContainer* enclosure,
				const HSizingOption hSizing, 
				const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mReadWrite = false;

	// Load strings if not already
	mCurrentUser.FromResource("UI::MailboxProp::CurrentUser");

	// Load icons
	mIcons.push_back(CIconLoader::GetIcon(IDI_DIAMONDTICKED, this, 16, 0x00FFFFFF));
	mIcons.push_back(CIconLoader::GetIcon(IDI_DIAMOND, this, 16, 0x00FFFFFF));
	mIcons.push_back(CIconLoader::GetIcon(IDI_ACL_SET, this, 16, 0x00FFFFFF));
	mIcons.push_back(CIconLoader::GetIcon(IDI_ACL_UNSET, this, 16, 0x00FFFFFF));
	
	mMboxACLs = NULL;

	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
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
void CACLTable::OnCreate()
{
	// Do inherited
	CTableDragAndDrop::OnCreate();

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
}

void CACLTable::SetList(CCalendarACLList* aList, bool read_write)
{
	// Create columns first time through
	if (!mCols)
	{
		// Create columns
		InsertCols(8, 1, NULL, 0, false);
		
		// Name column has variable width
		SetColWidth(GetApertureWidth() - 105, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(21, 2, 8);
	}

	mCalACLs = aList;
	mMbox = false;
	mAdbk = false;
	mCalendar = true;
	mReadWrite = read_write;

	// Delete all existing rows
	RemoveRows(mRows, 1, true);

	// Create entries
	if (mCalACLs)
		InsertRows(mCalACLs->size() + 1, 0, nil, 0, true);
}

void CACLTable::SetList(CAdbkACLList* aList, bool read_write)
{
	// Create columns first time through
	if (!mCols)
	{
		// Create columns
		InsertCols(7, 1, NULL, 0, false);
		
		// Name column has variable width
		SetColWidth(GetApertureWidth() - 126, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(21, 2, 7);
	}

	mAdbkACLs = aList;
	mMbox = false;
	mAdbk = true;
	mCalendar = false;
	mReadWrite = read_write;

	// Delete all existing rows
	RemoveRows(mRows, 1, true);

	// Create entries
	if (mAdbkACLs)
		InsertRows(mAdbkACLs->size() + 1, 0, nil, 0, true);
}

void CACLTable::SetList(CMboxACLList* aList, bool read_write)
{
	// Create columns first time through
	if (!mCols)
	{
		// Create columns
		InsertCols(10, 1, NULL, 0, false);
		
		// Name column has variable width
		SetColWidth(GetApertureWidth() - 189, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(21, 2, 10);
	}

	mMboxACLs = aList;
	mMbox = true;
	mAdbk = false;
	mCalendar = false;
	mReadWrite = read_write;

	// Delete all existing rows
	RemoveRows(mRows, 1, true);

	// Create entries
	if (mMboxACLs)
		InsertRows(mMboxACLs->size() + 1, 0, nil, 0, true);
}

// Option click does not select
bool CACLTable::ClickSelect(const STableCell &inCell, const JXKeyModifiers& modifiers)
{
	// Do not select when clicking button columns
	if (inCell.col == 1)
		return CTableDragAndDrop::ClickSelect(inCell, modifiers);
	else
		return true;
}

// Clicked item
void CACLTable::LClickCell(const STableCell &inCell, const JXKeyModifiers& modifiers)
{
	// Only if admin allowed and not 'Current User' row
	if (!mReadWrite || (inCell.row == 1))
		return;

	// Do inherited action first
	CTableDragAndDrop::LClickCell(inCell, modifiers);

	long right;
	bool set;

	// Get item
	CACL& rights = (mMbox ? (CACL&) mMboxACLs->at(inCell.row - 2) : (mAdbk ? (CACL&) mAdbkACLs->at(inCell.row - 2) : (CACL&) mCalACLs->at(inCell.row - 2)));

	// Look for calendar unchangeable rights
	if (mCalendar)
	{
		if (!static_cast<CCalendarACL&>(rights).CanChange())
			return;
	}

	switch(inCell.col)
	{
	case 2:
		right = (mMbox ? CMboxACL::eMboxACL_Lookup : (mAdbk ? CAdbkACL::eAdbkACL_Lookup : CCalendarACL::eCalACL_ReadFreeBusy));
		set = rights.HasRight(right) ? false : true;
		break;
	case 3:
		right = (mMbox ? CMboxACL::eMboxACL_Read : (mAdbk ? CAdbkACL::eAdbkACL_Read : CCalendarACL::eCalACL_Read));
		set = rights.HasRight(right) ? false : true;
		break;
	case 4:
		right = (mMbox ? CMboxACL::eMboxACL_Seen : (mAdbk ? CAdbkACL::eAdbkACL_Write : CCalendarACL::eCalACL_Write));
		set = rights.HasRight(right) ? false : true;
		break;
	case 5:
		right = (mMbox ? CMboxACL::eMboxACL_Write : (mAdbk ? CAdbkACL::eAdbkACL_Create : CCalendarACL::eCalACL_Schedule));
		set = rights.HasRight(right) ? false : true;
		break;
	case 6:
		right = (mMbox ? CMboxACL::eMboxACL_Insert : (mAdbk ? CAdbkACL::eAdbkACL_Delete : CCalendarACL::eCalACL_Create));
		set = rights.HasRight(right) ? false : true;
		break;
	case 7:
		right = (mMbox ? CMboxACL::eMboxACL_Post : (mAdbk ? CAdbkACL::eAdbkACL_Admin : CCalendarACL::eCalACL_Delete));
		set = rights.HasRight(right) ? false : true;
		break;
	case 8:
		right = (mMbox ? CMboxACL::eMboxACL_Create : CCalendarACL::eCalACL_Admin);
		set = rights.HasRight(right) ? false : true;
		break;
	case 9:
		right = CMboxACL::eMboxACL_Delete;
		set = rights.HasRight(right) ? false : true;
		break;
	case 10:
		right = CMboxACL::eMboxACL_Admin;
		set = rights.HasRight(right) ? false : true;
		break;
	default:
		return;
	}
	
	// Send message to parent
	JXContainer* parent = const_cast<JXContainer*>(GetEnclosure()->GetEnclosure()->GetEnclosure());
	if (mMbox)
		((CPropMailboxACL*) parent)->DoChangeACL((CMboxACL::EMboxACL) right, set);
	else if (mAdbk)
		((CPropAdbkACL*) parent)->DoChangeACL((CAdbkACL::EAdbkACL) right, set);
	else if (mCalendar)
		((CPropCalendarACL*) parent)->DoChangeACL((CCalendarACL::ECalACL) right, set);
}

// Set indicators from rights
void CACLTable::SetMyRights(SACLRight rights)
{
	mMyRights = rights;
	if (mRows)
		RefreshRow(1);
}

void CACLTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState save(pDC);

	cdstring name;
	SACLRight rights;
	int icon;
	int icon_on;
	int icon_off;

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get item
	if (inCell.row > 1)
	{
		CACL& acls = (mMbox ? (CACL&) mMboxACLs->at(inCell.row - 2) : (mAdbk ? (CACL&) mAdbkACLs->at(inCell.row - 2) : (CACL&) mCalACLs->at(inCell.row - 2)));
		name = acls.GetDisplayUID();
		rights = acls.GetRights();

		// Look for calendar unchangeable rights
		bool can_change = true;
		if (mCalendar)
		{
			can_change = static_cast<CCalendarACL&>(acls).CanChange();
		}

		icon_on = (mReadWrite && can_change) ? eACLDiamondTickedIcon : eACLSetIcon;
		icon_off = (mReadWrite && can_change) ? eACLDiamondIcon : eACLUnsetIcon;
	}
	else
	{
		name = mCurrentUser;
		rights = mMyRights;
		icon_on = eACLSetIcon;
		icon_off = eACLUnsetIcon;

		// Special background for current user
		StPenState save(pDC);
		pDC->SetPenColor(pDC->GetColormap()->Get3DShadeColor());
		pDC->SetFilling(kTrue);
		pDC->RectInside(inLocalRect);
	}

	switch(inCell.col)
	{
	
	case 1:
		// Draw name of item
		::DrawClippedStringUTF8(pDC, name, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
		break;
	
	case 2:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Lookup : (mAdbk ? CAdbkACL::eAdbkACL_Lookup : CCalendarACL::eCalACL_ReadFreeBusy)) ? icon_on : icon_off;
		break;

	case 3:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Read : (mAdbk ? CAdbkACL::eAdbkACL_Read : CCalendarACL::eCalACL_Read)) ? icon_on : icon_off;
		break;

	case 4:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Seen : (mAdbk ? CAdbkACL::eAdbkACL_Write : CCalendarACL::eCalACL_Write)) ? icon_on : icon_off;
		break;

	case 5:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Write : (mAdbk ? CAdbkACL::eAdbkACL_Create : CCalendarACL::eCalACL_Schedule)) ? icon_on : icon_off;
		break;

	case 6:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Insert : (mAdbk ? CAdbkACL::eAdbkACL_Delete : CCalendarACL::eCalACL_Create)) ? icon_on : icon_off;
		break;

	case 7:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Post : (mAdbk ? CAdbkACL::eAdbkACL_Admin : CCalendarACL::eCalACL_Delete)) ? icon_on : icon_off;
		break;

	case 8:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Create : CCalendarACL::eCalACL_Admin) ? icon_on : icon_off;
		break;

	case 9:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Delete) ? icon_on : icon_off;
		break;

	case 10:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Admin) ? icon_on : icon_off;
		break;

	default:
		break;
	}
	
	if (inCell.col > 1)
		pDC->Image(*mIcons[icon], mIcons[icon]->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
}

// Adjust column widths
void CACLTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do inherited call
	CTableDragAndDrop::ApertureResized(dw, dh);

	// NB Can be called before columns exist
	if (mCols > 0)
	{
		// Name column has variable width
		JCoordinate colwidth = GetApertureWidth() - (mMbox ? 9 : (mMbox ? 6 : 7)) * 21;
		if (colwidth < 32)
			colwidth = 32;
		SetColWidth(colwidth, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(21, 2, mMbox ? 10 : (mAdbk ? 7 : 8));
	}
}
