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


// Source for CTitleTableView class


#include "CTitleTableView.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CTableRowGeometry.h"
#include "CTableView.h"
#include "CXStringResources.h"

#include "StPenState.h"

#include <JPainter.h>
#include <JXImage.h>
#include <JRect.h>
#include <JXColormap.h>
#include <JXDragPainter.h>
#include <JXImage.h>
#include <JXWindow.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>
#include "TPopupMenu.h"

#include <algorithm>
#include <cassert>

int CTitleTableView::sClickColumn = 0;

const JSize cCellFrameWidth            = kJXDefaultBorderWidth;
const JCoordinate cDragRegionHalfWidth = 2;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// Default constructor
CTitleTableView::CTitleTableView(
	      JXScrollbarSet* scrollbarSet,
	      JXContainer* enclosure,
	      const HSizingOption hSizing, 
	      const VSizingOption vSizing,
	      const JCoordinate x, const JCoordinate y,
	      const JCoordinate w, const JCoordinate h)
  : CTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTableGeometry = new CTableRowGeometry(this, 72, 16);

	mCanSort = false;
	mCanSize = true;
	mValue = 0;

	SetRowSelect(true);

	mTableView = NULL;

	mPopup = new HPopupMenu("", enclosure, JXWidget::kFixedLeft,
													JXWidget::kFixedTop, 0, 0, 10, 10);
	mPopup->SetToHiddenPopupMenu(kTrue);
	mPopup->SetUpdateAction(JXMenu::kDisableNone);
	mPopup->Hide();
	ListenTo(mPopup);

	mDragLineCursor    = JXGetDragVertLineCursor(GetDisplay());
	mDragAllLineCursor = JXGetDragAllVertLineCursor(GetDisplay());
	mDragType = kInvalidDrag;

	WantInput(kFalse);
	SetBackColor((GetColormap())->GetDefaultBackColor());
}

// Default destructor
CTitleTableView::~CTitleTableView()
{
}

void CTitleTableView::OnCreate()
{
	// Get window that owns this one
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CTableView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CTableView*>(dynamic_cast<const CTableView*>(parent));
	if (mTableView == NULL)
		mCanSize = false;

	CTable::OnCreate();

	SetBorderWidth(0);
	InsertRows(1, 1, NULL, 0, false);
}

// Get column info
CColumnInfoArray& CTitleTableView::GetColumns()
{
	return mTableView->mColumnInfo;
}

void CTitleTableView::SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, unsigned long max_str, const cdstring& menu_items)
{
	mCanSort = can_sort;
	mCanSize = can_size;
	mMenuBase = menu_items;

	// Get titles from resource
	mTitles.clear();
	for(unsigned long i = 0; i < max_str; i++)
	{
		mTitles.push_back(rsrc::GetIndexedString(rsrcid, i));
	}
}

// Allow column size change
void CTitleTableView::SetValue(long value)
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
void CTitleTableView::SyncTable(const CTable* table, bool reposition)
{
	// Reset colum,n count
	TableIndexT rows, cols;
	table->GetTableSize(rows, cols);
	if (mCols > cols)
		RemoveCols(mCols - cols, 1, false);
	else if (mCols < cols)
		InsertCols(cols - mCols, 1);
	
	// Reset column widths
	for(TableIndexT col = 1; col <= cols; col++)
	{
		SetColWidth(table->GetColWidth(col), col, col);
	}
	
	// Reposition table to be just below the titles once row height adjust is done
	if (reposition)
	{
		JRect title_rect = GetFrameGlobal();
		
		int delta = GetRowHeight(1) - title_rect.height();
		if (delta != 0)
		{
			AdjustSize(0, delta);
			const_cast<CTable*>(table)->AdjustSize(0, -delta);
			const_cast<CTable*>(table)->Move(0, delta);
		}
	}
}

