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


// Header for CAdbkManagerTable class

#ifndef __CADBKMANAGERTABLE__MULBERRY__
#define __CADBKMANAGERTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"
#include "CListener.h"

#include "CWindowStatesFwd.h"
#include "CAdbkList.h"

#include "cdstring.h"

// Classes
class CAdbkProtocol;
class CAdbkManagerView;
class CAddressBook;
class CAddressBookManager;
class CAddressBookWindow;
class CKeyModifiers;

class CAdbkManagerTable : public CHierarchyTableDrag, public CListener
{

	friend class CAdbkManagerWindow;
	friend class CAdbkManagerTitleTable;

	DECLARE_DYNCREATE(CAdbkManagerTable)

public:
					CAdbkManagerTable();
	virtual			~CAdbkManagerTable();

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void		ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void		CollapseRow(UInt32 inWideOpenRow);
	virtual void		DeepCollapseRow(UInt32 inWideOpenRow);
	
	virtual void		ExpandRow(UInt32 inWideOpenRow);
	virtual void		DeepExpandRow(UInt32 inWideOpenRow);

	virtual void		ProcessExpansion(UInt32 inWideOpenRow, bool expand);

	virtual void	SetManager(CAddressBookManager* manager);		// Set the mail server

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

	// Common updaters
	afx_msg void	OnUpdateAllAdbk(CCmdUI* pCmdUI);			// Update command
	afx_msg void	OnUpdateAdbkSelection(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateLoggedInSelection(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateLoggedOutSelection(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateDisconnectedSelection(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateClearDisconnectedSelection(CCmdUI* pCmdUI);	// Update command

	// Command handlers
	void	DoSingleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview();
	void	DoPreview(CAddressBook* adbk);
	void	DoFullView();

	void	PreviewAddressBook(bool clear = false);							// Do preview of address book
	void	PreviewAddressBook(CAddressBook* adbk, bool clear = false);		// Do preview of address book

	void	OnNewDraft(void);								// Create new draft

	void	OnAddressBookProperties(void);					// Get properties of selected mailboxes

	void	OnNewAddressBook(void);							// Create new address book

	void	OnOpenAddressBook(void);						// Open the selected address book
	bool	OpenAddressBook(TableIndexT row);				// Open named address book

	void	OnRenameAddressBook(void);						// Rename the selected address books
	bool	RenameAddressBook(TableIndexT row);				// Rename the specified address book

	void	OnDeleteAddressBook(void);						// Delete the selected address books
	bool	DeleteAddressBook(TableIndexT row);				// Delete the specified address book

	void	OnSearchAddressBook(void);						// Search address books

	void	OnLoginAddressBook(void);						// Login to the selected servers
	bool	LoginAddressBook(TableIndexT row);				// Login to the specified server

	void	OnLogoutAddressBook(void);						// Logout from the selected servers
	bool	LogoutAddressBook(TableIndexT row);				// Logout from the specified server

	void	OnRefreshAddressBook(void);						// Refresh the selected servers
	bool	RefreshAddressBook(TableIndexT row);			// Refresh the specified server

	void	OnSynchroniseAddressBook(void);					// Synchronise the selected address books
	bool	SynchroniseAddressBook(TableIndexT row);		// Synchronise the specified address book

	void	OnClearDisconnectAddressBook(void);				// Clear disconnect selected address books
	bool	ClearDisconnectAddressBook(TableIndexT row);	// Clear disconnect the specified address book

	void	ResetTable(void);							// Reset the table from the mboxList
	void	ClearTable();								// Clear the table

	virtual void	AddNode(const CAdbkList::node_type* node,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
	virtual void	AddChildren(const CAdbkList::node_type* node,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
	virtual void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh);

			void	AddProtocol(CAdbkProtocol* proto);
			void	RemoveProtocol(CAdbkProtocol* proto);
			void	ClearProtocol(CAdbkProtocol* proto);
			void	RefreshProtocol(CAdbkProtocol* proto);
			void	LogoffProtocol(CAdbkProtocol* proto);

	virtual void	DoSelectionChanged(void);
	virtual void	RefreshSelection(void);								// Update button & captions as well
	virtual void	RefreshRow(TableIndexT row);						// Update button & captions as well

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);		// Clicked item
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);	// Double-clicked item

	virtual bool		IsDropCell(COleDataObject* pDataObject, const STableCell& cell);	// Draw drag row frame
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);					// Drop data into cell
private:
	CAddressBookManager*	mManager;				// Manager controlling window
	CAdbkManagerView*		mTableView;				// Owner view
	bool					mListChanging;			// In the process of changing the list
	CColumnInfoArray*		mColumnInfo;
	vector<void*>			mData;					// data
	bool					mHasOthers;				// Indicates 'Other' item exists

	bool TestSelectionAdbk(TableIndexT row);						// Test for selected adbk
	bool TestSelectionAdbkDisconnected(TableIndexT row);			// Test for selected adbk
	bool TestSelectionAdbkClearDisconnected(TableIndexT row);		// Test for selected adbk

	bool					IsCellAdbk(TableIndexT row);				// Check for adbk
	CAdbkList::node_type*	GetCellNode(TableIndexT row);				// Get the selected node
	CAddressBook*			GetCellAdbk(TableIndexT row);				// Get the selected adbk
	CAdbkProtocol*			GetCellAdbkProtocol(TableIndexT row);		// Get the selected adbk protocol

	void*	GetCellData(TableIndexT woRow);							// Get the selected adbk

	virtual int	GetPlotIcon(const CAdbkList::node_type* node,
									CAdbkProtocol* proto);		// Get appropriate icon id
	virtual void	SetTextStyle(CDC* pDC, const CAdbkList::node_type* node,
							CAdbkProtocol* proto, bool& strike);	// Get appropriate text style
	virtual bool 	UsesBackgroundColor(const CAdbkList::node_type* node) const;
	virtual COLORREF GetBackgroundColor(const CAdbkList::node_type* node) const;

	virtual bool	AddSelectionToList(TableIndexT row,
										CFlatAdbkList* list);		// Add selected address books to list

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
