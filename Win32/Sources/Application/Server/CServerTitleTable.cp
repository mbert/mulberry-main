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

#include "CTable.h"
#include "CTableView.h"

BEGIN_MESSAGE_MAP(CServerTitleTable, CTitleTableView)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerTitleTable::CServerTitleTable()
{
	// Load strings if not already
	SetTitleInfo(false, true, "UI::Titles::Server", eServerColumnMax, IDR_POPUP_SERVER_HEADER);

	EnableToolTips();
}

// Default destructor
CServerTitleTable::~CServerTitleTable()
{
}

// Clicked somewhere
void CServerTitleTable::OnRButtonDown(UINT nFlags, CPoint point)
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
		UINT enable = (col_info.column_type != eServerColumnName) ? MF_ENABLED : MF_GRAYED;
		pPopup->EnableMenuItem(IDM_SERVER_FLAGS, enable);
		pPopup->EnableMenuItem(IDM_SERVER_TOTAL, enable);
		pPopup->EnableMenuItem(IDM_SERVER_NEW, enable);
		pPopup->EnableMenuItem(IDM_SERVER_UNSEEN, enable);
		pPopup->EnableMenuItem(IDM_SERVER_AUTOCHECK, enable);
		pPopup->EnableMenuItem(IDM_SERVER_SIZE, enable);
		pPopup->EnableMenuItem(IDM_SERVER_DELETE, enable);
		switch(col_info.column_type)
		{
		case eServerColumnFlags:
			pPopup->CheckMenuItem(IDM_SERVER_FLAGS, MF_CHECKED);
			break;
		case eServerColumnTotal:
			pPopup->CheckMenuItem(IDM_SERVER_TOTAL, MF_CHECKED);
			break;
		case eServerColumnNew:
			pPopup->CheckMenuItem(IDM_SERVER_NEW, MF_CHECKED);
			break;
		case eServerColumnUnseen:
			pPopup->CheckMenuItem(IDM_SERVER_UNSEEN, MF_CHECKED);
			break;
		case eServerColumnAutoCheck:
			pPopup->CheckMenuItem(IDM_SERVER_AUTOCHECK, MF_CHECKED);
			break;
		case eServerColumnSize:
			pPopup->CheckMenuItem(IDM_SERVER_SIZE, MF_CHECKED);
			break;
		default:;
		}

		UINT result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, global.x, global.y, this);
		if (result)
		{
			switch(result)
			{
			case IDM_SERVER_INSERT_AFTER:
				mTableView->InsertColumn(hitCell.col + 1);
				break;
			case IDM_SERVER_INSERT_BEFORE:
				mTableView->InsertColumn(hitCell.col);
				break;
			case IDM_SERVER_DELETE:
				mTableView->DeleteColumn(hitCell.col);
				break;
			default:
				// Adjust for missing Name item in menu
				if (result > IDM_SERVER_FLAGS) result++;
				mTableView->SetColumnType(hitCell.col, static_cast<EServerColumn>(eServerColumnFlags + result - IDM_SERVER_FLAGS));
				break;
			}
		}
		RedrawWindow();
	}
}

// Draw the titles
void CServerTitleTable::DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	switch(col_info.column_type)
	{
	case eServerColumnFlags:
		DrawIcon(pDC, col_info, IDI_SERVER_TITLE_FLAGS, cellRect);
		break;

	case eServerColumnTotal:
		DrawIcon(pDC, col_info, IDI_SERVER_TITLE_TOTAL, cellRect);
		break;

	case eServerColumnNew:
		DrawIcon(pDC, col_info, IDI_SERVER_TITLE_NEW, cellRect);
		break;

	case eServerColumnUnseen:
		DrawIcon(pDC, col_info, IDI_SERVER_TITLE_UNSEEN, cellRect);
		break;

	case eServerColumnAutoCheck:
		DrawIcon(pDC, col_info, IDI_SERVER_TITLE_AUTOCHECK, cellRect);
		break;
	
	default:
		DrawText(pDC, col_info, cellRect);
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