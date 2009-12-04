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

#include "CWindowStatesFwd.h"
#include "CAdbkList.h"
#include "CAddressBookManager.h"

#include "cdstring.h"

// Classes
class CAdbkProtocol;
class CAddressBook;
class CAddressBookManager;
class CAddressBookWindow;

class CAdbkSearchTable : public CHierarchyTableDrag
{

	friend class CAdbkSearchWindow;
	friend class CAdbkSearchTitleTable;

	DECLARE_DYNCREATE(CAdbkSearchTable)

	enum
	{
		eAdbkSearchResult = 0,
		eAdbkSearchItem
	};

public:
					CAdbkSearchTable();
	virtual			~CAdbkSearchTable();

	virtual void		ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

	// Common updaters

	// Command handlers
	afx_msg void	OnEditCopy(void);

	void	OnNewLetter(void);							// Create new draft
	void	CreateNewLetter(bool option_key);			// Create new draft now
	void	OnSearch(void);								// Do search
	void	OnClear(void);								// Clear results

	void	DoEditEntry(void);							// Edit selected entries
	bool	EditEntry(TableIndexT row);							// Edit specified address

	void	OnDeleteAddress(void);						// Delete the selected results
	bool	DeleteAddress(TableIndexT row);						// Delete the specified result


	void	ResetTable(void);							// Reset the table from the mboxList
	void	AppendItem(const CAddressSearchResult* item);	// Add an item to end of list
	void	AddList(const CAddressList* list, TableIndexT& row);	// Add a node to the list

	// Key/Mouse related

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);						// Double-clicked item

	// Drag & Drop
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

private:
	CAdbkSearchWindow* 		mWindow;
	vector<void*>			mData;					// data
	int						mLastParent;			// last parent row
	CColumnInfoArray*		mColumnInfo;

	virtual bool TestSelectionAddr(TableIndexT row);		// Test for selected adbk

	virtual void*	GetCellData(TableIndexT row);			// Get the data

	virtual void	AddSelectionToList(CAddressList* list);			// Add selected addresses to list
	virtual bool	CopyEntryToList(TableIndexT row,
										CAddressList* list);		// Copy cell to list

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
