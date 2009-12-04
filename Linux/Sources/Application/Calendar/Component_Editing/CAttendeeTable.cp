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


//	CAttendeeTable.cp

#include "CAttendeeTable.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include "CICalendarCalAddressValue.h"
#include "CITIPProcessor.h"

#include "StPenState.h"

CAttendeeTable::CAttendeeTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTableGeometry = new CTableRowGeometry(this, w, 18);
	mTableSelector = new CTableRowSelector(this);
}


CAttendeeTable::~CAttendeeTable()
{
}

// Get details of sub-panes
void CAttendeeTable::OnCreate(void)
{
	// Do inherited
	CTableDragAndDrop::OnCreate();

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
	
	InitTable();
	EnableTooltips();
	
	ApertureResized(0, 0);
}

void CAttendeeTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods)
{
	Broadcast_Message(eBroadcast_DblClk, (void*) this);
}

void CAttendeeTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Save text state in stack object
	StPenState		save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get data for row
	cdstring theTxt;
	const iCal::CICalendarProperty& prop = mAttendees->at(inCell.row - 1);

	switch(inCell.col)
	{

	case 1: // Name
		theTxt = iCal::CITIPProcessor::GetAttendeeDescriptor(prop);
		break;

	case 2:	// Role
		theTxt = iCal::CITIPProcessor::GetAttendeeRoleDescriptor(prop);
		break;

	case 3:	// Status
		theTxt = iCal::CITIPProcessor::GetAttendeeStatusDescriptor(prop);
		break;

	default:
		break;
	}

	if (theTxt.length() > 0)
	{
		JFontStyle style;
		const CIdentity* id = NULL;
		if (iCal::CITIPProcessor::AttendeeIdentity(prop, id))
			style.bold = kTrue;
		if (prop.GetCalAddressValue()->GetValue() == mOrganizer->GetCalAddressValue()->GetValue())
			style.italic = kTrue;
		pDC->SetFontStyle(style);

		::DrawClippedStringUTF8(pDC, theTxt.c_str(), JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
	}
}

// Get text for current tooltip cell
void CAttendeeTable::GetTooltipText(cdstring& txt, const STableCell &inCell)
{
	// Get data for row
	const iCal::CICalendarProperty& prop = mAttendees->at(inCell.row - 1);
	txt = iCal::CITIPProcessor::GetAttendeeFullDescriptor(prop);
}

void CAttendeeTable::InitTable()
{
	// Create columns
	InsertCols(3, 1, NULL, 0, false);
	SetColWidth(176, 1, 1);
	SetColWidth(137, 2, 2);
	SetColWidth(137, 3, 3);
}

// Adjust column widths
void CAttendeeTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do inherited call
	CTableDragAndDrop::ApertureResized(dw, dh);

	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw - 176 - 98, 1, 1);
	}
}

void CAttendeeTable::SelectionChanged()
{
	Broadcast_Message(eBroadcast_SelectionChange, (void*) this);

	// Do inherited
	CTableDragAndDrop::SelectionChanged();
}

// Reset the table from the address list
void CAttendeeTable::ResetTable(const iCal::CICalendarPropertyList* items, const iCal::CICalendarProperty* organizer)
{

	mAttendees = items;
	mOrganizer = organizer;

	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	TableIndexT	old_rows;
	TableIndexT	old_cols;

	// Add cached mailboxes
	unsigned long num_atts = items->size();

	GetTableSize(old_rows, old_cols);

	if (old_rows > num_atts)
		RemoveRows(old_rows - num_atts, 1, false);
	else if (old_rows < num_atts)
		InsertRows(num_atts - old_rows, 1, NULL, 0, false);

	Refresh();
}
