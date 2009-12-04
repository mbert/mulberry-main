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

#include "CContextMenu.h"
#include "CMulberryCommon.h"
#include "CSplitDivider.h"
#include "CTableView.h"
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
CTitleTableView::CTitleTableView()
{
	InitTitleTable();
}

// Constructor from stream
CTitleTableView::CTitleTableView(LStream *inStream)
	: CTableDrag(inStream)
{
	InitTitleTable();
}

// Default destructor
CTitleTableView::~CTitleTableView()
{
}

// Do common init
void CTitleTableView::InitTitleTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, 72, 16);

	mCanSort = true;
	mCanSize = true;
	mValue = 0;

	SetRowSelect(true);
	
	mTracking = false;
	mTrackingInside = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CTitleTableView::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Get table window in heirarchy
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CTableView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CTableView*>(super);

	InitColumnChanger();

	// Always read-only for dd
	SetTable(this, false);
	SetDDReadOnly(true);

	CContextMenuAttachment::AddUniqueContext(this, new CNULLContextMenuAttachment);
}

// Get column info
CColumnInfoArray& CTitleTableView::GetColumns()
{
	return mTableView->mColumnInfo;
}

void CTitleTableView::SetTitleInfo(bool can_sort, bool can_size, const char* rsrcid, unsigned long max_str, ResIDT menu_id)
{
	mCanSort = can_sort;
	mCanSize = can_size;
	mMaxString = max_str;
	mMenuID = menu_id;

	// Get titles from resource
	mTitles.clear();
	for(unsigned long i = 0; i < mMaxString; i++)
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

void CTitleTableView::ActivateSelf ()
{

	// е Get everything redrawn so the changes take effect
	if ( mActive == triState_On )
		Refresh ();

	CTableDrag::ActivateSelf();
}


void CTitleTableView::DeactivateSelf ()
{

	// е Get everything redrawn so the changes take effect
	if ( mActive == triState_Off || mActive == triState_Latent )
		Refresh ();

	CTableDrag::DeactivateSelf();
}

// Allow column size change
void CTitleTableView::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// A click on this must active its table
	LCommander* cmdr = dynamic_cast<LCommander*>(mTableView->GetBaseTable());
	if (cmdr)
		SwitchTarget(cmdr);

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
				LTableView* info_table = mTableView->mTable;
				SDimension16 info_size;
				info_table->GetFrameSize(info_size);

				// Create gray line region
				RgnHandle	gray_line = ::NewRgn();
				Rect line = {0, col_pos, mFrameSize.height/* + info_size.height*/, col_pos+14};
				LocalToPortPoint(topLeft(line));
				LocalToPortPoint(botRight(line));
				PortToGlobalPoint(topLeft(line));
				PortToGlobalPoint(botRight(line));
				::RectRgn(gray_line, &line);

				// Create limit rect
#if 0
				Rect limit = {0, col_pos - GetColWidth(i) + 16, mFrameSize.height, mFrameSize.width - mImageLocation.h};
				if (limit.left < -mImageLocation.h)
					limit.left = -mImageLocation.h;
#else
				Rect limit = {0, col_pos - GetColWidth(i) + 16, mFrameSize.height, mFrameSize.width};
				if (limit.left < 0)
					limit.left = 0;
#endif
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
				//::SetPort(UScreenPort::GetScreenPort());

				// Drag gray line
				//SInt32 moved = ::DragGrayRgn(gray_line, startPt, &limit ,&slop, hAxisOnly, nil);
				Point outPt = startPt;
				MouseTrackingResult result = kMouseTrackingMouseDown;
				SInt16 old_width = GetColWidth(i);
				mTableView->Draw(NULL);
				Draw(NULL);
				while (result != kMouseTrackingMouseUp) {
					//::SetPort(save_port);
					UMouseTracking::TrackMouseDown(save_port, outPt, result);
					LocalToPortPoint(outPt);
					PortToGlobalPoint(outPt);
					SInt32 moved = outPt.h - startPt.h;

					// Clear up
					//::DisposeRgn(gray_line);
					//::SetPort(save_port);

					// Adjust column only if moved
					if (moved != 0x80008000)
					{
						SInt16 new_width = old_width + moved & 0x0000FFFF;

						// Resize columns
						if (new_width < 16)
							new_width = 16;
						mTableView->SetColumnWidth(i, new_width);
						mTableView->Draw(NULL);
						Draw(NULL);
					}
				}

				return;
			}
		}
	}

	CTableDrag::ClickSelf(inMouseDown);
}