void CTitleTableView::HandleMouseDown(const JPoint& pt, const JXMouseButton button,
																	const JSize clickCount, 
																	const JXButtonStates& buttonStates,
																	const JXKeyModifiers& modifiers)
{
	STableCell cell;
	if (!InDragRegion(pt, cell) && mCanSort && (clickCount == 1) && button == kJXLeftButton)
	{
		int sort = mTableView->GetSortBy();
		bool sort_col = (sort == GetColumns()[cell.col - 1].column_type);

		if (!sort_col)
			// Change sort by
			mTableView->SetSortBy(GetColumns()[cell.col - 1].column_type);
		else
			// Force change to sort direction
			mTableView->ToggleShowBy();

		return;
	}
	else if (button == kJXRightButton && clickCount == 1)
	{
		OnRButtonDown(pt, buttonStates, modifiers);
		return;
	}

	// Remainder of this code is copied from JXColHeaderWidget

	mDragType = kInvalidDrag;
	if (!mCanSize)
		{
		return;
		}

	const JBoolean inDragRegion = InDragRegion(pt, mDragCell);

	if (inDragRegion && button == kJXLeftButton)
	{
		GetImageCellBounds(mDragCell, mDragCellRect);
		if (modifiers.meta())
		{
			mDragType = kDragAllCols;
		}
		else
		{
			mDragType = kDragOneCol;
		}

		JPainter* p = CreateDragOutsidePainter();
		JRect defClipRect = p->GetDefaultClipRect();
		JRect apG = GetApertureGlobal();
		defClipRect.left  = apG.left;
		defClipRect.right = apG.right;
		p->SetDefaultClipRect(defClipRect);

		const JRect enclAp = JXContainer::GlobalToLocal((GetEnclosure())->GetApertureGlobal());
		p->Line(pt.x, enclAp.top, pt.x, enclAp.bottom);
		mPrevPt = pt;
	}
}

void
CTitleTableView::HandleMouseDrag
	(
	const JPoint&			origPt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	if (mDragType != kInvalidDrag)
		{
		JPoint pt = origPt;

		// keep col width larger than minimum

		if (pt.x < mDragCellRect.left + 16)
			{
			pt.x = mDragCellRect.left + 16;
			}

		// check if we have moved

		if (pt.x != mPrevPt.x)
			{
                        JPainter* p = NULL;
                        JBoolean ok = GetDragPainter(&p);
                        assert( ok );

			const JRect enclApG = (GetEnclosure())->GetApertureGlobal();
			JRect enclAp = JXContainer::GlobalToLocal(enclApG);

			// scroll, if necessary

			const JPoint ptG    = JXContainer::LocalToGlobal(pt);
			const JPoint ptT    = JPinInRect(GetEnclosure()->GlobalToLocal(ptG),
											 GetEnclosure()->GetBounds());
			const JRect tableAp = GetEnclosure()->GetAperture();
			const JCoordinate y = tableAp.ycenter();
			const JRect tableRect(y-1, ptT.x-1, y+1, ptT.x+1);
#ifdef _TODO
			if (itsTable->ScrollToRect(tableRect))
				{
				(GetWindow())->Update();
				enclAp = JXContainer::GlobalToLocal(enclApG);	// local coords changed
				}
			else
#endif
				{
				// erase the old line

				p->Line(mPrevPt.x, enclAp.top, mPrevPt.x, enclAp.bottom);
				}

			// draw the new line

			p->Line(pt.x, enclAp.top, pt.x, enclAp.bottom);

			// ready for next call

			mPrevPt = pt;
			}
		}
}

void
CTitleTableView::HandleMouseUp
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	if (mDragType != kInvalidDrag)
	{
		// erase the line

                JPainter* p = NULL;
                JBoolean ok = GetDragPainter(&p);
                assert( ok );

		const JRect enclAp = JXContainer::GlobalToLocal((GetEnclosure())->GetApertureGlobal());
		p->Line(mPrevPt.x, enclAp.top, mPrevPt.x, enclAp.bottom);

		DeleteDragPainter();

		// set the column width(s)
		const JCoordinate colWidth = mPrevPt.x - mDragCellRect.left;
		mTableView->SetColumnWidth(mDragCell.col, colWidth);
	}

	mDragType = kInvalidDrag;
}

void
CTitleTableView::AdjustCursor
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	STableCell cell;
	const JBoolean inDragRegion = InDragRegion(pt, cell);
	if (mCanSize && inDragRegion && modifiers.meta())
		{
		DisplayCursor(mDragAllLineCursor);
		}
	else if (mCanSize && inDragRegion)
		{
		DisplayCursor(mDragLineCursor);
		}
	else
		{
		CTable::AdjustCursor(pt, modifiers);
		}
}

JBoolean
CTitleTableView::InDragRegion
	(
	const JPoint&	pt,
	STableCell&		cell
	)
	const
{
	if (GetCellHitBy(pt, cell))
	{
		JRect cellRect;
		GetImageCellBounds(cell, cellRect);
		
		// See if mouse is close to the left or right edges
		if ((pt.x < cellRect.left + cDragRegionHalfWidth) && (cell.col > 1) ||
			(pt.x > cellRect.right - cDragRegionHalfWidth))
		{
			// If on left side, adjust to previous cell
			if (pt.x < cellRect.left + cDragRegionHalfWidth)
				cell.col--;
			return kTrue;
		}
	}

	return kFalse;
}

