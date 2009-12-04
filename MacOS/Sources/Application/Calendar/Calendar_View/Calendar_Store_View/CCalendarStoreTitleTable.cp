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

#include "CCalendarStoreTitleTable.h"

#include "CContextMenu.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CTableView.h"

MenuHandle CCalendarStoreTitleTable::sColumnChanger = NULL;

// ---------------------------------------------------------------------------
//	CCalendarStoreTitleTable														  [public]
/**
	Default constructor */

CCalendarStoreTitleTable::CCalendarStoreTitleTable(LStream *inStream)
	: CTitleTableView(inStream)
{
}

// ---------------------------------------------------------------------------
//	~CCalendarStoreTitleTable														  [public]
/**
	Destructor */

CCalendarStoreTitleTable::~CCalendarStoreTitleTable()
{
}

#pragma mark -

void CCalendarStoreTitleTable::FinishCreateSelf()
{
	SetTitleInfo(false, true, "UI::Titles::CalendarStore", eCalendarStoreColumnMax, MENU_CalendarStoreColumnChanger);
	CTitleTableView::FinishCreateSelf();

	InsertRows(1, 0, NULL);
	AdaptToNewSurroundings();
}

// Init column changer menu
void CCalendarStoreTitleTable::InitColumnChanger()
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
void CCalendarStoreTitleTable::ClickCell(const STableCell& inCell,
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
		if (col_info.column_type == eCalendarStoreColumnName)
		{
			::DisableItem(sColumnChanger, cColumnCalendarStoreSubscribe);
			::DisableItem(sColumnChanger, cColumnCalendarStoreColour);
			::DisableItem(sColumnChanger, cColumnCalendarStoreDelete);
		}
		else
		{
			::EnableItem(sColumnChanger, cColumnCalendarStoreSubscribe);
			::EnableItem(sColumnChanger, cColumnCalendarStoreColour);
			::EnableItem(sColumnChanger, cColumnCalendarStoreDelete);
		}

		// Adjust for no name
		if (col_info.column_type >= eCalendarStoreColumnName)
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
void CCalendarStoreTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
		case cColumnCalendarStoreSubscribe:
			mTableView->SetColumnType(col, eCalendarStoreColumnSubscribe);
			break;

		case cColumnCalendarStoreColour:
			mTableView->SetColumnType(col, eCalendarStoreColumnColour);
			break;

		case cColumnCalendarStoreInsertAfter:
			mTableView->InsertColumn(col + 1);
			break;

		case cColumnCalendarStoreInsertBefore:
			mTableView->InsertColumn(col);
			break;

		case cColumnCalendarStoreDelete:
			mTableView->DeleteColumn(col);
			break;
	}
}

// Draw the titles
void CCalendarStoreTitleTable::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	switch(col_info.column_type)
	{
	case eCalendarStoreColumnSubscribe:
		DrawIcon(col_info, icnx_SearchHit, sort_col, inLocalRect);
		break;

	default:
		DrawText(col_info, sort_col, inLocalRect);
		break;
	}
}
