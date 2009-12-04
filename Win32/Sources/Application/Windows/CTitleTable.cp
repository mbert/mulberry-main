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


// Source for CTitleTable class


#include "CTitleTable.h"

#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CTableWindow.h"
#include "CTableRowGeometry.h"
#include "CXStringResources.h"

BEGIN_MESSAGE_MAP(CTitleTable, CTable)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


HCURSOR CTitleTable::sVertCursor = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// Default constructor
CTitleTable::CTitleTable()
{
	mTableGeometry = new CTableRowGeometry(this, 72, 16);
	
	mHasScrollbars = false;

	mCanSort = false;
	mCanSize = true;
	mValue = 0;

	SetRowSelect(true);
	
	mTrackingCol = 0;
	
	mSortTracking = false;
	mSortTrackingInside = false;

	mSizeTracking = false;
	mMouseOverDivider = false;

	mMenuID = 0;
	mTableWindow = NULL;

	// Load cursors first time through
	if (!sVertCursor)
		sVertCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
}

// Default destructor
CTitleTable::~CTitleTable()
{
}

int CTitleTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Get window that owns this one
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CTableWindow*>(parent))
		parent = parent->GetParent();
	mTableWindow = dynamic_cast<CTableWindow*>(parent);
	if (mTableWindow == NULL)
		mCanSize = false;

	InsertRows(1, 1, NULL, 0, false);

	return 0;
}

BOOL CTitleTable::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	// Do inherited
	BOOL result = CTable::SubclassDlgItem(nID, pParent);

	mCanSize = false;
	InsertRows(1, 1, NULL, 0, false);

	return result;
}

// Get column info
CColumnInfoArray& CTitleTable::GetColumns()
{
	return mTableWindow->mColumnInfo;
}

void CTitleTable::SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, UINT max_str, UINT menu_id)
{
	mCanSort = can_sort;
	mCanSize = can_size;
	mMenuID = menu_id;

	// Get titles from resource
	mTitles.clear();
	for(unsigned long i = 0; i < max_str; i++)
	{
		mTitles.push_back(rsrc::GetIndexedString(rsrcid, i));
	}
}

// Allow column size change
void CTitleTable::SetValue(long value)
{
	if (mCanSort)
	{
		long old_value = mValue;
		mValue = value;

		// Redraw cells
		RefreshCol(old_value);
		RefreshCol(value);
	}
}


// Sync columns with the table
void CTitleTable::SyncTable(const CTable* table, bool reposition)
{
	// Reset colum,n count
	TableIndexT rows, cols;
	table->GetTableSize(rows, cols);
	if (mCols > cols)
		RemoveCols(mCols - cols, 1, false);
	else if (mCols < cols)
		InsertCols(cols - mCols, 1, false);
	
	// Reset column widths
	for(TableIndexT col = 1; col <= cols; col++)
	{
		SetColWidth(table->GetColWidth(col), col, col);
	}
	
	// Reposition table to be just below the titles once row height adjust is done
	if (reposition)
	{
		
		CRect title_rect;
		GetWindowRect(title_rect);
		
		int delta = GetRowHeight(1) - title_rect.Height();
		if (delta != 0)
		{
			::ResizeWindowBy(this, 0, delta, false);
			::ResizeWindowBy(const_cast<CTable*>(table), 0, -delta, false);
			::MoveWindowBy(const_cast<CTable*>(table), 0, delta, true);
		}
	}
}

// Clicked somewhere
void CTitleTable::OnRButtonDown(UINT nFlags, CPoint point)
{
	STableCell	hitCell;
	CPoint		imagePt;
	LocalToImagePoint(point, imagePt);
	if (!GetCellHitBy(imagePt, hitCell) || (mMenuID == 0))
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

		int sort = mTableWindow->GetSortBy();
		bool sort_col = (sort == col_info.column_type) && mCanSort;

		// Track the popup
		CMenu* pPopup = popup.GetSubMenu(0);
		
		// Disable items if no change column type
		for(int i = 0; i < pPopup->GetMenuItemCount(); i++)
		{
			pPopup->EnableMenuItem(i, MF_ENABLED | MF_BYPOSITION);
		}

		pPopup->CheckMenuItem(col_info.column_type - 1 + pPopup->GetMenuItemID(0), MF_CHECKED);

		UINT result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, global.x, global.y, this);
		if (result)
		{
			switch(result)
			{
			case IDM_CHANGE_INSERT_AFTER:
				mTableWindow->InsertColumn(hitCell.col + 1);
				break;
			case IDM_CHANGE_INSERT_BEFORE:
				mTableWindow->InsertColumn(hitCell.col);
				break;
			case IDM_CHANGE_DELETE:
				mTableWindow->DeleteColumn(hitCell.col);
				break;
			default:
				// Adjust for missing Name item in menu
				mTableWindow->SetColumnType(hitCell.col, result - pPopup->GetMenuItemID(0) + 1);
				if (sort_col)
					mTableWindow->SetSortBy(result - pPopup->GetMenuItemID(0) + 1);
				break;
			}
		}
		RedrawWindow();
	}
}

