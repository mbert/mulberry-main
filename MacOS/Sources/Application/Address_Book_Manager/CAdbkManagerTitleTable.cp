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

#include "CContextMenu.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CTableView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

MenuHandle CAdbkManagerTitleTable::sColumnChanger = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTitleTable::CAdbkManagerTitleTable()
{
}

// Constructor from stream
CAdbkManagerTitleTable::CAdbkManagerTitleTable(LStream *inStream)
	: CTitleTableView(inStream)
{
}

// Default destructor
CAdbkManagerTitleTable::~CAdbkManagerTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAdbkManagerTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(false, true, "UI::Titles::AdbkManager", eAdbkColumnMax, MENU_AdbkManagerColumnChanger);
	CTitleTableView::FinishCreateSelf();

	// Make it fit to the superview
	InsertRows(1, 1, nil, 0, false);
	AdaptToNewSurroundings();
}

// Get menu
void CAdbkManagerTitleTable::InitColumnChanger(void)
{
	// Get menu handle if not already here
	if (!sColumnChanger)
	{
		sColumnChanger = ::GetMenu(mMenuID);
		ThrowIfResFail_(sColumnChanger);
		::InsertMenu(sColumnChanger, hierMenu);
	}

	mColumnChanger = sColumnChanger;
}

// Stop clicks
void CAdbkManagerTitleTable::ClickCell(const STableCell& inCell,
									const SMouseDownEvent &inMouseDown)
{
	// Get column info for cell
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	if (CContextMenuProcessAttachment::IsCMMEvent(inMouseDown.macEvent) || (inMouseDown.macEvent.modifiers & cmdKey))
	{
		// Do menu Select
		Point startPt = inMouseDown.wherePort;
		PortToGlobalPoint(startPt);

		// Disable column info items if name
		if (col_info.column_type == eAdbkColumnName)
		{
			::DisableItem(sColumnChanger, cColumnAdbkOpen);
			::DisableItem(sColumnChanger, cColumnAdbkResolve);
			::DisableItem(sColumnChanger, cColumnAdbkSearch);
			::DisableItem(sColumnChanger, cColumnAdbkDelete);
		}
		else
		{
			::EnableItem(sColumnChanger, cColumnAdbkOpen);
			::EnableItem(sColumnChanger, cColumnAdbkResolve);
			::EnableItem(sColumnChanger, cColumnAdbkSearch);
			::EnableItem(sColumnChanger, cColumnAdbkDelete);
		}

		// Adjust for no name
		if (col_info.column_type >= eAdbkColumnName)
			col_info.column_type = (col_info.column_type - 1);

		// Do popup after setting check mark
		::SetItemMark(sColumnChanger, col_info.column_type, 0x12);
		SInt32 result = ::PopUpMenuSelect(sColumnChanger, startPt.v, startPt.h, col_info.column_type);
		::SetItemMark(sColumnChanger, col_info.column_type, noMark);

		// Handle menu result
		bool sort_col = mCanSort && (mTableView->GetSortBy() == col_info.column_type);
		MenuChoice(inCell.col, sort_col, result & 0x0000FFFF);
	}
}

// Handle choice
void CAdbkManagerTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
		case cColumnAdbkOpen:
			mTableView->SetColumnType(col, eAdbkColumnOpen);
			break;

		case cColumnAdbkResolve:
			mTableView->SetColumnType(col, eAdbkColumnResolve);
			break;

		case cColumnAdbkSearch:
			mTableView->SetColumnType(col, eAdbkColumnSearch);
			break;

		case cColumnAdbkInsertAfter:
			mTableView->InsertColumn(col + 1);
			break;

		case cColumnAdbkInsertBefore:
			mTableView->InsertColumn(col);
			break;

		case cColumnAdbkDelete:
			mTableView->DeleteColumn(col);
			break;
	}
}

// Draw the titles
void CAdbkManagerTitleTable::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
		DrawIcon(col_info, ICNx_AdbkOpen, sort_col, inLocalRect);
		break;

	case eAdbkColumnResolve:
		DrawIcon(col_info, ICNx_AdbkResolve, sort_col, inLocalRect);
		break;

	case eAdbkColumnSearch:
		DrawIcon(col_info, ICNx_AdbkSearch, sort_col, inLocalRect);
		break;

	default:
		DrawText(col_info, sort_col, inLocalRect);
		break;
	}
}
