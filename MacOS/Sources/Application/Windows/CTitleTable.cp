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

#include "CContextMenu.h"
#include "CHierarchyTableDrag.h"
#include "CMulberryCommon.h"
#include "CSplitDivider.h"
#include "CTableWindow.h"
#include "CTableRowGeometry.h"
#include "CXStringResources.h"

#include <UEventMgr.h>
#include <UGAColorRamp.h>
#include <UGraphicUtils.h>

#include <Memory.h>
#include <Resources.h>

// __________________________________________________________________________________________________
// C L A S S __ C T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTitleTable::CTitleTable()
{
	InitTitleTable();
}

// Constructor from stream
CTitleTable::CTitleTable(LStream *inStream)
	: CTableDrag(inStream)
{
	InitTitleTable();
}

// Default destructor
CTitleTable::~CTitleTable()
{
}

// Do common init
void CTitleTable::InitTitleTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, 72, 16);

	mCanSort = true;
	mCanSize = true;
	mValue = 0;

	SetRowSelect(true);
	
	mTracking = false;
	mTrackingInside = false;
	
	mTableWindow = NULL;
	mTable = NULL;
	mHTable = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Get table window in heirarchy
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CTableWindow*>(super))
		super = super->GetSuperView();
	mTableWindow = dynamic_cast<CTableWindow*>(super);
	if (mTableWindow == NULL)
		mCanSize = false;

	InitColumnChanger();

	// Always read-only for dd
	SetTable(this, false);
	SetDDReadOnly(true);

	CContextMenuAttachment::AddUniqueContext(this, new CNULLContextMenuAttachment);
}

//	Respond to commands
Boolean CTitleTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean	cmdHandled = true;

	switch (inCommand)
	{
	case msg_TabSelect:
		// Can never be the target
		cmdHandled = false;
		break;

	default:
		cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

// Get column info
CColumnInfoArray& CTitleTable::GetColumns()
{
	return mTableWindow->mColumnInfo;
}

void CTitleTable::SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, unsigned long max_str, ResIDT menu_id)
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

void CTitleTable::ActivateSelf ()
{

	// ¥ Get everything redrawn so the changes take effect
	if ( mActive == triState_On )
		Refresh ();

	CTableDrag::ActivateSelf();
}


void CTitleTable::DeactivateSelf ()
{

	// ¥ Get everything redrawn so the changes take effect
	if ( mActive == triState_Off || mActive == triState_Latent )
		Refresh ();

	CTableDrag::DeactivateSelf();
}

// Allow column size change
void CTitleTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// A click on this must active its table
	if (mTable)
		SwitchTarget(mTable);
	else if (mHTable)
		SwitchTarget(mHTable);

	SInt16	col_pos = 0;

	// Is it near column edge
	if (mCanSize)
	{
		for(SInt16 i = 1; i <= mCols; i++)
		{
			col_pos += GetColWidth(i);
			if ((inMouseDown.whereLocal.h > col_pos - 2) &&
				(inMouseDown.whereLocal.h < col_pos + 2))
			{
				// Find current size of info table
				LTableView* info_table = mTableWindow->mTable;
				SDimension16 info_size;
				info_table->GetFrameSize(info_size);

				// Create gray line region
				RgnHandle	gray_line = ::NewRgn();
				Rect line = {0, col_pos, mFrameSize.height + info_size.height, col_pos+1};
				LocalToPortPoint(topLeft(line));
				LocalToPortPoint(botRight(line));
				PortToGlobalPoint(topLeft(line));
				PortToGlobalPoint(botRight(line));
				::RectRgn(gray_line, &line);

				// Create limit rect
				Rect limit = {0, col_pos - GetColWidth(i) + 16, mFrameSize.height, mFrameSize.width - mImageLocation.h};
				if (limit.left < -mImageLocation.h)
					limit.left = -mImageLocation.h;
				LocalToPortPoint(topLeft(limit));
				LocalToPortPoint(botRight(limit));
				PortToGlobalPoint(topLeft(limit));
				PortToGlobalPoint(botRight(limit));

				// Create slop rect
				SDimension16 super_size;
				mSuperView->GetFrameSize(super_size);
				Rect slop = {-32, -32, super_size.height + 32, super_size.width + 32};
				mSuperView->LocalToPortPoint(topLeft(slop));
				mSuperView->LocalToPortPoint(botRight(slop));
				mSuperView->PortToGlobalPoint(topLeft(slop));
				mSuperView->PortToGlobalPoint(botRight(slop));

				// Get start point (force onto column)
				Point startPt = inMouseDown.whereLocal;
				startPt.h = col_pos;
				LocalToPortPoint(startPt);
				PortToGlobalPoint(startPt);

				// Focus on desktop
				GrafPtr save_port;
				::GetPort(&save_port);
				::SetPort(UScreenPort::GetScreenPort());

				// Drag gray line
				SInt32 moved = ::DragGrayRgn(gray_line, startPt, &limit ,&slop, hAxisOnly, nil);

				// Clear up
				::DisposeRgn(gray_line);
				::SetPort(save_port);

				// Adjust column only if moved
				if (moved != 0x80008000)
				{
					SInt16 new_width = GetColWidth(i) + moved & 0x0000FFFF;

					// Resize columns
					if (new_width < 16)
						new_width = 16;
					mTableWindow->SetColumnWidth(i, new_width);
				}

				return;
			}
		}
	}

	CTableDrag::ClickSelf(inMouseDown);
}

