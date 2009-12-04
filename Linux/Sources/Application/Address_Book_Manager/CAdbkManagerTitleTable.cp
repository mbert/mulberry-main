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


// Source for CAdbkManagerTitleTable class

#include "CAdbkManagerTitleTable.h"

#include "CTableView.h"

#include "TPopupMenu.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTitleTable::CAdbkManagerTitleTable(JXScrollbarSet* scrollbarSet,
				     JXContainer* enclosure,
				     const HSizingOption hSizing, 
				     const VSizingOption vSizing,
				     const JCoordinate x, const JCoordinate y,
				     const JCoordinate w, const JCoordinate h)
  : CTitleTableView(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}


// Default destructor
CAdbkManagerTitleTable::~CAdbkManagerTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAdbkManagerTitleTable::OnCreate()
{
	CTitleTableView::OnCreate();

	// Load strings if not already
	cdstring menu = "Open on Start %r | Resolve Nick-Name %r | Allow Search %r %l | Insert Column after | Insert Column before | Delete Column";
	SetTitleInfo(false, true, "UI::Titles::AdbkManager", eAdbkColumnMax, menu);
}


// Handle choice
void CAdbkManagerTitleTable::MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
	case kPopupOpen:
		mTableView->SetColumnType(col, eAdbkColumnOpen);
		break;

	case kPopupResolve:
		mTableView->SetColumnType(col, eAdbkColumnResolve);
		break;
		
	case kPopupSearch:
		mTableView->SetColumnType(col, eAdbkColumnSearch);
		break;
		
	case kPopupInsertAfter:
		mTableView->InsertColumn(col + 1);
		break;

	case kPopupInsertBefore:
		mTableView->InsertColumn(col);
		break;

	case kPopupDelete:
		mTableView->DeleteColumn(col);
		break;
	}
}

void CAdbkManagerTitleTable::OnRButtonDown(const JPoint& point,
											 const JXButtonStates& buttonStates,
											 const JXKeyModifiers& modifiers)
{
	STableCell	hitCell;
	JPoint		imagePt;
	LocalToImagePoint(point, imagePt);
	if (!GetCellHitBy(imagePt, hitCell))
	{
		CTitleTableView::OnRButtonDown(point, buttonStates, modifiers);
		return;
	}
	sClickColumn = hitCell.col;

	mPopup->SetMenuItems(mMenuBase.c_str());

	if (sClickColumn > 0)
	{
		// Save info on sort column
		SColumnInfo col_info = GetColumns()[hitCell.col - 1];
		
		// Handle name column as special case
		if( col_info.column_type == eAdbkColumnName)
		{
			mPopup->SetValue(0);
			// Disable items if name column
			mPopup->DisableItem(kPopupOpen);
			mPopup->DisableItem(kPopupResolve);
			mPopup->DisableItem(kPopupSearch);
		}
		else
		{
			mPopup->SetValue(col_info.column_type - 1);
			mPopup->EnableAll();
		}

		mPopup->PopUp(this, point, buttonStates, modifiers);
	}
	
	return;
}

// Draw the titles
void CAdbkManagerTitleTable::DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &rect)
{
	unsigned int icon_id = 0;
	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
		icon_id = IDI_ADBK_TITLE_OPEN;
		break;

	case eAdbkColumnResolve:
		icon_id = IDI_ADBK_TITLE_RESOLVE;
		break;

	case eAdbkColumnSearch:
		icon_id = IDI_ADBK_TITLE_SEARCH;
		break;

	default:
		DrawText(p, col_info, rect);
		return;
	}

	DrawIcon(p, col_info, icon_id, rect);
}
