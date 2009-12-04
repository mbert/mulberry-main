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

#include "CTableView.h"

#include "TPopupMenu.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// Default constructor
CServerTitleTable::CServerTitleTable(JXScrollbarSet* scrollbarSet,
				     JXContainer* enclosure,
				     const HSizingOption hSizing, 
				     const VSizingOption vSizing,
				     const JCoordinate x, const JCoordinate y,
				     const JCoordinate w, const JCoordinate h)
  : CTitleTableView(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CServerTitleTable::~CServerTitleTable()
{
}

// Get details of sub-panes
void CServerTitleTable::OnCreate(void)
{
	// Do inherited
	CTitleTableView::OnCreate();

	cdstring menu = "State %r| Total %r| Recent %r| Unseen %r| Favourite %r| Size %r%l | Insert Column after | Insert Column before | Delete Column";
	SetTitleInfo(false, true, "UI::Titles::Server", eServerColumnMax, menu);
}

// Clicked somewhere
void CServerTitleTable::OnRButtonDown(const JPoint& point,
										 const JXButtonStates& buttonStates,
										 const JXKeyModifiers& modifiers)
{
	mPopup->SetMenuItems(mMenuBase.c_str());
	sClickColumn = 0;

	// Find column clicked
	JPoint imagePt;
	LocalToImagePoint(point, imagePt);
	STableCell cell;
	if (GetCellHitBy(imagePt, cell))
		sClickColumn = cell.col;
	
	if (sClickColumn > 0) {
		// Save info on sort column
		SColumnInfo col_info = GetColumns()[sClickColumn - 1];
		
		
		// Handle name column as special case
		if( col_info.column_type == eServerColumnName) {
			mPopup->SetValue(0);
		} else {
			int ctype = col_info.column_type;
			//move down one because eServerColumnName doesn't have a menu entry
			if (ctype > eServerColumnFlags) {
				ctype--;
			}
			mPopup->SetValue(ctype);
		}

		// Disable items if name column
		if(col_info.column_type == eServerColumnName)
		{
			mPopup->DisableItem(kPopupState);
			mPopup->DisableItem(kPopupTotal);
			mPopup->DisableItem(kPopupRecent);
			mPopup->DisableItem(kPopupUnseen);
			mPopup->DisableItem(kPopupFavourite);
			mPopup->DisableItem(kPopupSize);
			mPopup->DisableItem(kPopupDelete);
		}
		else
	    	mPopup->EnableAll();

		mPopup->PopUp(this, point, buttonStates, modifiers);
	}
	
	return;
}

// Draw the titles
void CServerTitleTable::DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &rect)
{
	unsigned int icon_id = 0;
	switch(col_info.column_type)
	{
	case eServerColumnFlags:
		icon_id = IDI_SERVER_TITLE_FLAGS;
		break;

	case eServerColumnTotal:
		icon_id = IDI_SERVER_TITLE_TOTAL;
		break;

	case eServerColumnNew:
		icon_id = IDI_SERVER_TITLE_NEW;
		break;

	case eServerColumnUnseen:
		icon_id = IDI_SERVER_TITLE_UNSEEN;
		break;

	case eServerColumnAutoCheck:
		icon_id = IDI_SERVER_TITLE_AUTOCHECK;
		break;

	case eServerColumnSize:
	default:
		DrawText(p, col_info, rect);
		return;
	}

	DrawIcon(p, col_info, icon_id, rect);
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

// Handle choice
void CServerTitleTable::MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
	case kPopupState:
		mTableView->SetColumnType(col, eServerColumnFlags);
		break;

	case kPopupTotal:
		mTableView->SetColumnType(col, eServerColumnTotal);
		break;

	case kPopupRecent:
		mTableView->SetColumnType(col, eServerColumnNew);
		break;

	case kPopupUnseen:
		mTableView->SetColumnType(col, eServerColumnUnseen);
		break;

	case kPopupFavourite:
		mTableView->SetColumnType(col, eServerColumnAutoCheck);
		break;

	case kPopupSize:
		mTableView->SetColumnType(col, eServerColumnSize);
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

// Get text for current tooltip cell
void CServerTitleTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Determine which heading it is and draw it
	SColumnInfo col_info = GetColumns()[cell.col - 1];

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