// Clicked somewhere
void CTitleTable::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (mSortTracking || mSizeTracking)
		return;

	// Check for possible track start
	if ((mTableWindow != NULL) && (mCanSort || mCanSize))
	{
		// Determine type of track

		// Find cell hit by point
		CPoint imagePt;
		LocalToImagePoint(point, imagePt);
		STableCell cell;
		if (GetCellHitBy(imagePt, cell))
		{
			CRect cellRect;
			GetLocalCellRect(cell, cellRect);
			
			// See if mouse is close to the left or right edges
			if (mCanSize && ((point.x < cellRect.left + 2) && (cell.col > 1) ||
				(point.x > cellRect.right - 2)))
			{
				mSizeTracking = true;

				// Determine which column to track
				if (point.x < cellRect.left + 2)
					mTrackingCol = cell.col - 1;
				else
					mTrackingCol = cell.col;

				// Determine tracking rect
				CRect table;
				mTableWindow->mWndTable->GetWindowRect(table);
				
				if (point.x < cellRect.left + 2)
					mSizeTrackRect.SetRect(cellRect.left, 0, cellRect.left + 1, cellRect.Height() + table.Height());
				else
					mSizeTrackRect.SetRect(cellRect.right, 0, cellRect.right - 1, cellRect.Height() + table.Height());
				
				// Get track clip rect
				GetClientRect(mSizeTrackLimits);
				GetLocalCellRect(STableCell(1, mTrackingCol), cellRect);
				mSizeTrackLimits.left = cellRect.left + 16;
				
				mSizeTrackPt = mSizeTrackMovePt = point;

				OnInvertTracker(mSizeTrackRect);

				// Capture mouse so mouse-up can be tracked				
				SetCapture();
				return;
			}
			else
			{
				if (mCanSort)
				{
					// Do sort tracking
					mSortTracking = true;
					mSortTrackingInside = true;
					mTrackingCol = cell.col;

					RefreshCell(cell);

					// Capture mouse so mouse-up can be tracked				
					SetCapture();
					return;
				}
			}
		}
	}

	// Do inherited
	CTable::OnLButtonDown(nFlags, point);
}

// Clicked somewhere
void CTitleTable::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Handle sort column tracking
	if (mSortTracking)
	{
		ReleaseCapture();

		// Is mouse inside the cell
		STableCell cell(1, mTrackingCol);
		CRect cellRect;
		GetLocalCellRect(cell, cellRect);
		bool inside = cellRect.PtInRect(point);
		
		if (inside)
		{
			SColumnInfo col_info = GetColumns()[cell.col - 1];

			// Determine if this is the current sort by method
			int sort = mTableWindow->GetSortBy();
			bool sort_col = (sort == col_info.column_type);

			if (!sort_col)
			{
				// Change sort by
				mTableWindow->SetSortBy(col_info.column_type);
				RefreshRow(1);
			}
			else
			{
				// Must redraw cell to keep it correct
				RefreshCell(cell);

				// Force change to sort direction
				// Do this after drawing cell as port gets reset
				mTableWindow->ToggleShowBy();
			}
		}
		
		mSortTracking = false;
		mSortTrackingInside = false;
		mTrackingCol = 0;
	}

	// Handle size column tracking
	if (mSizeTracking)
	{
		mSizeTracking = false;

		ReleaseCapture();

		// erase tracker rectangle
		OnInvertTracker(mSizeTrackRect);

		// Check for actual move
		long moveby = mSizeTrackMovePt.x - mSizeTrackPt.x;
		if (moveby)
		{
			// Resize columns
			SInt32 new_width = GetColWidth(mTrackingCol) + moveby;
			if (new_width < 16)
				new_width = 16;
			mTableWindow->SetColumnWidth(mTrackingCol, new_width);
		}
		
		mTrackingCol = 0;
	}

	// Do inherited
	CTable::OnLButtonUp(nFlags, point);
}

