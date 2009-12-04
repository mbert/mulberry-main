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


// Header for CServerTable class

#ifndef __CSERVERTABLE__MULBERRY__
#define __CSERVERTABLE__MULBERRY__

#include "CServerBrowse.h"

// Classes
class CKeyModifiers;
class CServerView;

class CServerTable : public CServerBrowse
{
	friend class CServerView;
	friend class CServerTitleTable;

	DECLARE_DYNCREATE(CServerTable)

public:
					CServerTable();
	virtual			~CServerTable();

	virtual void	ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void	SetServer(CMboxProtocol* server);		// Set the mail server
	virtual void	SetManager(void);						// Make this table the manager
	virtual void	SetTitle();								// Set the title of the window

	void	DoPreview(CMbox* mbox,							// Preview this mailbox
						bool use_dynamic = true,
						bool is_search = false);
	bool	IsPreviewed(const CMbox* mbox) const;			// Is this mailbox being previewed now

protected:
	virtual void	UpdateState();							// Update status items like toolbar

	// Handle single click
	void	DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview(bool clear = false);							// Do preview
	void	DoFullView();											// Do full view

	// General
	void	DoNewLetter(bool option_key);					// Create a new draft

	// Server related
	void	DoServerProperties(void);						// Get properties of server
	void	DoOpenServers(void);							// Open selected server windows
	bool	OpenTheServer(TableIndexT row);					// Open specific server window
	void	DoLogonServers(void);							// Logon selected server windows
	void	DoLogoffServers(void);							// Logoff selected server windows

	// Favourite related
	void	DoNewFavourite(void);							// Create new favourite
	void	DoRenameFavourite(void);						// Rename favourites
	void	DoRemoveFavourite(void);						// Rename favourites
	void	DoRemoveFavouriteItems(void);					// Remove favourite items

	void	DoNewWildcardItem(void);							// Create new favourite wildcard item
	bool	NewWildcardItem(TableIndexT row, cdstring* name);	// Add new wildcard item
	void	DoRenameWildcardItem(void);							// Rename favourite wildcard item
	bool	RenameWildcardItem(TableIndexT row);				// Rename wildcard item

	// Hierarchy related
	void	DoNewHierarchy(void);							// Create new hierarchy
	void	DoRenameHierarchy(void);						// Rename hierarchy
	void	DoDeleteHierarchy(void);						// Delete hierarchy
	void	DoResetHierarchy(void);							// Reset hierarchy

	void	DoFlatHierarchy(void);							// Change to flat view of hierarchy
	void	SetFlatHierarchy(bool hier);					// Change to flat view of hierarchy

	// Mailbox related
	void	DoMailboxProperties(void);						// Get properties of selected mailboxes

	void	SelectNextRecent(bool reverse);					// Select next mailbox with recent messages
	void	DoCheckMail(void);								// Check mail on selected mailboxes
	bool	CheckMailbox(CMbox* mbox, long* num_new, bool fast = false);		// Check mail on the specified mailbox

	bool	PickMailboxName(CMbox*& mbox);					// Pick mailbox name

	void	DoChooseMailbox(void);							// Choose & open a mailbox

	virtual void	PostCreateAction(CMbox* mbox);					// Process mailbox after creation

	void	DoOpenMailbox(void);							// Open the selected mailboxes
	bool	OpenTheMailbox(CMbox* mbox,
									bool subscribe);				// Open named mailbox

	bool	AddNamedSelectionToList(TableIndexT row,
										cdstrvect* list);			// Add names of selected items to list
	bool	SelectFromList(cdstrvect* list, CMboxList* add_list);	// Select named items in list

	void	DoPuntUnseenMailbox(void);						// Punt unseen in selected mailboxes
	void	PuntUnseenMailbox(CMbox* mbox);					// Punt unseen in specified mailbox

	void	DoDeleteMailbox(void);							// Delete the selected mailboxes
	void	DeleteMailbox(CMbox* mbox);						// Delete the specified mailbox

	void	DoRenameMailbox(void);							// Rename the selected mailboxes
	bool	RenameMailbox(CMbox* mbox);						// Rename the specified mailbox

	void	DoSynchroniseMailbox();							// Synchronise
	void	DoClearDisconnectMailbox();						// Clear disconnected cache