// Stop clicks
void CTitleTable::ClickCell(const STableCell& inCell,
									const SMouseDownEvent &inMouseDown)
{
	if (mTableWindow == NULL)
		return;

	// Get column info for cell
	SColumnInfo col_info = GetColumns()[inCell.col - 1];

	if (mMenuID &&
		(CContextMenuProcessAttachment::IsCMMEvent(inMouseDown.macEvent) || (inMouseDown.macEvent.modifiers & cmdKey)))
	{

		// Do menu Select
		Point startPt = inMouseDown.wherePort;
		PortToGlobalPoint(startPt);
		::SetItemMark(mColumnChanger, col_info.column_type, 0x12);
		SInt32 result = ::PopUpMenuSelect(mColumnChanger, startPt.v, startPt.h, col_info.column_type);
		::SetItemMark(mColumnChanger, col_info.column_type, noMark);

		// Handle menu result
		bool sort_col = mCanSort && (mTableWindow->GetSortBy() == col_info.column_type);
		MenuChoice(inCell.col, sort_col, result & 0x0000FFFF);
	}

	else if (mCanSort)
	{
		// Track mouse down
		bool click_hit = false;
		{
			// Turn tracking on
			mTracking = true;
			mTrackingInside = true;

			Rect	cellRect;
			GetLocalCellRect(inCell, cellRect);

			// Draw cell in new state
			FocusDraw();
			DrawCell(inCell, cellRect);
			
			// Track mouse down
			while (::StillDown())
			{
				Point currPt;
				::GetMouse(&currPt);		// Must keep track if mouse moves from

				STableCell	hitCell;
				SPoint32	imagePt;
				
				LocalToImagePoint(currPt, imagePt);
				bool inside = GetCellHitBy(imagePt, hitCell) && (hitCell == inCell);
				if (inside ^ mTrackingInside)
				{
					mTrackingInside = inside;
					DrawCell(inCell, cellRect);
				}
			}
			
			EventRecord	macEvent;			// Get location from MouseUp event
			if (UEventMgr::GetMouseUp(macEvent))
			{
				Point currPt = macEvent.where;
				::GlobalToLocal(&currPt);

				STableCell	hitCell;
				SPoint32	imagePt;
				LocalToImagePoint(currPt, imagePt);
				click_hit = GetCellHitBy(imagePt, hitCell) && (hitCell == inCell);
			}
			
			// Reset flags
			mTracking = false;
			mTrackingInside = false;
		}

		// Process click if hit
		if (click_hit)
		{
			// Determine if this is the current sort by method
			int sort = mTableWindow->GetSortBy();
			bool sort_col = (sort == col_info.column_type);

			if (!sort_col)
				// Change sort by
				mTableWindow->SetSortBy(col_info.column_type);
			else
			{
				// Must redraw cell to keep it correct
				Rect	cellRect;
				GetLocalCellRect(inCell, cellRect);
				DrawCell(inCell, cellRect);

				// Force change to sort direction
				// Do this after drawing cell as port gets reset
				mTableWindow->ToggleShowBy();
			}
		}
	}
}

void CTitleTable::DrawBackground()
{
	// Don't draw any background - let the parent view handle it
}

