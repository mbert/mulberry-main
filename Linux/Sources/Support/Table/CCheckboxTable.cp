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

#include "CTableRowGeometry.h"

#include <JXTextCheckbox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCheckboxTable::CCheckboxTable(JXScrollbarSet* scrollbarSet,
						  JXContainer* enclosure,
						  const HSizingOption hSizing, 
						  const VSizingOption vSizing,
						  const JCoordinate x, const JCoordinate y,
						  const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, 
							x, y, w, h)
{
	mTableGeometry = new CTableRowGeometry(this, 16, 18);
}

// Default destructor
CCheckboxTable::~CCheckboxTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Broadcast message when selected cells change

void CCheckboxTable::SetContents(const cdstrvect& items)
{
	// Make sure columns are present
	if (mCols != 1)
	{
		InsertCols(1, 0);
		SetColWidth(GetApertureWidth(), 1, 1);
	}

	DeleteEnclosedObjects();
	RemoveAllRows(false);
	mItems.clear();
	
	if (items.size())
	{
		InsertRows(items.size(), 0);
		SetRowHeight(20, 1, items.size());

		TableIndexT row = 0;
		for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++, row++)
		{
			// Create a checkbox
	        JXTextCheckbox* cbox = new JXTextCheckbox(*iter, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, row * 20, GetApertureWidth(), 20);
	        mItems.push_back(cbox);
		}
	}

	Refresh();
}

void CCheckboxTable::SetSelection(const boolvector& selection)
{
	TableIndexT row = 0;
	for(boolvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++, row++)
	{
		JXTextCheckbox* cbox = mItems.at(row);
		if (cbox)
			cbox->SetState(JBoolean(*iter));
	}
}

void CCheckboxTable::GetSelection(boolvector& selection) const
{
	for(TableIndexT row = 1; row <= mRows; row++)
	{
		JXTextCheckbox* cbox = mItems.at(row - 1);
		if (cbox)
			selection.push_back(cbox->IsChecked());
	}
}

// Resize columns
void CCheckboxTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CTableDragAndDrop::ApertureResized(dw, dh);

	//Since only the name column has variable width, we just
	//adjust it.
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
	}
}
