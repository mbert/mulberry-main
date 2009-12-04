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

BEGIN_MESSAGE_MAP(CAdbkManagerTitleTable, CTitleTableView)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTitleTable::CAdbkManagerTitleTable()
{
	// Load strings if not already
	SetTitleInfo(false, true, "UI::Titles::AdbkManager", eAdbkColumnMax, IDR_POPUP_ADBKMANAGER_HEADER);
}

// Default destructor
CAdbkManagerTitleTable::~CAdbkManagerTitleTable()
{
}

// Clicked somewhere
void CAdbkManagerTitleTable::OnRButtonDown(UINT nFlags, CPoint point)
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
		UINT enable = (col_info.column_type != eAdbkColumnName) ? MF_ENABLED : MF_GRAYED;
		pPopup->EnableMenuItem(IDM_CHANGE_OPENONSTART, enable);
		pPopup->EnableMenuItem(IDM_CHANGE_RESOLVE, enable);
		pPopup->EnableMenuItem(IDM_CHANGE_SEARCH, enable);
		pPopup->EnableMenuItem(IDM_CHANGE_DELETE, enable);
		switch(col_info.column_type)
		{
		case eAdbkColumnOpen:
			pPopup->CheckMenuItem(IDM_CHANGE_OPENONSTART, MF_CHECKED);
			break;
		case eAdbkColumnResolve:
			pPopup->CheckMenuItem(IDM_CHANGE_RESOLVE, MF_CHECKED);
			break;
		case eAdbkColumnSearch:
			pPopup->CheckMenuItem(IDM_CHANGE_SEARCH, MF_CHECKED);
			break;
		default:;
		}

		UINT result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, global.x, global.y, this);
		if (result)
		{
			switch(result)
			{
			case IDM_CHANGE_INSERT_AFTER:
				mTableView->InsertColumn(hitCell.col + 1);
				break;
			case IDM_CHANGE_INSERT_BEFORE:
				mTableView->InsertColumn(hitCell.col);
				break;
			case IDM_CHANGE_DELETE:
				mTableView->DeleteColumn(hitCell.col);
				break;
			default:
				// Adjust for missing Name item in menu
				mTableView->SetColumnType(hitCell.col, static_cast<EAdbkManagerColumn>(eAdbkColumnOpen + result - IDM_CHANGE_OPENONSTART));
				break;
			}
		}
		RedrawWindow();
	}
}

// Draw the titles
void CAdbkManagerTitleTable::DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
		DrawIcon(pDC, col_info, IDI_ADBK_TITLE_OPEN, cellRect);
		break;

	case eAdbkColumnResolve:
		DrawIcon(pDC, col_info, IDI_ADBK_TITLE_RESOLVE, cellRect);
		break;

	case eAdbkColumnSearch:
		DrawIcon(pDC, col_info, IDI_ADBK_TITLE_SEARCH, cellRect);
		break;

	default:
		DrawText(pDC, col_info, cellRect);
		break;
	}
}