// Draw the titles
void CTitleTable::DrawCell(const STableCell& inCell, const Rect &inLocalRect)
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Determine which heading it is and draw it
	SColumnInfo col_info;
	if (mTableWindow != NULL)
		col_info = GetColumns()[inCell.col - 1];
	else
		col_info.column_type = inCell.col;

	// Determine if this is the current sort by method
	int sort = (mTableWindow != NULL) ? mTableWindow->GetSortBy() : 0;
	bool sort_col = (sort == col_info.column_type) && mCanSort;

	// Handle tracking highlighting
	if (mTracking && mTrackingInside)
		sort_col = !sort_col;

	Rect r = inLocalRect;
	::OffsetRect(&r, 1, 1);
	ThemeButtonDrawInfo theme;
	theme.state = (IsActive() && IsEnabled()) ? (sort_col ? kThemeStatePressed : kThemeStateActive) : kThemeStateInactive;
	theme.value = kThemeButtonOff;
	theme.adornment = kThemeAdornmentNone;
	::DrawThemeButton(&r, kThemeSmallBevelButton, &theme, NULL, NULL, NULL, NULL);

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	DrawItem(col_info, sort_col, inLocalRect);
}


// Draw an item
void CTitleTable::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	// Default to draw text
	DrawText(col_info, sort_col, inLocalRect);
}

// Draw the titles
void CTitleTable::DrawText(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	// Move to origin for text
	::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);

	// Clip to cell frame & table frame & draw string
	if ((col_info.column_type >= 1) && (col_info.column_type <= mTitles.size()))
	{
		Rect clipper = mRevealedRect;
		PortToLocalPoint(topLeft(clipper));
		PortToLocalPoint(botRight(clipper));
		::SectRect(&clipper,&inLocalRect,&clipper);
		StClipRgnState	clip(clipper);
		RGBColor textColor = sort_col ? UGAColorRamp::GetWhiteColor() : UGAColorRamp::GetBlackColor();
		if (!sort_col && !IsActive())
			textColor = UGraphicUtils::Lighten(textColor);
		::RGBForeColor(&textColor);

		::DrawClippedStringUTF8(mTitles[col_info.column_type - 1], inLocalRect.right - inLocalRect.left, RightJustify(col_info.column_type) ? eDrawString_Right : eDrawString_Left);
	}
}

// Draw the titles
void CTitleTable::DrawIcon(SColumnInfo& col_info, ResIDT icon, bool sort_col, const Rect &inLocalRect)
{
	// Clip to cell frame & table frame & draw string
	unsigned long offset = ((inLocalRect.right - inLocalRect.left) > 24) ? 3 : 0;
	Rect	iconRect;
	if (RightJustify(col_info.column_type))
	{
		iconRect.right = inLocalRect.right - offset;
		iconRect.left = iconRect.right - 16;
	}
	else
	{
		iconRect.left = inLocalRect.left + offset;
		iconRect.right = iconRect.left + 16;
	}
	iconRect.bottom = inLocalRect.bottom - mIconDescent + 1;
	iconRect.top = iconRect.bottom - 16;
	::Ploticns(&iconRect, atNone, IsActive() ? ttNone : ttDisabled, icon);
}

// Display column change cursor
void CTitleTable::AdjustMouseSelf(Point inPortPt, const EventRecord& inMacEvent, RgnHandle outMouseRgn)
{
	// Is it near column edge
	if (mCanSize)
	{
		SInt16	col_pos = 0;
		Point tempPt = inPortPt;
		PortToLocalPoint(tempPt);

		for(SInt16 i = 1; i <= mCols; i++) {
			col_pos += GetColWidth(i);
			if ((tempPt.h > col_pos - 2) && (tempPt.h < col_pos +2)) {
				UCursor::SetThemeCursor(kThemeResizeLeftRightCursor);
				return;
			}
		}
	}

	// Do inherited
	CTableDrag::AdjustMouseSelf(inPortPt, inMacEvent, outMouseRgn);
}

// Sync columns with the table
void CTitleTable::SyncTable(const LTableView* table, bool reposition)
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
		
		SDimension16 title_size;
		GetFrameSize(title_size);
		
		int delta = GetRowHeight(1) - title_size.height;
		if (delta != 0)
		{
			ResizeFrameBy(0, delta, true);
			table->GetSuperView()->ResizeFrameBy(0, -delta, true);
			table->GetSuperView()->MoveBy(0, delta, false);
		}
	}
}
