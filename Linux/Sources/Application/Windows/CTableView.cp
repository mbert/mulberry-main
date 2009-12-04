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


// Source for CTableView class

#include "CTableView.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTitleTableView.h"
#include "CTable.h"
#include "CTableViewWindow.h"
#include "CTableWindow.h"
#include "C3PaneWindow.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableView::CTableView(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h)
	: CBaseView(enclosure, hSizing, vSizing, x, y, w, h)
{
	mTitles = NULL;
	mTable = NULL;
	mNewColumn = 0;
	mNewSize = 16;
	mHierarchic = false;
}

// Default destructor
CTableView::~CTableView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CTableView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTable::eBroadcast_Activate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewActivate, this);
		break;
	case CTable::eBroadcast_Deactivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewDeactivate, this);
		break;
	case CTable::eBroadcast_SelectionChanged:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ViewSelectionChanged, this);
		break;
	}
}		

// Focus on the table
bool CTableView::HasFocus() const
{
	return GetBaseTable()->IsTarget();
}

// Focus on the table
void CTableView::Focus()
{
	if (!Is3Pane())
		// Make window active
		FRAMEWORK_WINDOW_TO_TOP(GetOwningWindow())

	// Always focus on table
	if (IsVisible())
		GetBaseTable()->Focus();
}

void CTableView::PostCreate(CTable* table, CTitleTableView* titles)
{
	mTable = table;
	mTitles = titles;

	// Must listen to the view
	mTable->Add_Listener(this);

	// Init columns
	InitColumns();

	// Force title height to default size in case current font is bigger.
	// This will ensure proper relative shift when font is reset.
	mTitles->SetRowHeight(16, 1, 1);

	// Get text traits resource
	ResetFont(CPreferences::sPrefs->mListTextFontInfo.GetValue());
}

// Insert a column
void CTableView::InsertColumn(short col_pos, int col_type, short col_width)
{
	// Add to column info
	SColumnInfo newInfo;

	// Replace with new info
	newInfo.column_type = col_type;
	newInfo.column_width = col_width;
	mColumnInfo.insert(mColumnInfo.begin() + (col_pos - 1), newInfo);

	// Add required column to titles before changing table
	if (mTitles != NULL)
		mTitles->InsertCols(1, col_pos - 1, NULL, 0, false);

	// Add required column to tables
	mTable->UnselectAllCells();
	mTable->InsertCols(1, col_pos - 1);

	SetColumnWidth(col_pos, mNewSize);

	// Refresh panes
	if (mTitles != NULL)
		mTitles->Refresh();
	mTable->Refresh();
}

// Delete a column
void CTableView::DeleteColumn(short col_pos)
{
	if (mColumnInfo.size() > 1)
	{
		// Remove required column
		if (mTitles != NULL)
			mTitles->RemoveCols(1, col_pos, false);
		mTable->UnselectAllCells();
		mTable->RemoveCols(1, col_pos, false);
		
		// Remove from column info
		mColumnInfo.erase(mColumnInfo.begin() + (col_pos - 1));
		
		// Refresh panes
		if (mTitles != NULL)
			mTitles->Refresh();
		mTable->Refresh();
	}
	else
		// Must have at least one column
		::MessageBeep(1);
}

// Set type of a column
void CTableView::SetColumnType(short col_pos,
									int col_type)
{
	// Get current info
	SColumnInfo& newInfo = mColumnInfo[col_pos - 1];

	// Replace with new info
	newInfo.column_type = col_type;
	
	if (mTitles != NULL)
		mTitles->RefreshCol(col_pos);
	mTable->RefreshCol(col_pos);
}

// Set width of column
void CTableView::SetColumnWidth(short col_pos,
									short col_width)
{
	// Get current info
	SColumnInfo& newInfo = mColumnInfo[col_pos - 1];

	// Replace with new info
	newInfo.column_width = col_width;

	// Set width in each table
	if (mTitles != NULL)
		mTitles->SetColWidth(col_width, col_pos, col_pos);
	mTable->SetColWidth(col_width, col_pos, col_pos);

	if (mTitles != NULL)
		mTitles->Refresh();
	mTable->Refresh();
}

// Get sort
int CTableView::GetSortBy(void)
{
	// Windows that sort will override
	return 0;
}

// Force change of sort
void CTableView::SetSortBy(int sort)
{
	// Windows that sort will override
}

// Get sort direction
int CTableView::GetShowBy(void)
{
	// Windows that sort will override
	return 0;
}

// Force change of sort direction
void CTableView::SetShowBy(int sort)
{
	// Windows that sort will override
}

// Force toggle of sort direction
void CTableView::ToggleShowBy()
{
	// Windows that sort will override
}

// Set window state
void CTableView::ResetColumns(const CColumnInfoArray& newCols)
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
		if (mTitles != NULL)
			mTitles->SetColWidth(info.column_width, col, col);
		mTable->SetColWidth(info.column_width, col, col);
	}
}

// Sync column state with current set
void CTableView::SyncColumns()
{
	// Sync column widths
	for(size_t i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = mTable->GetColWidth(i + 1);
}

// Reset text traits from prefs
void CTableView::ResetFont(const SFontInfo& finfo)
{
	// Change fonts
	mTable->ResetFont(finfo);
	
	// May need to manually adjust titles and position of table
	if (mTitles != NULL)
	{
		mTitles->ResetFont(finfo);
		mTitles->SyncTable(mTable, true);
	}
}
