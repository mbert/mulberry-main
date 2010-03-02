/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSimpleTitleTable.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include "CICalendarCalAddressValue.h"
#include "CITIPProcessor.h"

BEGIN_MESSAGE_MAP(CAttendeeTable, CTableDragAndDrop)
	ON_WM_CREATE()
END_MESSAGE_MAP()

CAttendeeTable::CAttendeeTable()
{
	mTableGeometry = new CTableRowGeometry(this,16, 16);
	mTableSelector = new CTableRowSelector(this);
}


CAttendeeTable::~CAttendeeTable()
{
}

int CAttendeeTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableDragAndDrop::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitTable();
	
	return 0;
}

BOOL CAttendeeTable::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	// Do inherited
	BOOL result = CTableDragAndDrop::SubclassDlgItem(nID, pParent);

	InitTable();

	return result;
}

void CAttendeeTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Save text state in stack object
	cdstring			theTxt;
	StDCState		save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Move to origin for text
	int x = inLocalRect.left + 4;
	int y = inLocalRect.top + mTextOrigin;

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
		// Determine participation status
		theTxt = iCal::CITIPProcessor::GetAttendeeStatusDescriptor(prop);
		break;

	default:
		break;
	}

	if (theTxt.length() > 0)
	{
		short text_style = normal;
		const CIdentity* id = NULL;
		if (iCal::CITIPProcessor::AttendeeIdentity(prop, id))
			text_style = bold;
		if (prop.GetCalAddressValue()->GetValue() == mOrganizer->GetCalAddressValue()->GetValue())
			text_style = italic;
		switch(text_style)
		{
		case normal:
		default:
			pDC->SelectObject(CFontCache::GetListFont());
			break;
		case bold:
			pDC->SelectObject(CFontCache::GetListFontBold());
			break;
		case italic:
			pDC->SelectObject(CFontCache::GetListFontItalic());
			break;
		case bold + italic:
			pDC->SelectObject(CFontCache::GetListFontBoldItalic());
			break;
		}
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, eDrawString_Left);
	}
}

void CAttendeeTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID() + 1);
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
	InsertCols(3, 1, NULL, 0, false);
	SetColWidth(206, 1, 1);
	SetColWidth(126, 2, 2);
	SetColWidth(118, 3, 3);

	mTitles->SyncTable(this, true);
}

void CAttendeeTable::SelectionChanged()
{
	GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());

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

	RedrawWindow();
}