	void	DoSubscribeMailbox(void);						// Subscribe to the selected mailboxes
	bool	SubscribeMailbox(CMbox* mbox);					// Subscribe to the specified mailbox

	void	DoUnsubscribeMailbox(void);						// Unsubscribe to the selected mailboxes
	bool	UnsubscribeMailbox(CMbox* mbox);				// Unsubscribe to the specified mailbox

	void	OnSearchMailbox(void);							// Search the selected mailboxes
	void	OnSearchAgainMailbox(void);						// Search again the selected mailboxes

public:
	void	SetLogon(bool pushed);							// Set logon button
	void	GetLogonState(bool& enabled, bool& pushed);		// Get state of logon/off
	bool	LogonServer(void);								// Logon to the server
	void	LogonAction(CMboxProtocol* proto);				// Logon to the server
	void	LogoffAction(CMboxProtocol* proto);				// Logoff from the server

	// Common updaters

	afx_msg void	OnUpdateManager(CCmdUI* pCmdUI);				// Update command
	afx_msg void	OnUpdateLoggedIn(CCmdUI* pCmdUI);				// Update command
	afx_msg void	OnUpdateLoggedInSelection(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateLoggedInDSelection(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateProperties(CCmdUI* pCmdUI);				// Update command
	afx_msg void	OnUpdateOpenMailbox(CCmdUI* pCmdUI);			// Update command
	afx_msg void	OnUpdateMailboxCreate(CCmdUI* pCmdUI);			// Update command
	afx_msg void	OnUpdateMailboxSynchronise(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateMailboxClearDisconnect(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateFavouriteRemove(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateFavouriteRemoveItems(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateFavouriteWildcard(CCmdUI* pCmdUI);		// Update command
	afx_msg void	OnUpdateFavouriteWildcardItems(CCmdUI* pCmdUI);	// Update command
	afx_msg void	OnUpdateHierarchy(CCmdUI* pCmdUI);				// Update command
	afx_msg void	OnUpdateResetHierarchy(CCmdUI* pCmdUI);			// Update command
	afx_msg void	OnUpdateHierarchic(CCmdUI* pCmdUI);				// Update command
	afx_msg void	OnUpdateLoginBtn(CCmdUI* pCmdUI);				// Update command

	// Other commands
	afx_msg void	OnFileNewDraft(void);							// File:Draft

	afx_msg void	OnEditProperties(void);							// Edit:Properties command

	afx_msg void	OnMailboxCheckFavourites(void);					// Mailbox:Check Favourites command

	afx_msg void	OnServerLogon(void);							// Server logon button
	afx_msg void	OnServerView(UINT nID);							// Server view popup

	afx_msg void	OnCreateMailbox(void);							// Create Mailbox

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);		// Single-click
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);	// Double-clicked item

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)				// Reset header details from array
						{ mColumnInfo = &col_info; }
	virtual SColumnInfo	GetColumnInfo(TableIndexT col);
	virtual void	DoSelectionChanged(void);

	virtual BOOL	DoDrag(TableIndexT row);											// Prevent drag if improper selection
	virtual void	SetDragFlavors(TableIndexT row);
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

	virtual bool	ItemIsAcceptable(COleDataObject* pDataObject);						// Check its suitable
	virtual bool	IsDropCell(COleDataObject* pDataObject, const STableCell& cell);			// Draw drag row frame
	virtual bool	IsDropAtCell(COleDataObject* pDataObject, STableCell& cell);		// Test drop at cell
	virtual bool	ValidDragSelection(void) const;										// Check for valid drag selection
	virtual bool	CanDropExpand(COleDataObject* pDataObject, TableIndexT row);		// Can row expand for drop

	virtual DROPEFFECT	GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point); // Determine effect
	virtual bool		DropData(unsigned int theFlavor,
									char* drag_data,
									unsigned long data_size);					// Drop data into whole table
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);						// Drop data into cell
	virtual bool		DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);				// Drop data at cell
private:
	CServerView*		mTableView;					// Owning view

	CMbox*				mCopyToMbox;				// Mbox to copy to during drop
	CMbox*				mCopyFromMbox;				// Mbox to copy from during drop
	ulvector			mCopyMsgNums;				// Messages numbers to copy during drop

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
