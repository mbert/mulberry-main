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

#include "CTableView.h"

#include "TPopupMenu.h"

// ---------------------------------------------------------------------------
//	CCalendarStoreTitleTable														  [public]
/**
	Default constructor */

CCalendarStoreTitleTable::CCalendarStoreTitleTable(JXScrollbarSet* scrollbarSet,
				     JXContainer* enclosure,
				     const HSizingOption hSizing, 
				     const VSizingOption vSizing,
				     const JCoordinate x, const JCoordinate y,
				     const JCoordinate w, const JCoordinate h)
  : CTitleTableView(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
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

void CCalendarStoreTitleTable::OnCreate()
{
	CTitleTableView::OnCreate();

	cdstring menu = "Subscribed %r | Colour %r %l | Insert Column after | Insert Column before | Delete Column";
	SetTitleInfo(false, true, "UI::Titles::CalendarStore", eCalendarStoreColumnMax, menu);
}

// Stop clicks
void CCalendarStoreTitleTable::OnRButtonDown(const JPoint& point,
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
		if( col_info.column_type == eCalendarStoreColumnName) {
			mPopup->SetValue(0);
		} else {
			int ctype = col_info.column_type;
			//move down one because eServerColumnName doesn't have a menu entry
			if (ctype > eCalendarStoreColumnName) {
				ctype--;
			}
			mPopup->SetValue(ctype);
		}

		// Disable items if name column
		if(col_info.column_type == eCalendarStoreColumnName)
		{
			mPopup->DisableItem(kPopupSubscribe);
			mPopup->DisableItem(kPopupColour);
			mPopup->DisableItem(kPopupDelete);
		}
		else
	    	mPopup->EnableAll();

		mPopup->PopUp(this, point, buttonStates, modifiers);
	}
	
	return;
}

// Handle choice
void CCalendarStoreTitleTable::MenuChoice(TableIndexT col, bool sort_col, JIndex menu_item)
{
	// Handle menu result
	switch (menu_item)
	{
	case kPopupSubscribe:
		mTableView->SetColumnType(col, eCalendarStoreColumnSubscribe);
		break;

	case kPopupColour:
		mTableView->SetColumnType(col, eCalendarStoreColumnColour);
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

// Draw the titles
void CCalendarStoreTitleTable::DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &cellRect)
{
	switch(col_info.column_type)
	{
	case eCalendarStoreColumnSubscribe:
		DrawIcon(p, col_info, IDI_CALENDARSTORE_TITLE_SUBSCRIBE, cellRect);
		break;

	default:
		DrawText(p, col_info, cellRect);
		break;
	}
}
