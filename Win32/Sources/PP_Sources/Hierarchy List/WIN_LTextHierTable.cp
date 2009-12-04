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


#include <WIN_LTextHierTable.h>
#include <WIN_LCollapsableTree.h>

#include "CDrawUtils.h"
#include "CFontCache.h"


LTextHierTable::LTextHierTable()
{
}


LTextHierTable::~LTextHierTable()
{
}


void LTextHierTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	StDCState save(pDC);

	// Get cell data item
	int	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	const char* data = GetCellString(woRow);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Use different font for heading
	if (!nestingLevel)
	{
		// Use bold font
		pDC->SelectObject(CFontCache::GetListFontBold());
	}

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, data, 0U, CellIsSelected(inCell));
}