void CTitleTableView::OnRButtonDown(const JPoint& point,
								const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers)
{
	// reset menu titles
	mPopup->SetMenuItems(mMenuBase.c_str());
	sClickColumn = 0;

	// Find column clicked
	STableCell cell;
	if (GetCellHitBy(point, cell))
		sClickColumn = cell.col;
	
	if (sClickColumn > 0) {
		// Save info on sort column
		SColumnInfo col_info = GetColumns()[sClickColumn - 1];
		
		// Track the popup
		mPopup->SetValue(col_info.column_type);
		mPopup->PopUp(this, point, buttonStates, modifiers);
	}
	
	return;
}

void CTitleTableView::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mPopup && message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		SColumnInfo col_info = GetColumns()[sClickColumn - 1];
		bool sort_col = mCanSort && (mTableView->GetSortBy() == col_info.column_type);
		MenuChoice(sClickColumn, sort_col, is->GetIndex());
	}
	else
	    CTable::Receive(sender, message);
}

void CTitleTableView::AdjustScrollbars()
{
	// Ignore - do not do scrollbar state changes - the table (not titles) controls the scroll range
}

void CTitleTableView::DrawBorder(JXWindowPainter& p, const JRect& frame)
{
	// Do default draw for D&D only
	if (IsDNDTarget())
		CTable::DrawBorder(p, frame);
	else
	{
		StPenState savestate(&p);

		// Draw single line black border all the way across the bottom
		p.SetLineWidth(1);
		p.SetPenColor(p.GetColormap()->GetBlackColor());
		const JPoint pt1 = frame.bottomLeft();
		const JPoint pt2 = frame.bottomRight();
		p.JPainter::Line(pt1, pt2);
	}
}

void CTitleTableView::DrawBackground(JXWindowPainter& p, const JRect& frame)
{
	// Reduce size of background to not draw over bottom line
	JRect temp = frame;
	temp.bottom--;
	CTable::DrawBackground(p, temp);
}

void CTitleTableView::DrawCell(JPainter* p, const STableCell& inCell, const JRect &rect)
{
	StPenState savestate(p);

	// Always reduce rect height by one to account for 'fake' black border at bottom
	JRect actual_rect = rect;
	actual_rect.bottom--;

	SColumnInfo col_info;
	if (mTableView)
		col_info = GetColumns()[inCell.col - 1];
	else
		col_info.column_type = inCell.col;

	// Determine if this is the current sort by method
	int sort = (mTableView != NULL) ? mTableView->GetSortBy() : 0;
	bool sort_col = mCanSort && (sort == col_info.column_type);

	if (sort_col)
	{
		CDrawUtils::DrawBackground(*p, actual_rect, true, true);
	}
	else
	{
		CDrawUtils::DrawSimpleBorder(*p, rect, false, true);
	}

	// Change text color if sort
	if (sort_col)
	{
		JFontStyle text_style = p->GetFontStyle();
		text_style.color = p->GetColormap()->GetWhiteColor();
		p->SetFontStyle(text_style);
	}

	DrawItem(p, col_info, actual_rect);
}

void CTitleTableView::DrawItem(JPainter* p, SColumnInfo& col_info, const JRect &cellRect)
{
	// Default is always text
	DrawText(p, col_info, cellRect);
}

// Draw text
void CTitleTableView::DrawText(JPainter* p, SColumnInfo& col_info, const JRect &cellRect)
{
	if ((col_info.column_type >= 1) && (col_info.column_type <= mTitles.size()))
	{
		::DrawClippedStringUTF8(p, mTitles[col_info.column_type - 1], JPoint(cellRect.left + 6, cellRect.top + 1), cellRect,
							RightJustify(col_info.column_type) ? eDrawString_Right : eDrawString_Left);
	}
}

// Draw icon
void CTitleTableView::DrawIcon(JPainter* p, SColumnInfo& col_info, unsigned int icon_id, const JRect &cellRect)
{
	JXImage* icon = CIconLoader::GetIcon(icon_id, this, 16, 0x00CCCCCC);
	int hoffset = std::min(cellRect.width() - 16, 2L);
	int voffset = (cellRect.height() - icon->GetBounds().height())/2;
	int left = 0;
	if (RightJustify(col_info.column_type))
		left = cellRect.right - 18 - hoffset;
	else
		left = cellRect.left + hoffset;
	p->Image(*icon, icon->GetBounds(), left, cellRect.top + voffset);
}

