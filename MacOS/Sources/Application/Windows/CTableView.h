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


// Header for CTableView class

#ifndef __CTABLEVIEW__MULBERRY__
#define __CTABLEVIEW__MULBERRY__

#include "CBaseView.h"

#include "CWindowStatesFwd.h"

// Constants

// Classes
class CTitleTableView;
class CUserAction;
class LTableView;

class CTableView : public CBaseView
{
	friend class CTitleTableView;

protected:
	CColumnInfoArray	mColumnInfo;
	CTitleTableView*	mTitles;
	LTableView*			mTable;
	int					mNewColumn;
	short				mNewSize;
	bool				mHierarchic;

public:
					CTableView();
					CTableView(LStream *inStream);
	virtual 		~CTableView();

	virtual void ListenTo_Message(long msg, void* param);

	CColumnInfoArray&	GetColumnInfo()
		{ return mColumnInfo; }

	LTableView*		GetBaseTable() const
		{ return mTable; }
	CTitleTableView*	GetBaseTitles() const
		{ return mTitles; }

	virtual bool	HasFocus() const;
	virtual void	Focus();

protected:
	virtual void	FinishCreateSelf(void);
	virtual void	SetViewInfo(int new_col_type, short new_size, bool hierarchic)
		{ mNewColumn = new_col_type; mNewSize = new_size; mHierarchic = hierarchic; }

public:
	virtual const CUserAction& GetPreviewAction() const = 0;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const = 0;		// Return user action data

	virtual void	ResetTable(void) = 0;					// Reset the table
	virtual void	ClearTable(void) = 0;					// Clear the table

	virtual void	InitColumns(void) = 0;					// Init columns and text
	virtual void	InsertColumn(short col_pos,				// Insert a column
									int col_type,
									short col_width);
	virtual void	InsertColumn(short col_pos)						// Insert a column
		{ InsertColumn(col_pos, mNewColumn, mNewSize); }
	virtual void	AppendColumn(int col_type,				// Append a column
									short col_width)
		{ InsertColumn(mColumnInfo.size() + 1, col_type, col_width); }
	virtual void	DeleteColumn(short col_pos);			// Delete a column
	virtual void	SetColumnType(short col_pos,
									int col_type);			// Set type of a column
	virtual void	SetColumnWidth(short col_pos,
									short col_width);		// Set width of column
	
	virtual int		GetSortBy(void);						// Get sort
	virtual void	SetSortBy(int sort)	;					// Force change of sort

	virtual int		GetShowBy(void);						// Get sort direction
	virtual void	SetShowBy(int sort);					// Force change of sort direction
	virtual void	ToggleShowBy();							// Force toggle of sort direction

	virtual void	ResetState(bool force = false) = 0;			// Reset window state
	virtual void	ResetColumns(const CColumnInfoArray& newCols);	// Reset column state
	virtual void	SaveState(void) {}							// Save current state in prefs
	virtual void	SaveDefaultState(void) = 0;					// Save current state as default

	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits);	// Reset text traits
};

#endif
