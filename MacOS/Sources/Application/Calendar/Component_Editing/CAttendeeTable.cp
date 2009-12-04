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
#include "CResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include "CICalendarCalAddressValue.h"
#include "CITIPProcessor.h"

CAttendeeTable::CAttendeeTable(LStream *inStream)
	: CTableDrag(inStream)

{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
}


CAttendeeTable::~CAttendeeTable()
{
}

// Get details of sub-panes
void CAttendeeTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
	
	InitTable();
	EnableTooltips();
}

void CAttendeeTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Draw selection
	DrawCellSelection(inCell);

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	// Get data for row
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
		Style text_style = normal;
		const CIdentity* id = NULL;
		if (iCal::CITIPProcessor::AttendeeIdentity(prop, id))
			text_style |= bold;
		if (prop.GetCalAddressValue()->GetValue() == mOrganizer->GetCalAddressValue()->GetValue())
			text_style |= italic;
		::TextFace(text_style);

		::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
		short width = inLocalRect.right - inLocalRect.left;
		::DrawClippedStringUTF8(theTxt, width, eDrawString_Left);
	}
}

// Handle button hit
void CAttendeeTable::ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	if (GetClickCount() == 1)
		// Carry on to do default action
		CTableDrag::ClickCell(inCell, inMouseDown);

	else if (GetClickCount() == 2)
	{
		BroadcastMessage(GetPaneID() + 1, (void*) this);
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
	SDimension16 frame;
	GetFrameSize(frame);

	InsertCols(3, 1, NULL, 0, false);
	SetColWidth(176, 1, 1);
	SetColWidth(176, 2, 2);
	SetColWidth(98, 3, 3);
}

// Adjust column widths
void CAttendeeTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CTableDrag::AdaptToNewSurroundings();

	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	// Name column has variable width
	//SetColWidth(mFrameSize.width - 128, 1, 1);
}

void CAttendeeTable::SelectionChanged()
{
	BroadcastMessage(GetPaneID(), (void*) this);

	// Do inherited
	CTableDrag::SelectionChanged();
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
