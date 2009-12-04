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

#include "LWindow.h"

#include "CPreferences.h"

#include "CWindowStatus.h"


// Constants
const	PaneIDT		paneid_TitleTable = 'TITL';
const	PaneIDT		paneid_ListTable = 'LIST';

// Classes
class CTitleTable;
class LTableView;

class CTableWindow : public LWindow,
							public CWindowStatus {

	friend class CTitleTable;

public:
	static Point		sMouseUp;

protected:
	CColumnInfoArray	mColumnInfo;
	CTitleTable*		mTitles;
	LTableView*			mTable;
	int					mNewColumn;
	short				mNewSize;
	bool				mHierarchic;

public:
					CTableWindow();
					CTableWindow(LStream *inStream);
	virtual 		~CTableWindow();

	CColumnInfoArray&	GetColumnInfo()
		{ return mColumnInfo; }

protected:
	virtual void	FinishCreateSelf(void);
	virtual void	SetWindowInfo(int new_col_type, short new_size, bool hierarchic)
		{ mNewColumn = new_col_type; mNewSize = new_size; mHierarchic = hierarchic; }

public:
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void	ClickInContent(const EventRecord &inMacEvent);

	virtual void	ResetTable(void) = 0;					// Reset the table

	virtual void	InitColumns(void) = 0;					// Init columns and text
	virtual void	InsertColumn(short col_pos);			// Insert a column
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

	virtual Boolean	CalcStandardBounds(Rect &outStdBounds) const;
	virtual void	ResetStandardSize(void);				// Take column width into account
	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits);	// Reset text traits
};

#endif
