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


// Source for CScriptsTitleTable class

#include "CScriptsTitleTable.h"

#include "CMulberryCommon.h"

IMPLEMENT_DYNCREATE(CScriptsTitleTable, CHeaderCtrl)

BEGIN_MESSAGE_MAP(CScriptsTitleTable, CTitleTable)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

CStringArray CScriptsTitleTable::sTitles;
CIconArray CScriptsTitleTable::sIcons;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CScriptsTitleTable::CScriptsTitleTable()
{
	// Load strings if not already
	LoadTitles(IDS_TARGETS_TITLE_NAME, 1);
	LoadIcons(IDI_TICKMARK, 1);
	mMenuID = 0;
}

// Default destructor
CScriptsTitleTable::~CScriptsTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Draw the titles
void CScriptsTitleTable::DrawCol(CDC* pDC, SColumnInfo& col_info, const CRect &cellRect)
{
	if ((col_info.column_type == eRulesColumnApplyEnable) && (GetItemCount() == 2))
		DrawIcon(pDC, col_info, sIcons[0], cellRect);
	else
	{
		// Always force this to 1 to get proper string index
		SColumnInfo temp;
		temp.column_type = 1;
		temp.column_width = col_info.column_width;

		DrawText(pDC, temp, cellRect);
	}
}
