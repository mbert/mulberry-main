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

#include "CFontCache.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CTitleTable.h"
#include "CTable.h"

#include "StValueChanger.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

BEGIN_MESSAGE_MAP(CTableWindow, CView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableWindow::CTableWindow()
{
	mNewColumn = 0;
	mNewSize = 16;
	mWndTitles = NULL;
	mWndTable = NULL;
}

// Default destructor
CTableWindow::~CTableWindow()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CTableWindow::PostCreate(CTable* tbl, CTitleTable* titles)
{
	mWndTable = tbl;
	mWndTitles = titles;

	// Init columns
	InitColumns();

	// Get text traits resource
	ResetFont(CFontCache::GetListFont());
}

void CTableWindow::OnDestroy(void)
{
	// Set status
	SetClosing();

	// Save state
	SaveState();

	// Do default action now
	CView::OnDestroy();

	// Set status
	SetClosed();
}

// Get child windows
void CTableWindow::OnInitialUpdate(void)
{
	// Attach sub-views
	if (mWndTable)
		mWndTable->SetFocus();
}

BOOL CTableWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
   // Extend the framework's command route from the view to
   // the application-specific CMyShape that is currently selected
   // in the view. m_pActiveShape is NULL if no shape object
   // is currently selected in the view.
   if (mWndTable && mWndTable->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
      return true;

   // If the object(s) in the extended command route don't handle
   // the command, then let the base class OnCmdMsg handle it.
   return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Tell window to focus on this one
void CTableWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to table
	if (mWndTable)
		mWndTable->SetFocus();
}

// Insert a column
void CTableWindow::InsertColumn(short col_pos)
{
	// Add required column to tables
	mWndTitles->InsertCols(1, col_pos-1, nil, 0, false);
	mWndTable->UnselectAllCells();
	mWndTable->InsertCols(1, col_pos-1, nil, 0, false);

	// Add to column info
	SColumnInfo newInfo;

	// Replace with new info
	newInfo.column_type = mNewColumn;
	newInfo.column_width = mNewSize;
	mColumnInfo.insert(mColumnInfo.begin() + (col_pos - 1), newInfo);

	// Refresh panes
	SetColumnWidth(col_pos, mNewSize);

	// Refresh panes
	mWndTitles->RedrawWindow();
	mWndTable->RedrawWindow();
}

// Delete a column
void CTableWindow::DeleteColumn(short col_pos)
{
	if (mColumnInfo.size() > 1)
	{
		// Remove required column
		mWndTitles->RemoveCols(1, col_pos, false);
		mWndTable->UnselectAllCells();
		mWndTable->RemoveCols(1, col_pos, false);

		// Remove from column info
		mColumnInfo.erase(mColumnInfo.begin() + (col_pos - 1));

		// Refresh panes
		mWndTitles->RedrawWindow();
		mWndTable->RedrawWindow();
	}
	else
		// Must have at least one column
		::MessageBeep(-1);
}

// Set type of a column
void CTableWindow::SetColumnType(short col_pos,
									int col_type)
{
	// Get current info
	SColumnInfo& newInfo = mColumnInfo[col_pos - 1];

	// Replace with new info
	newInfo.column_type = col_type;
	
	// Refresh panes
	mWndTitles->RefreshCol(col_pos);
	mWndTable->RefreshCol(col_pos);
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
	mWndTitles->SetColWidth(col_width, col_pos, col_pos);
	mWndTable->SetColWidth(col_width, col_pos, col_pos);
	
	// Refresh panes
	mWndTitles->RedrawWindow();
	mWndTable->RedrawWindow();
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
	long old_cols = mColumnInfo.size();
	long new_cols = newCols.size();
	if (old_cols > new_cols)
	{
		for(short i = old_cols; i > new_cols; i--)
			DeleteColumn(i);
	}
	else if (old_cols < new_cols)
	{
		for(short i = old_cols + 1; i <= new_cols; i++)
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
		mWndTitles->SetColWidth(info.column_width, col, col);
		mWndTable->SetColWidth(info.column_width, col, col);
	}

} // CTableWindow::ResetColumns

// Reset text traits from prefs
void CTableWindow::ResetFont(CFont* font)
{
	// Change font in table and determine change in row height
	mWndTable->ResetFont(font);

	// May need to manually adjust titles and position of table
	UInt16 old_height = mWndTitles->GetRowHeight(1);
	mWndTitles->ResetFont(font);
	SInt16 delta = mWndTitles->GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(mWndTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(mWndTable, 0, -delta, false);
		::MoveWindowBy(mWndTable, 0, delta, false);
		
		mWndTable->RedrawWindow();
		mWndTitles->RedrawWindow();
	}
}

// Reset to default state
void CTableWindow::OnSaveDefaultState(void)
{
	SaveDefaultState();
}

// Reset to default state
void CTableWindow::OnResetDefaultState(void)
{
	ResetState(true);
}

