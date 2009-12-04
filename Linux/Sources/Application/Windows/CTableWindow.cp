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


// Source for CTableWindow class

#include "CTableWindow.h"

#include "CCommands.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTitleTable.h"
#include "CTable.h"

// Default constructor
CTableWindow::CTableWindow(JXDirector *owner) : CWindow(owner)
{
	mTable = NULL;
	mTitles = NULL;
	mNewColumn = 0;
	mNewSize = 16;
}

// Default destructor
CTableWindow::~CTableWindow()
{
	// Set status
	SetClosing();

	// Add info to prefs (must do while mbox open)
	SaveState();

	// Set status
	SetClosed();
}

// Setup help balloons
void CTableWindow::PostCreate(CTable* tbl, CTitleTable* titles)
{
	mTable = tbl;
	mTitles = titles;

	// Init columns
	InitColumns();

	// Get text traits resource
#ifndef USE_FONTMAPPER
	ResetFont(CPreferences::sPrefs->mListTextFontInfo.GetValue());
#else
	ResetFont(CPreferences::sPrefs->mListFontMap.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits);
#endif
}

// Insert a column
void CTableWindow::InsertColumn(short col_pos)
{
	// Add required column to tables
	mTitles->InsertCols(1, col_pos - 1, NULL, 0, false);
	mTable->UnselectAllCells();
	mTable->InsertCols(1, col_pos - 1, NULL, 0, false);

	// Add to column info
	SColumnInfo newInfo;

	// Replace with new info
	newInfo.column_type = mNewColumn;
	newInfo.column_width = mNewSize;
	mColumnInfo.insert(mColumnInfo.begin() + (col_pos - 1), newInfo);

	SetColumnWidth(col_pos, mNewSize);

	// Refresh panes
	mTitles->Refresh();
	mTable->Refresh();
}

// Delete a column
void CTableWindow::DeleteColumn(short col_pos)
{
	if (mColumnInfo.size() > 1)
	{
		// Remove required column
		mTitles->RemoveCols(1, col_pos, false);
		mTable->UnselectAllCells();
		mTable->RemoveCols(1, col_pos, false);
		
		// Remove from column info
		mColumnInfo.erase(mColumnInfo.begin() + (col_pos - 1));
		
		// Refresh panes
		mTitles->Refresh();
		mTable->Refresh();
	}
	else
		// Must have at least one column
		::MessageBeep(1);
}

// Set type of a column
void CTableWindow::SetColumnType(short col_pos,
									int col_type)
{
	// Get current info
	SColumnInfo& newInfo = mColumnInfo[col_pos - 1];

	// Replace with new info
	newInfo.column_type = col_type;
	
	mTitles->RefreshCol(col_pos);
	mTable->RefreshCol(col_pos);
}

// Set width of column
void CTableWindow::SetColumnWidth(short col_pos,
									short col_width)
{
	// Get current info
	SColumnInfo& newInfo = mColumnInfo[col_pos - 1];

	// Replace with new info
	newInfo.column_width = col_width;

	// Set width in each table
	mTitles->SetColWidth(col_width, col_pos, col_pos);
	mTable->SetColWidth(col_width, col_pos, col_pos);

	mTitles->Refresh();
	mTable->Refresh();
}

// Get sort
int CTableWindow::GetSortBy(void)
{
	// Windows that sort will override
	return 0;
}

// Force change of sort
void CTableWindow::SetSortBy(int sort)
{
	// Windows that sort will override
}

// Get sort direction
int CTableWindow::GetShowBy(void)
{
	// Windows that sort will override
	return 0;
}

// Force change of sort direction
void CTableWindow::SetShowBy(int sort)
{
	// Windows that sort will override
}

// Force toggle of sort direction
void CTableWindow::ToggleShowBy()
{
	// Windows that sort will override
}

// Set window state
void CTableWindow::ResetColumns(const CColumnInfoArray& newCols)
{
	// Adjust size of tables
	unsigned long old_cols = mColumnInfo.size();
	unsigned long new_cols = newCols.size();
	if (old_cols > new_cols)
	{
		for(unsigned long i = old_cols; i > new_cols; i--)
			DeleteColumn(i);
	}
	else if (old_cols < new_cols)
	{
		for(unsigned long i = old_cols+1; i <= new_cols; i++)
			InsertColumn(i);
	}

	// Remove existing column info
	mColumnInfo.clear();

	// Copy items in column list
	TableIndexT col = 0;
	for(CColumnInfoArray::const_iterator iter = newCols.begin(); iter != newCols.end(); iter++)
	{
		SColumnInfo info = *iter;

		col++;
		mColumnInfo.push_back(info);
		mTitles->SetColWidth(info.column_width, col, col);
		mTable->SetColWidth(info.column_width, col, col);
	}
}

// Reset text traits from prefs
void CTableWindow::ResetFont(const SFontInfo& list_traits)
{
	mTitles->ResetFont(list_traits);
	mTable->ResetFont(list_traits);
	mTitles->SyncTable(mTable, true);
}

void CTableWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eWindowsSaveDefault:
	case CCommand::eWindowsResetToDefault:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;
	default:;
	}

	// Call inherited
	CWindow::UpdateCommand(cmd, cmdui);
}

bool CTableWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eWindowsSaveDefault:
		SaveDefaultState();
		return true;
	case CCommand::eWindowsResetToDefault:
		ResetState(true);
		return true;
	default:;
	}

	// Call inherited
	return CWindow::ObeyCommand(cmd, menu);
}

CCommander* CTableWindow::GetTarget()
{
	// Route commands through table, which will reroute back to us
	if (mTable)
		return mTable;
	else
		return this;
}