// Stop clicks
void CTitleTableView::ClickCell(const STableCell& inCell,
									const SMouseDownEvent &inMouseDown)
{
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
		bool sort_col = mCanSort && (mTableView->GetSortBy() == col_info.column_type);
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
			int sort = mTableView->GetSortBy();
			bool sort_col = (sort == col_info.column_type);

			if (!sort_col)
				// Change sort by
				mTableView->SetSortBy(col_info.column_type);
			else
			{
				// Must redraw cell to keep it correct
				Rect	cellRect;
				GetLocalCellRect(inCell, cellRect);
				DrawCell(inCell, cellRect);

				// Force change to sort direction
				// Do this after drawing cell as port gets reset
				mTableView->ToggleShowBy();
			}
		}
	}
}

void CTitleTableView::DrawBackground()
{
	// Don't draw any background - let the parent view handle it
}

// Draw the titles
void CTitleTableView::DrawCell(const STableCell& inCell, const Rect &inLocalRect)
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Determine which heading it is and draw it
	SColumnInfo col_info = GetColumns()[inCell.col - 1];

	// Determine if this is the current sort by method
	int sort = mTableView->GetSortBy();
	bool sort_col = (sort == col_info.column_type) && mCanSort;

	// Handle tracking highlighting
	if (mTracking && mTrackingInside)
		sort_col = !sort_col;

#if PP_Target_Carbon
	if (UEnvironment::HasFeature(env_HasAquaTheme))
	{
		Rect r = inLocalRect;
		::OffsetRect(&r, 1, 1);
		ThemeButtonDrawInfo theme;
		theme.state = (IsActive() && IsEnabled()) ? (sort_col ? kThemeStatePressed : kThemeStateActive) : kThemeStateInactive;
		theme.value = kThemeButtonOff;
		theme.adornment = kThemeAdornmentNone;
		::DrawThemeButton(&r, kThemeSmallBevelButton, &theme, NULL, NULL, NULL, NULL);
	}
	else
#endif
	{
		// Erase rect to gray
		Rect r = inLocalRect;
		r.left++;
		r.right++;
		r.bottom++;
		if (!(IsActive() && IsEnabled()) || sort_col)
			r.top++;

		if (sort_col)
		{
			mValue = inCell.col;	// Keep this up to date

			// Paint background
			RGBColor temp = UGAColorRamp::GetColor(7);
			::RGBForeColor(&temp);
			::PaintRect(&r);
		}
		// Draw 3D frame
		else
		{
			// Need to repaint background
			RGBColor temp = UGAColorRamp::GetColor(2);
			::RGBForeColor(&temp);
			::PaintRect(&r);
				
			if (IsActive() && IsEnabled())
			{
				RGBColor colorArray[2];

				// ее LIGHT BEVELS
				// е Setup Colors
				colorArray[0] = UGAColorRamp::GetColor(3);
				colorArray[1] = UGAColorRamp::GetWhiteColor();

				// е Draw top and left edges
				UGraphicUtils::BevelledEdge(r,
													rectEdges_TopLeft,
													colorArray,
													2);
				// ее SHADOW BEVELS
				// е Setup Colors
				colorArray[0] = UGAColorRamp::GetColor(8);
				colorArray[1] = UGAColorRamp::GetColor(6);

				// е Draw bottom and right edges
				UGraphicUtils::BevelledEdge(r,
													rectEdges_BottomRight,
													colorArray,
													2);
				// ее CORNER PIXELS
				colorArray[0] = UGAColorRamp::GetColor(5);
				colorArray[1] = UGAColorRamp::GetColor(3);

				// е Paint corner pixels
				// е TOP RIGHT
				UGraphicUtils::PaintCornerPixels(r,
														rectCorners_TopRight,
														colorArray,
														2);

				// е BOTTOM LEFT
				UGraphicUtils::PaintCornerPixels(r,
														rectCorners_BottomLeft,
														colorArray,
														2);
			}
			else
			{
				// е DISABLED NORMAL STATE
				r.left--;
				r.bottom++;

				RGBColor temp = UGAColorRamp::GetColor(5);
				::RGBForeColor(&temp);
				UGraphicUtils::TopLeftSide(r, 0, 0, 0, 0);
				temp = UGAColorRamp::GetColor(6);
				::RGBForeColor(&temp);
				UGraphicUtils::BottomRightSide(r, 1, 1, 0, 0);
			}
		}
	}

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	DrawItem(col_info, sort_col, inLocalRect);
}


// Draw an item
void CTitleTableView::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	// Default to draw text
	DrawText(col_info, sort_col, inLocalRect);
}

// Draw the titles
void CTitleTableView::DrawText(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
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
void CTitleTableView::DrawIcon(SColumnInfo& col_info, ResIDT icon, bool sort_col, const Rect &inLocalRect)
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
void CTitleTableView::AdjustMouseSelf(Point inPortPt, const EventRecord& inMacEvent, RgnHandle outMouseRgn)
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
void CTitleTableView::SyncTable(const LTableView* table, bool reposition)
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
