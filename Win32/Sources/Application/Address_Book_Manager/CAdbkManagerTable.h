/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
#include "CAddressBook.h"

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
	virtual void		ExpandRestore(UInt32 inWideOpenRow);
	virtual void		ExpandAction(UInt32 inWideOpenRow, bool deep);

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
	afx_msg void	OnUpdateHierarachy(CCmdUI* pCmdUI);					// Update command

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

	void	OnLogin(void);									// Login to the selected servers

	void	OnNewHierarchy(void);							// Login to the selected servers
	void	OnRenameHierarchy(void);						// Login to the selected servers
	void	OnDeleteHierarchy(void);						// Login to the selected servers

	void	OnLogoutAddressBook(void);						// Logout from the selected servers
	bool	LogoutAddressBook(TableIndexT row);				// Logout from the specified server

	void	OnRefreshAddressBook(void);						// Refresh the selected servers

	void	OnSynchroniseAddressBook(void);					// Synchronise the selected address books
	bool	SynchroniseAddressBook(TableIndexT row);		// Synchronise the specified address book

	void	OnClearDisconnectAddressBook(void);				// Clear disconnect selected address books
	bool	ClearDisconnectAddressBook(TableIndexT row);	// Clear disconnect the specified address book

	void	ResetTable(void);							// Reset the table from the mboxList
	void	ClearTable();								// Clear the table

	virtual void	AddNode(CAddressBook* adbk,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
	virtual void	AddChildren(const CAddressBook* adbk,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
	virtual void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh);

			void	AddProtocol(CAdbkProtocol* proto);
			void	InsertProtocol(CAdbkProtocol* proto);
			void	RemoveProtocol(CAdbkProtocol* proto);
			void	ClearProtocol(CAdbkProtocol* proto);
			void	RefreshProtocol(CAdbkProtocol* proto);
			void	SwitchProtocol(CAdbkProtocol* proto);

			void	InsertNode(CAddressBook* node);					// Insert a node to the list
			void	DeleteNode(CAddressBook* node);					// Delete a node from the list
			void	RefreshNode(CAddressBook* node);				// Refresh a node from the list

			void	ClearSubList(CAddressBook* node);
			void	RefreshSubList(CAddressBook* node);

	virtual void	DoSelectionChanged(void);
	virtual void	RefreshSelection(void);								// Update button & captions as well
	virtual void	RefreshRow(TableIndexT row);						// Update button & captions as well

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);		// Clicked item
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);	// Double-clicked item

#if NOTYET
	virtual bool		IsDropCell(COleDataObject* pDataObject, const STableCell& cell);	// Draw drag row frame
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);					// Drop data into cell
#endif
private:
	CAddressBookManager*	mManager;				// Manager controlling window
	CAdbkManagerView*		mTableView;				// Owner view
	bool					mListChanging;			// In the process of changing the list
	CColumnInfoArray*		mColumnInfo;
	std::vector<CAddressBook*>	mData;					// data

	bool TestSelectionServer(TableIndexT row);						// Test for selected adbk
	bool TestSelectionAdbk(TableIndexT row);						// Test for selected adbk
	bool TestSelectionAdbkDisconnected(TableIndexT row);			// Test for selected adbk
	bool TestSelectionAdbkClearDisconnected(TableIndexT row);		// Test for selected adbk
	bool TestSelectionHierarchy(TableIndexT row);					// Test for selected adbk

	virtual int	GetPlotIcon(const CAddressBook* adbk,
									CAdbkProtocol* proto);		// Get appropriate icon id
	virtual void	SetTextStyle(CDC* pDC, const CAddressBook* adbk,
							CAdbkProtocol* proto, bool& strike);	// Get appropriate text style
	virtual bool 	UsesBackgroundColor(const STableCell &inCell) const;
	virtual COLORREF GetBackgroundColor(const STableCell &inCell) const;


	CAddressBook*	GetCellNode(TableIndexT row, bool worow = false) const;			// Get the selected node
	CAdbkProtocol*	GetCellAdbkProtocol(TableIndexT row) const;					// Get the selected adbk protocol

	virtual bool	AddSelectionToList(TableIndexT row,
										CAddressBookList* list);		// Add selected address books to list

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
