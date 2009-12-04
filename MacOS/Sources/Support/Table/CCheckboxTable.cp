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


// Source for CCheckboxTable class

#include "CCheckboxTable.h"

#include "CMulberryCommon.h"
#include "CTableRowGeometry.h"

#include <LCheckBox.h>
#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCheckboxTable::CCheckboxTable()
{
	InitTextTable();
}

// Default constructor - just do parents' call
CCheckboxTable::CCheckboxTable(LStream *inStream)
		: CTableDrag(inStream)

{
	InitTextTable();
}

// Default destructor
CCheckboxTable::~CCheckboxTable()
{
}

// Common init
void CCheckboxTable::InitTextTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, 16, 18);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Broadcast message when selected cells change

void CCheckboxTable::SetContents(const cdstrvect& items)
{
	DeleteAllSubPanes();
	RemoveAllRows(false);
	
	if (items.size())
	{
		InsertRows(items.size(), 0, NULL, 0, false);

		// Get height of current row
		UInt16 rowHeight = GetRowHeight(1);
		UInt16 rowOffset = (rowHeight - 18) / 2;

		STableCell aCell(0, 1);
		TableIndexT row = 1;
		for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++, row++)
		{
			// Create a checkbox
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left =
			pane.bindings.right =
			pane.bindings.top =
			pane.bindings.bottom = false;
			pane.paneID = row;
			pane.width = 300;	// Provide enough width to keep control title on one line
			pane.height = 18;
			pane.left = 0;
			pane.top = (row - 1) * rowHeight + rowOffset;

			LCheckBox* cbox = new LCheckBox(pane);
			MyCFString temp(*iter, kCFStringEncodingUTF8);
			cbox->SetCFDescriptor(temp);
		}
	}

	// If currently visible, redo state setting to force checkboxes into proper state
	if (IsVisible())
	{
		Activate();
		Show();
	}
	Enable();

	Refresh();
}

void CCheckboxTable::SetSelection(const boolvector& selection)
{
	SInt32 row = 1;
	for(boolvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++, row++)
	{
		LCheckBox* cbox = static_cast<LCheckBox*>(FindPaneByID(row));
		if (cbox)
			cbox->SetValue(*iter);
	}
}

void CCheckboxTable::GetSelection(boolvector& selection) const
{
	SInt32 row = 1;
	for(SInt32 row = 1; row <= GetItemCount(); row++)
	{
		LCheckBox* cbox = static_cast<LCheckBox*>(const_cast<CCheckboxTable*>(this)->FindPaneByID(row));
		if (cbox)
			selection.push_back(cbox->GetValue());
	}
}

// Get details of sub-panes
void CCheckboxTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);

	// Get super frame's width - scroll bar
	SDimension16 super_frame;
	mSuperView->GetFrameSize(super_frame);
	super_frame.width -= 16;

	// Set image to super frame size
	ResizeImageTo(super_frame.width, mImageSize.height, true);

	// Set column width
	SetColWidth(super_frame.width, 1, 1);

}

