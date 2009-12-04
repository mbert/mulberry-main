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


// Header for CTableWindow class

#ifndef __CTABLEWINDOW__MULBERRY__
#define __CTABLEWINDOW__MULBERRY__

#include "CWindow.h"

#include "CWindowStatesFwd.h"

// Classes
class CTable;
class CTitleTable;

class CTableWindow : public CWindow
{
	friend class CTitleTable;
	friend class CTable;

public:
					CTableWindow(JXDirector* owner);
					virtual ~CTableWindow();

protected:

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual CCommander* GetTarget();

public:
	CColumnInfoArray&	GetColumnInfo()
		{ return mColumnInfo; }

	virtual void	ResetTable(void) = 0;					// Reset the table

	virtual void	InitColumns(void) = 0;					// Init columns and text
	virtual void	InsertColumn(short col_pos);			// Insert a column
	virtual void	DeleteColumn(short col_pos);			// Delete a column
	virtual void	SetColumnType(short col_pos, int col_type);			// Set type of a column
	virtual void	SetColumnWidth(short col_pos, short col_width);		// Set width of column

	virtual int 	GetSortBy();
	virtual void	SetSortBy(int sort);					// Force change of sort

	virtual int		GetShowBy();							// Get sort direction
	virtual void	SetShowBy(int sort);					// Force change of sort direction
	virtual void	ToggleShowBy();							// Force toggle of sort direction

	virtual void	ResetColumns(const CColumnInfoArray& newCols);	// Reset column state
	virtual void	ResetState(bool force = false) = 0;			// Reset window state
	virtual void	SaveState(void) {}							// Save current state in prefs
	virtual void	SaveDefaultState(void) = 0;					// Save current state as default

	virtual void	ResetFont(const SFontInfo& list_traits);	// Reset text traits

protected:
	CColumnInfoArray	mColumnInfo;
	CTitleTable*		mTitles;
	CTable*				mTable;
	int					mNewColumn;
	short				mNewSize;

	virtual void	PostCreate(CTable* tbl, CTitleTable* titles);
	virtual void	SetWindowInfo(int new_col_type, short new_size)
		{ mNewColumn = new_col_type; mNewSize = new_size; }

};

#endif
