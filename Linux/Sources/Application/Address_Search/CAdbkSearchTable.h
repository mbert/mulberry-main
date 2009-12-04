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


// Header for CAdbkSearchTable class

#ifndef __CADBKSEARCHTABLE__MULBERRY__
#define __CADBKSEARCHTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "cdstring.h"
#include "CWindowStatesFwd.h"


// Classes
class CAdbkSearchWindow;

class CAdbkSearchTable : public CHierarchyTableDrag
{
	friend class CAdbkManagerWindow;
	friend class CAdbkSearchWindow;

public:
	CAdbkSearchTable(JXScrollbarSet* scrollbarSet, 
						 JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h);
	virtual	~CAdbkSearchTable();

private:
			void	InitAdbkSearchTable();

protected:
	virtual void		OnCreate();			// Get details of sub-panes

	virtual SColumnInfo GetColumnInfo(TableIndexT col);
	virtual void SetColumnInfo(CColumnInfoArray& col_info)	  // Reset header details from array
		{ mColumnInfo = &col_info; }

public:
	virtual void ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	virtual void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);

	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,
			 					const JRect& inLocalRect);				// Draw the items

	// Command handlers
	void	OnNewLetter(void);							// Create new draft
	void	CreateNewLetter(bool option_key);			// Create new draft now
	void	OnEditCopy();
	bool	AddAddressText(TableIndexT row, cdstring* txt);			// Add address as text
	void	OnSearch(void);								// Do search
	void	OnClear(void);								// Clear results

	void	DoEditEntry(void);							// Edit selected entries
	bool	EditEntry(TableIndexT row);							// Edit specified address

	void	OnDeleteAddress(void);						// Delete the selected results
	bool	DeleteAddress(TableIndexT row);						// Delete the specified result


	void	ResetTable(void);							// Reset the table from the mboxList
	void	AppendItem(const CAddressSearchResult* item);	// Add an item to end of list
	void	AddList(const CAddressList* list, TableIndexT& row);	// Add a node to the list

private:
	CAdbkSearchWindow*					mWindow;				// Owner window
	std::vector<void*>					mData;					// data
	CColumnInfoArray*					mColumnInfo;
	TableIndexT							mLastParent;			// last parent row

	bool TestSelectionAddr(TableIndexT row);						// Test for selected addr

	virtual void*	GetCellData(TableIndexT row);					// Get the selected adbk
	virtual void	AddSelectionToList(CAddressList* list);			// Add selected address books to list
	virtual bool	CopyEntryToList(TableIndexT row,
										CAddressList* list);		// Copy cell to list

protected:
// Drag methods
	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
};

#endif
