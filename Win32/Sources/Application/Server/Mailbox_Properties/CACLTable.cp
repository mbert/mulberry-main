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

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPropAdbkACL.h"
#include "CPropCalendarACL.h"
#include "CPropMailboxACL.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

IMPLEMENT_DYNCREATE(CACLTable, CTableDragAndDrop)

BEGIN_MESSAGE_MAP(CACLTable, CTableDragAndDrop)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

cdstring CACLTable::sCurrentUser;

CACLTable::CACLTable()

{	
	// Load strings if not already
	if (sCurrentUser.empty())
	{
		sCurrentUser.FromResource("UI::MailboxProp::CurrentUser");
	}
	
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

BOOL CACLTable::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	// Do inherited
	BOOL result = CTableDragAndDrop::SubclassDlgItem(nID, pParent);

	// Drag and drop
	SetAllowDrag(false);

	return result;
}

void CACLTable::SetList(CCalendarACLList* aList, bool read_write)
{
	// Create columns first time through
	if (!mCols)
	{
		// Create columns
		InsertCols(7, 1, NULL, 0, false);
		
		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		// Name column has variable width
		SetColWidth(cx - 114, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(19, 2, 7);
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
		
		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		// Name column has variable width
		SetColWidth(cx - 114, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(19, 2, 7);
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
		InsertCols(10, 1, nil, 0, false);
		
		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		// Name column has variable width
		SetColWidth(cx - 171, 1, 1);
		
		// Remaining columns have fixed width
		SetColWidth(19, 2, 10);
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
bool CACLTable::ClickSelect(const STableCell &inCell, UINT	nFlags)
{
	// Special for button columns
	if ((inCell.col == 1) || ((nFlags & MK_CONTROL) == 0))
		return CTableDragAndDrop::ClickSelect(inCell, nFlags);
	else
		return true;
}

// Clicked item
void CACLTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	// Only if admin allowed and not 'Current User' row
	if (!mReadWrite || (inCell.row == 1))
		return;

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
		right = (mMbox ? CMboxACL::eMboxACL_Lookup : (mAdbk ? CAdbkACL::eAdbkACL_Lookup : CCalendarACL::eCalACL_Lookup));
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
		right = (mMbox ? CMboxACL::eMboxACL_Write : (mAdbk ? CAdbkACL::eAdbkACL_Create : CCalendarACL::eCalACL_Create));
		set = rights.HasRight(right) ? false : true;
		break;
	case 6:
		right = (mMbox ? CMboxACL::eMboxACL_Insert : (mAdbk ? CAdbkACL::eAdbkACL_Delete : CCalendarACL::eCalACL_Delete));
		set = rights.HasRight(right) ? false : true;
		break;
	case 7:
		right = (mMbox ? CMboxACL::eMboxACL_Post : (mAdbk ? CAdbkACL::eAdbkACL_Admin : CCalendarACL::eCalACL_Admin));
		set = rights.HasRight(right) ? false : true;
		break;
	case 8:
		right = CMboxACL::eMboxACL_Create;
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
	if (mMbox)
		((CPropMailboxACL*) GetParent())->DoChangeACL((CMboxACL::EMboxACL) right, set);
	else if (mAdbk)
		((CPropAdbkACL*) GetParent())->DoChangeACL((CAdbkACL::EAdbkACL) right, set);
	else if (mCalendar)
		((CPropCalendarACL*) GetParent())->DoChangeACL((CCalendarACL::ECalACL) right, set);
}

// Set indicators from rights
void CACLTable::SetMyRights(SACLRight rights)
{
	mMyRights = rights;
	RefreshRow(1);
}

void CACLTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	StDCState save(pDC);

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

		icon_on = (mReadWrite && can_change) ? IDI_DIAMONDTICKED : IDI_ACL_SET;
		icon_off = (mReadWrite && can_change) ? IDI_DIAMOND : IDI_ACL_UNSET;
	}
	else
	{
		name = sCurrentUser;
		rights = mMyRights;
		icon_on = IDI_ACL_SET;
		icon_off = IDI_ACL_UNSET;

		// Special background for current user
		pDC->FillRect(&inLocalRect, &CDrawUtils::sGrayBrush);
		pDC->SetBkColor(CDrawUtils::sGrayColor);
	}

	switch(inCell.col)
	{
	
	case 1:
		// Draw name of item
		{
			CRect temp(inLocalRect);
			temp.left += 2;
			::DrawClippedStringUTF8(pDC, name, CPoint(temp.left, temp.top + mTextOrigin), temp, eDrawString_Left);
			break;
		}
	
	case 2:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Lookup : (mAdbk ? CAdbkACL::eAdbkACL_Lookup : CCalendarACL::eCalACL_Lookup)) ? icon_on : icon_off;
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
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Write : (mAdbk ? CAdbkACL::eAdbkACL_Create : CCalendarACL::eCalACL_Create)) ? icon_on : icon_off;
		break;

	case 6:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Insert : (mAdbk ? CAdbkACL::eAdbkACL_Delete : CCalendarACL::eCalACL_Delete)) ? icon_on : icon_off;
		break;

	case 7:
		// Determine icon
		icon = rights.HasRight(mMbox ? CMboxACL::eMboxACL_Post : (mAdbk ? CAdbkACL::eAdbkACL_Admin : CCalendarACL::eCalACL_Admin)) ? icon_on : icon_off;
		break;

	case 8:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Create) ? icon_on : icon_off;
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
		CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, icon, 16);
}
