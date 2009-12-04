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

#include "CTable.h"
#include "CTableView.h"

BEGIN_MESSAGE_MAP(CCalendarStoreTitleTable, CTitleTableView)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
//	CCalendarStoreTitleTable														  [public]
/**
	Default constructor */

CCalendarStoreTitleTable::CCalendarStoreTitleTable()
{
	// Load strings if not already
	SetTitleInfo(false, true, "UI::Titles::CalendarStore", eCalendarStoreColumnMax, IDR_POPUP_CALENDARSTORE_HEADER);
	
	EnableToolTips();
}

// ---------------------------------------------------------------------------
//	~CCalendarStoreTitleTable														  [public]
/**
	Destructor */

CCalendarStoreTitleTable::~CCalendarStoreTitleTable()
{
}

#pragma mark -

// Clicked somewhere
void CCalendarStoreTitleTable::OnRButtonDown(UINT nFlags, CPoint point)
{
	STableCell	hitCell;
	CPoint		imagePt;
	LocalToImagePoint(point, imagePt);
	if (!GetCellHitBy(imagePt, hitCell))
	{
		CTable::OnRButtonDown(nFlags, point);
		return;
	}

	CMenu popup;
	CPoint global = point;
	ClientToScreen(&global);

	if (popup.LoadMenu(mMenuID))
	{

		// Get info
		SColumnInfo col_info = GetColumns()[hitCell.col - 1];

		// Track the popup
		CMenu* pPopup = popup.GetSubMenu(0);
		
		// Handle name column as special case
		UINT enable = (col_info.column_type != eCalendarStoreColumnName) ? MF_ENABLED : MF_GRAYED;
		pPopup->EnableMenuItem(IDM_CALENDARSTORE_SUBSCRIBE, enable);
		pPopup->EnableMenuItem(IDM_CALENDARSTORE_COLOUR, enable);
		pPopup->EnableMenuItem(IDM_CALENDARSTORE_DELETE, enable);
		switch(col_info.column_type)
		{
		case eCalendarStoreColumnSubscribe:
			pPopup->CheckMenuItem(IDM_CALENDARSTORE_SUBSCRIBE, MF_CHECKED);
			break;
		case eCalendarStoreColumnColour:
			pPopup->CheckMenuItem(IDM_CALENDARSTORE_COLOUR, MF_CHECKED);
			break;
		default:;
		}

		UINT result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, global.x, global.y, this);
		if (result)
		{
			switch(result)
			{
			case IDM_CALENDARSTORE_INSERT_AFTER:
				mTableView->InsertColumn(hitCell.col + 1);
				break;
			case IDM_CALENDARSTORE_INSERT_BEFORE:
				mTableView->InsertColumn(hitCell.col);
				break;
			case IDM_CALENDARSTORE_DELETE:
				mTableView->DeleteColumn(hitCell.col);
				break;
			default:
				// Adjust for missing Name item in menu
				mTableView->SetColumnType(hitCell.col, static_cast<ECalendarStoreColumn>(eCalendarStoreColumnSubscribe + result - IDM_CALENDARSTORE_SUBSCRIBE));
				break;
			}
		}
		RedrawWindow();
	}
}

// Get text for current tooltip cell
void CCalendarStoreTitleTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Determine which heading it is and draw it
	SColumnInfo col_info = GetColumns()[cell.col - 1];

	switch(col_info.column_type)
	{
	case eCalendarStoreColumnName:
		txt = "The name of the calendar";
		break;
	case eCalendarStoreColumnSubscribe:
		txt = "Indicates whether the calendar is subscribed";
		break;
	case eCalendarStoreColumnColour:
		txt = "The colour used to display items from the calendar";
		break;
	default:;
	}
}

// Draw the titles
void CCalendarStoreTitleTable::DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	switch(col_info.column_type)
	{
	case eCalendarStoreColumnSubscribe:
		DrawIcon(pDC, col_info, IDI_CALENDARSTORE_TITLE_SUBSCRIBE, cellRect);
		break;

	default:
		DrawText(pDC, col_info, cellRect);
		break;
	}
}
