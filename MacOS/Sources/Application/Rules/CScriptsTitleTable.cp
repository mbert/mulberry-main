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

#include "CResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CScriptsTitleTable::CScriptsTitleTable()
{
}

// Constructor from stream
CScriptsTitleTable::CScriptsTitleTable(LStream *inStream)
	: CTitleTable(inStream)
{
}

// Default destructor
CScriptsTitleTable::~CScriptsTitleTable()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CScriptsTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(false, false, STRx_ScriptsTitles, str_ScriptsTitleName, 0);
	CTitleTable::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(2, 2, nil, 0, false);
	InsertRows(1, 1, nil, 0, false);
	AdaptToNewSurroundings();
}

// Get menu
void CScriptsTitleTable::InitColumnChanger(void)
{
	mColumnChanger = NULL;
}

// Get column info
CColumnInfoArray& CScriptsTitleTable::GetColumns()
{
	return mColumnInfo;
}

// Draw the titles
void CScriptsTitleTable::DrawItem(SColumnInfo& col_info, bool sort_col, const Rect &inLocalRect)
{
	if ((col_info.column_type == eRulesColumnApplyEnable) && (mCols == 2))
		DrawIcon(col_info, ICNx_Tickmark, sort_col, inLocalRect);
	else
	{
		// Always force this to 1 to get proper string index
		SColumnInfo temp;
		temp.column_type = 1;
		temp.column_width = col_info.column_width;

		DrawText(temp, sort_col, inLocalRect);
	}
}

// Handle choice
void CScriptsTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
}

