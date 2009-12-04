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


// Source for CServerTitleTable class

#include "CServerTitleTable.h"

#include "CContextMenu.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CTableView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

MenuHandle CServerTitleTable::sColumnChanger = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerTitleTable::CServerTitleTable()
{
}

// Constructor from stream
CServerTitleTable::CServerTitleTable(LStream *inStream)
	: CTitleTableView(inStream)
{
}

// Default destructor
CServerTitleTable::~CServerTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CServerTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(false, true, "UI::Titles::Server", eServerColumnMax, MENU_ServerColumnChanger);
	CTitleTableView::FinishCreateSelf();

	// Make it fit to the superview
	InsertRows(1, 1, NULL, 0, false);
	AdaptToNewSurroundings();

	// Turn on tooltips
	EnableTooltips();
}

// Get menu
void CServerTitleTable::InitColumnChanger(void)
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
void CServerTitleTable::ClickCell(const STableCell& inCell,
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
		if (col_info.column_type == eServerColumnName)
		{
			::DisableItem(sColumnChanger, cColumnServerFlags);
			::DisableItem(sColumnChanger, cColumnServerTotal);
			::DisableItem(sColumnChanger, cColumnServerNew);
			::DisableItem(sColumnChanger, cColumnServerUnseen);
			::DisableItem(sColumnChanger, cColumnServerAutoCheck);
			::DisableItem(sColumnChanger, cColumnServerSize);
			::DisableItem(sColumnChanger, cColumnServerDelete);
		}
		else
		{
			::EnableItem(sColumnChanger, cColumnServerFlags);
			::EnableItem(sColumnChanger, cColumnServerTotal);
			::EnableItem(sColumnChanger, cColumnServerNew);
			::EnableItem(sColumnChanger, cColumnServerUnseen);
			::EnableItem(sColumnChanger, cColumnServerAutoCheck);
			::EnableItem(sColumnChanger, cColumnServerSize);
			::EnableItem(sColumnChanger, cColumnServerDelete);
		}

		// Adjust for no name
		if (col_info.column_type >= eServerColumnName)
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
void CServerTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
		case cColumnServerFlags:
			mTableView->SetColumnType(col, eServerColumnFlags);
			break;

		case cColumnServerTotal:
			mTableView->SetColumnType(col, eServerColumnTotal);
			break;

		case cColumnServerNew:
			mTableView->SetColumnType(col, eServerColumnNew);
			break;

		case cColumnServerUnseen:
			mTableView->SetColumnType(col, eServerColumnUnseen);
			break;

		case cColumnServerAutoCheck:
			mTableView->SetColumnType(col, eServerColumnAutoCheck);
			break;

		case cColumnServerSize:
			mTableView->SetColumnType(col, eServerColumnSize);
			break;

		case cColumnServerInsertAfter:
			mTableView->InsertColumn(col + 1);
			break;

		case cColumnServerInsertBefore:
			mTableView->InsertColumn(col);
			break;

		case cColumnServerDelete:
			mTableView->DeleteColumn(col);
			break;
	}
}

// Draw the titles
void CServerTitleTable::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	switch(col_info.column_type)
	{
	case eServerColumnFlags:
		DrawIcon(col_info, ICNx_Title_Flag, sort_col, inLocalRect);
		break;

	case eServerColumnTotal:
		DrawIcon(col_info, ICNx_TitleTotal, sort_col, inLocalRect);
		break;

	case eServerColumnNew:
		DrawIcon(col_info, ICNx_TitleNew, sort_col, inLocalRect);
		break;

	case eServerColumnUnseen:
		DrawIcon(col_info, ICNx_TitleUnseen, sort_col, inLocalRect);
		break;

	case eServerColumnAutoCheck:
		DrawIcon(col_info, ICNx_TitleAutoCheck, sort_col, inLocalRect);
		break;

	case eServerColumnSize:
	default:
		DrawText(col_info, sort_col, inLocalRect);
		break;
	}
}

// Check for right justification
bool CServerTitleTable::RightJustify(int col_type)
{
	switch(col_type)
	{
	case eServerColumnTotal:
	case eServerColumnNew:
	case eServerColumnUnseen:
		return true;
	default:
		return false;
	}
}

// Get text for current tooltip cell
void CServerTitleTable::GetTooltipText(cdstring& txt, const STableCell &inCell)
{
	// Determine which heading it is and draw it
	SColumnInfo col_info = GetColumns()[inCell.col - TABLE_START_INDEX];

	switch(col_info.column_type)
	{
	case eServerColumnFlags:
		txt = "Status of a mailbox";
		break;

	case eServerColumnName:
		txt = "The name of the item";
		break;

	case eServerColumnTotal:
		txt = "Total number of messages in a mailbox";
		break;

	case eServerColumnNew:
		txt = "Total number of recent messages in a mailbox";
		break;

	case eServerColumnUnseen:
		txt = "Total number of unseen messages in a mailbox";
		break;

	case eServerColumnAutoCheck:
		txt = "Indicates whether a mailbox is marked for new mail checking";
		break;

	case eServerColumnSize:
		txt = "The size of a mailbox";
		break;
	default:;
	}

}