void CTitleTable::OnMouseMove(UINT nFlags, CPoint point)
{
	// Always start in not over state
	mMouseOverDivider = false;

	if (mSortTracking)
	{
		// Is mouse inside the cell
		STableCell cell(1, mTrackingCol);
		CRect cellRect;
		GetLocalCellRect(cell, cellRect);
		bool inside = cellRect.PtInRect(point);
		
		// Toggle cell draw state
		if (inside ^ mSortTrackingInside)
		{
			mSortTrackingInside = inside;
			RefreshCell(cell);
		}
	}
	else if (mSizeTracking)
	{
		// Clip point to limit rect
		if (point.x < mSizeTrackLimits.left)
			point.x = mSizeTrackLimits.left;
		else if (point.x > mSizeTrackLimits.right)
			point.x = mSizeTrackLimits.right;
			
		// See if it has moved
		long moveby = point.x - mSizeTrackMovePt.x;
		if (moveby)
		{
			// Update current track point
			mSizeTrackMovePt = point;

			// Move the track line
			OnInvertTracker(mSizeTrackRect);
			mSizeTrackRect.OffsetRect(moveby, 0);
			OnInvertTracker(mSizeTrackRect);

			// Live resize of column
			SInt32 new_width = GetColWidth(mTrackingCol) + moveby;
			if (new_width < 16)
				new_width = 16;
			mTableWindow->SetColumnWidth(mTrackingCol, new_width);
			mSizeTrackPt = mSizeTrackMovePt;
		}
	}
	else if (mCanSize)
	{
		// Find cell hit by point
		CPoint imagePt;
		LocalToImagePoint(point, imagePt);
		STableCell cell;
		if (GetCellHitBy(imagePt, cell))
		{
			CRect cellRect;
			GetLocalCellRect(cell, cellRect);
			
			// See if mouse is close to the left or right edges
			if ((point.x < cellRect.left + 2) && (cell.col > 1) ||
				(point.x > cellRect.right - 2))
			{
				mMouseOverDivider = true;
			}
		}
	}
}

// Display column change cursor
BOOL CTitleTable::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ((nHitTest == HTCLIENT) && (pWnd == this) && mMouseOverDivider &&
		(::GetKeyState(VK_MENU) >= 0) && (::GetKeyState(VK_SHIFT) >= 0) && (::GetKeyState(VK_CONTROL) >= 0))
	{
		::SetCursor(sVertCursor);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CTitleTable::OnInvertTracker(const CRect& rect)
{
	// pat-blt without clip children on
	CDC* pDC = GetDC();
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);
}

void CTitleTable::DrawBackground(CDC* pDC, const CRect& inLocalRect)
{
	pDC->FillSolidRect(inLocalRect, CDrawUtils::sGrayColor);
}

// Draw a row
void CTitleTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	SColumnInfo col_info;
	if (mTableWindow)
		col_info = GetColumns()[inCell.col - 1];
	else
		col_info.column_type = inCell.col;

	// Determine if this is the current sort by method
	int sort = (mTableWindow != NULL) ? mTableWindow->GetSortBy() : 0;
	bool sort_col = mCanSort && (sort == col_info.column_type);

	// Handle tracking highlighting
	if (mSortTracking && (inCell.col == mTrackingCol))
		sort_col = mSortTrackingInside;

	// Draw frame
	pDC->FillRect(&inLocalRect, sort_col ? &CDrawUtils::sDkGrayBrush : &CDrawUtils::sGrayBrush);
	if (!sort_col)
	{
		pDC->SelectObject(CDrawUtils::sWhitePen);
		pDC->MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
		pDC->LineTo(inLocalRect.left, inLocalRect.top);
		pDC->LineTo(inLocalRect.right, inLocalRect.top);
		pDC->SelectObject(CDrawUtils::sDkGrayPen);
		pDC->MoveTo(inLocalRect.right - 2, inLocalRect.top + 1);
		pDC->LineTo(inLocalRect.right - 2, inLocalRect.bottom - 2);
		pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 2);
		pDC->SelectObject(CDrawUtils::sBlackPen);
		pDC->MoveTo(inLocalRect.right - 1, inLocalRect.top);
		pDC->LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
		pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 1);
		pDC->SetTextColor(CDrawUtils::sBtnTextColor);
	}
	else
		pDC->SetTextColor(CDrawUtils::sWhiteColor);
	pDC->SetBkColor(sort_col ? CDrawUtils::sDkGrayColor : CDrawUtils::sGrayColor);

	DrawItem(pDC, col_info, inLocalRect);
}

void CTitleTable::DrawItem(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	// Default is always text
	DrawText(pDC, col_info, cellRect);
}

// Draw text
void CTitleTable::DrawText(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	if ((col_info.column_type >= 1) && (col_info.column_type <= mTitles.size()))
	{
		pDC->SelectObject(CFontCache::GetListFont());
		::DrawClippedStringUTF8(pDC, mTitles[col_info.column_type - 1], CPoint(cellRect.left + 6, cellRect.top + 1), cellRect, RightJustify(col_info.column_type) ? eDrawString_Right : eDrawString_Left);
	}
}

// Draw icon
void CTitleTable::DrawIcon(CDC* pDC, SColumnInfo& col_info, UINT iconID, const CRect &cellRect)
{
	int offset = (cellRect.Width() < 24) ? 0 : 2;
	int left = 0;
	if (RightJustify(col_info.column_type))
		left = cellRect.right - 18 - offset;
	else
		left = cellRect.left + offset;

	CIconLoader::DrawIcon(pDC, left, cellRect.top + 1, iconID, 16);
}
