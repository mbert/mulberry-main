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


#include <UNX_LTextHierTable.h>
#include <UNX_LCollapsableTree.h>

#include "StPenState.h"

#include <JPainter.h>

LTextHierTable::LTextHierTable(JXScrollbarSet* scrollbarSet, 
								JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
  : LHierarchyTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}


LTextHierTable::~LTextHierTable()
{
}


void LTextHierTable::DrawCell(JPainter* pDC,
								const STableCell&	inCell,
								const JRect&		inLocalRect)
{
	StPenState save(pDC);

	// Get cell data item
	int	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	const char* data = GetCellString(woRow);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Use different font for heading
	if (!nestingLevel)
	{
		// Use bold font
		JFontStyle text_style = pDC->GetFontStyle();
		text_style.bold = kTrue;
		pDC->SetFontStyle(text_style);
	}

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, data, NULL);
}
