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

#include "CSummaryTable.h"

#include "CCalendarUtils.h"
#include "CDrawUtils.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSummaryEvent.h"
#include "CSummaryTitleTable.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include "StPenState.h"

#include <JPainter.h>
#include <JXColormap.h>

#include <algorithm>

const uint32_t cRowHeight = 18;
const uint32_t cLinkColumnWidth = 16;
const uint32_t cDateColumnWidth = 96;
const uint32_t cTimeColumnWidth = 72;
const uint32_t cSummaryColumnWidth = 196;
const uint32_t cStatusColumnWidth = 64;
const uint32_t cCalendarColumnWidth = 128;

const uint32_t cLinkHOffset = 8;
const uint32_t cLinkWidth = 4;
const uint32_t cLinkVOffset = 4;
const uint32_t cLinkHeight = 16;

// ---------------------------------------------------------------------------
//	CSummaryTable														  [public]
/**
	Default constructor */

CSummaryTable::CSummaryTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CCalendarTableBase(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTitles = NULL;

	mTableGeometry = new CTableRowGeometry(this, 128, cRowHeight);
	
	// Single selector
	mTableSelector = new CTableSingleRowSelector(this);
	
	SetRowSelect(true);
}


// ---------------------------------------------------------------------------
//	~CSummaryTable														  [public]
/**
	Destructor */

CSummaryTable::~CSummaryTable()
{
}

#pragma mark -

void CSummaryTable::OnCreate()
{
	// Call inherited
	CCalendarTableBase::OnCreate();

	InsertCols(6, 1, NULL);

	mColumnInfo.push_back(CColumnSpec(eLink, cLinkColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eDate, cDateColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eTime, cTimeColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eSummary, cSummaryColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eStatus, cStatusColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eCalendar, cCalendarColumnWidth));

	//SPoint32 pt = {cDateColumnWidth, cRowHeight};
	//SetScrollUnit(pt);

	ResetColumns();

	ApertureResized(0, 0);

	// Turn on tooltips
	EnableTooltips();

}

void CSummaryTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& mods)
{
	// Ignore clicks on non-events
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (!event->IsEvent())
	{
		return;
	}
	
	CCalendarTableBase::LClickCell(inCell, mods);
	DoSingleClick(inCell, CKeyModifiers(mods));
}

void CSummaryTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods)
{
	// Ignore clicks on non-events
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (!event->IsEvent())
	{
		return;
	}
	
	// Broadcast double-click
	DoDoubleClick(inCell, CKeyModifiers(mods));
}

void CSummaryTable::DrawCellRange(JPainter* pDC, const STableCell& inTopLeftCell, const STableCell& inBottomRightCell)
{
	STableCell	cell;
	
	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ )
	{
		// See whether this is a row item
		const CSummaryEvent* event = mEvents[cell.row - 1];
		if (event->IsEvent())
		{
			for ( cell.col = inTopLeftCell.col;
				  cell.col <= inBottomRightCell.col;
				  cell.col++ )
			{
				  
				JRect	cellRect;
				GetLocalCellRect(cell, cellRect);
				DrawCell(pDC, cell, cellRect);
			}
		}
		else
		{
			// Draw entire row
			JRect	rowRect;
			GetLocalCellRectAlways(STableCell(cell.row, 1), rowRect);

			JRect	cellRect;
			GetLocalCellRectAlways(STableCell(cell.row, mCols), cellRect);
			rowRect.right = cellRect.right;

			DrawRow(pDC, cell.row, rowRect);
		}
	}
}

void CSummaryTable::DrawRow(JPainter* pDC, TableIndexT row, const JRect& inLocalRect)
{
	StPenState save(pDC);

	const CSummaryEvent* event = mEvents[row - 1];

	CDrawUtils::DrawSimpleBorder(*pDC, inLocalRect, false, true);

	pDC->SetPenColor(pDC->GetColormap()->GetGray50Color());
	::DrawClippedStringUTF8(pDC, event->mSummary, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Center);
}

void CSummaryTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState		save(pDC);

	const CSummaryEvent* event = mEvents[inCell.row - 1];

	JRect adjustedRect = inLocalRect;

	bool link = (mColumnInfo[inCell.col - 1].first == eLink);

	// Do selection background
	float red = CCalendarUtils::GetRed(event->mColour);
	float green = CCalendarUtils::GetGreen(event->mColour);
	float blue = CCalendarUtils::GetBlue(event->mColour);
	if (CellIsSelected(inCell) && mDrawSelection)
		CCalendarUtils::UnflattenColours(red, green, blue);
	JColorIndex cindex;
	GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(red, green, blue), &cindex);
	pDC->SetPenColor(cindex);
	if (!link)
	{
		pDC->SetFilling(kTrue);
		pDC->Rect(adjustedRect);
		pDC->SetFilling(kFalse);
	}
	pDC->SetPenColor(GetColormap()->GetBlackColor());

	if (!link)
	{
		int adjust_bottom = (inCell.row == mRows) ? -1 : 0;
		int adjust_right = (inCell.col == mCols) ? 1 : 0;

		JColorIndex old_cindex = pDC->GetPenColor();
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(0.5, 0.5, 0.5), &cindex);
		pDC->SetPenColor(cindex);

		// Left-side always
		pDC->Line(adjustedRect.left, adjustedRect.bottom + adjust_bottom, adjustedRect.left, adjustedRect.top);

		// Right-side only for last column
		if (inCell.col == mCols)
		{
			pDC->Line(adjustedRect.right - adjust_right, adjustedRect.top, adjustedRect.right - adjust_right, adjustedRect.bottom + adjust_bottom);
		}

		// Top-side always
		{
			pDC->Line(adjustedRect.left, adjustedRect.top, adjustedRect.right - 1, adjustedRect.top);
		}

		// Bottom-side only for last row
		if (inCell.row == mRows)
		{
			pDC->Line(adjustedRect.left, adjustedRect.bottom + adjust_bottom, adjustedRect.right - 1, adjustedRect.bottom + adjust_bottom);
		}

		pDC->SetPenColor(old_cindex);
	}

	adjustedRect.Shrink(2, 1);
	adjustedRect.top--;
	
	// Get string data for column
	cdstring text;
	bool draw_txt = false;
	EDrawStringAlignment just = eDrawString_Left;
	switch(mColumnInfo[inCell.col - 1].first)
	{
	case eLink:
		if (event->mDayStart)
		{
			pDC->SetPenLocation(inLocalRect.left + cLinkHOffset + cLinkWidth, inLocalRect.top + cLinkVOffset);
			pDC->LineTo(inLocalRect.left + cLinkHOffset, inLocalRect.top + cLinkVOffset);
			pDC->LineTo(inLocalRect.left + cLinkHOffset, (inLocalRect.top + inLocalRect.bottom) / 2);
		}
		else
		{
			pDC->SetPenLocation(inLocalRect.left + cLinkHOffset, inLocalRect.top);
			pDC->LineTo(inLocalRect.left + cLinkHOffset, (inLocalRect.top + inLocalRect.bottom) / 2);
		}
		if (event->mDayEnd)
		{
			pDC->SetPenLocation(inLocalRect.left + cLinkHOffset, (inLocalRect.top + inLocalRect.bottom) / 2);
			pDC->LineTo(inLocalRect.left + cLinkHOffset, inLocalRect.bottom - cLinkVOffset);
			pDC->LineTo(inLocalRect.left + cLinkHOffset + cLinkWidth + 1, inLocalRect.bottom - cLinkVOffset);
		}
		else
		{
			pDC->SetPenLocation(inLocalRect.left + cLinkHOffset, (inLocalRect.top + inLocalRect.bottom) / 2);
			pDC->LineTo(inLocalRect.left + cLinkHOffset, inLocalRect.bottom);
		}
		break;
	case eDate:
		text = event->mStartDate;
		draw_txt = true;
		just = eDrawString_Right;
		break;
	case eTime:
		text = event->mStartTime;
		draw_txt = true;
		just = eDrawString_Right;
		break;
	case eSummary:
		text = event->mSummary;
		draw_txt = true;
		break;
	case eStatus:
		break;
	case eCalendar:
		text = event->mCalendar;
		draw_txt = true;
		break;
	}

	if (draw_txt)
	{
		::DrawClippedStringUTF8(pDC, text, JPoint(adjustedRect.left, adjustedRect.top + mTextOrigin), adjustedRect, just);
	}

	// Strike out if status is cancelled
	if (!link && event->mCancelled)
	{
		pDC->SetPenLocation(adjustedRect.left, (adjustedRect.top + adjustedRect.bottom)/2);
		pDC->LineTo(adjustedRect.right, (adjustedRect.top + adjustedRect.bottom)/2);
	}
}

// Draw or undraw active hiliting for a Cell
void CSummaryTable::HiliteCellActively(const STableCell &inCell, bool inHilite)
{
	// Only if real event
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (event->IsEvent())
		CCalendarTableBase::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CSummaryTable::HiliteCellInactively(const STableCell &inCell, bool inHilite)
{
	// Only if real event
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (event->IsEvent())
		CCalendarTableBase::HiliteCellInactively(inCell, inHilite);
}

void CSummaryTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Allow frame adapter to adjust size
	CCalendarTableBase::ApertureResized(dw, dh);
	
	// Change column info widths here
	
	// Columns changed
	ResetColumns();
}

// Keep titles in sync
void CSummaryTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	if (mTitles)
		mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}
