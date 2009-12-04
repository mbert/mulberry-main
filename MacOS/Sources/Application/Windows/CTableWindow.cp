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
#include "CHierarchyTableDrag.h"
#include "CTitleTable.h"
#include "CTableDrag.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

Point CTableWindow::sMouseUp = {0, 0};

// Default constructor
CTableWindow::CTableWindow()
{
	mNewColumn = 0;
	mNewSize = 16;
	mHierarchic = false;
}

// Constructor from stream
CTableWindow::CTableWindow(LStream *inStream)
		: LWindow(inStream)
{
	mNewColumn = 0;
	mNewSize = 16;
	mHierarchic = false;
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

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CTableWindow::FinishCreateSelf(void)
{
	// Do inherited
	LWindow::FinishCreateSelf();

	// Titles
	mTitles = (CTitleTable*) FindPaneByID(paneid_TitleTable);

	// Info
	mTable = (LTableView*) FindPaneByID(paneid_ListTable);

	// Init columns
	InitColumns();

	// Get text traits resource
	ResetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);
}

// Respond to commands
Boolean CTableWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{

		case cmd_SetDefaultSize:
			SaveDefaultState();
			break;

		case cmd_ResetDefaultWindow:
			ResetState(true);
			break;

		default:
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CTableWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{

		case cmd_SetDefaultSize:
		case cmd_ResetDefaultWindow:
			outEnabled = true;
			break;

		default:
			LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
	}
}

void CTableWindow::ClickInContent(const EventRecord &inMacEvent)
{
									// Enabled Windows respond to clicks
	Boolean		respondToClick = HasAttribute(windAttr_Enabled);

									// Set up our extended event record
	SMouseDownEvent		theMouseDown;
	theMouseDown.wherePort	 = inMacEvent.where;
	GlobalToPortPoint(theMouseDown.wherePort);
	theMouseDown.whereLocal	 = theMouseDown.wherePort;
	theMouseDown.macEvent	 = inMacEvent;
	theMouseDown.delaySelect = false;

	if (!UDesktop::WindowIsSelected(this)) {
									// Window is not in front, we might
									//   need to select it
		Boolean	doSelect = true;
		if (HasAttribute(windAttr_DelaySelect))
		{
			// Reset mouse up
			sMouseUp.h = 0;
			sMouseUp.v = 0;

									// Delay selection until after handling
									//   the click (called click-through)
			theMouseDown.delaySelect = true;
			Click(theMouseDown);

									// After click-through, we select the
									//   Window if the mouse is still down
									//   or the mouse up occurred inside
									//   this Window.
			if (!::StillDown() && (sMouseUp.h || sMouseUp.v))
			{
									// Check location of mouse up event
				WindowPtr	upWindow;
				::MacFindWindow(sMouseUp, &upWindow);
				doSelect = (upWindow == mMacWindowP);
			}
		}

		if (doSelect) {				// Selecting a Window brings it to the
									//   front of its layer and activates it
			Select();
			respondToClick = HasAttribute(windAttr_GetSelectClick);
		}
	}

	if (respondToClick) {
		if (!theMouseDown.delaySelect) {
			Click(theMouseDown);
		} else {
			sLastPaneClicked = nil;
		}
	}
}

// Insert a column
void CTableWindow::InsertColumn(short col_pos)
{
	// Add required column to tables
	mTitles->InsertCols(1, col_pos-1, nil, 0, false);
	mTable->UnselectAllCells();
	mTable->InsertCols(1, col_pos-1, nil, 0, false);

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
		SysBeep(1);
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
	mTitles->RefreshCol(col_pos);
	if (mHierarchic)
		((CHierarchyTableDrag*) mTable)->RefreshCol(col_pos);
	else
		((CTableDrag*) mTable)->RefreshCol(col_pos);
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
	SInt32 old_cols = mColumnInfo.size();
	SInt32 new_cols = newCols.size();
	if (old_cols > new_cols)
	{
		for(short i = old_cols; i > new_cols; i--)
			DeleteColumn(i);
	}
	else if (old_cols < new_cols)
	{
		for(short i = old_cols+1; i <= new_cols; i++)
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

} // CTableWindow::ResetColumns

Boolean CTableWindow::CalcStandardBounds(Rect &outStdBounds) const
{
	// Redo size calculation
	const_cast<CTableWindow*>(this)->ResetStandardSize();
	
	return LWindow::CalcStandardBounds(outStdBounds);
}

// Take column width into account
void CTableWindow::ResetStandardSize(void)
{
	TableIndexT	row_num;
	TableIndexT	col_num;
	SDimension16	std_size = {0, 0};

	mTable->GetTableSize(row_num, col_num);

	// Use the table's image width and height and vertical position
	SDimension32 tableSize;
	SPoint32 tablePos;
	mTable->GetImageSize(tableSize);
	mTable->GetFrameLocation(tablePos);
	std_size.width = tableSize.width;
	std_size.height = tableSize.height + tablePos.v;

	// Adjust for vertical scrollbar
	std_size.width += 14;
	if (std_size.width < mMinMaxSize.left)
		std_size.width = mMinMaxSize.left;

	// Adjust for horizontal scroll bar
	std_size.height += 15;
	if (std_size.height < mMinMaxSize.top)
		std_size.height = mMinMaxSize.top;

	SetStandardSize(std_size);

} // CTableWindow::ResetStandardSize

// Reset text traits from prefs
void CTableWindow::ResetTextTraits(const TextTraitsRecord& list_traits)
{
	// Get old header row height
	short old_height = mTitles->GetRowHeight(1);

	// Change fonts
	mTitles->SetTextTraits(list_traits);
	if (mHierarchic)
		((CHierarchyTableDrag*) mTable)->SetTextTraits(list_traits);
	else
		((CTableDrag*) mTable)->SetTextTraits(list_traits);

	// Get new row height
	short diff_height = mTitles->GetRowHeight(1) - old_height;

	// Change panes if row height changed
	if (diff_height)
	{
		// Offset and resize table
		mTable->GetSuperView()->MoveBy(0, diff_height, false);
		mTable->GetSuperView()->ResizeFrameBy(0, -diff_height, true);

		// Do header and button
		mTitles->ResizeFrameBy(0, diff_height, true);
	}
}
