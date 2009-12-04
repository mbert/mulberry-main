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
class JXTextMenu;

class CServerTable : public CServerBrowse
{
	friend class CMulberryApp;
	friend class CServerView;
	friend class CServerTitleTable;

public:
	CServerTable(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual ~CServerTable();

	virtual void OnCreate();
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void SetServer(CMboxProtocol* server);// Set the mail server
	virtual void SetManager(void);      // Make this table the manager
	virtual void SetTitle();	      // Set the title of the window

	void	DoPreview(CMbox* mbox,							// Preview this mailbox
						bool use_dynamic = true,
						bool is_search = false);
	bool	IsPreviewed(const CMbox* mbox) const;			// Is this mailbox being previewed now

	enum {
		kServerContextCreate = 1, kServerContextOpen, kServerContextRename,
		kServerContextDisplayHierarchy, kServerContextCabinets, 
		kServerContextSubscribe, kServerContextUnsubscribe, 
		kServerContextCheckMail, kServerContextCheckFavourites, 
		kServerContextMarkContentsSeen, kServerContextDelete, 
		kServerContextSearch, kSearchPopupProperties
	};

	enum {kServerContextDispHNew = 1, kServerContextDispHEdit, kServerContextDispHRemove, 
				kServerContextDispHRefresh};
	enum {kServerContextCabNew = 1, kServerContextCabEdit, kServerContextCabRemove, kServerContextCabAddWild,
				kServerContextCabEditWild, kServerContextCabRemoveItem};


protected:
	virtual void	UpdateState();							// Update status items like toolbar

	// Handle single click
	void	DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview(bool clear = false);							// Do preview
	void	DoFullView();											// Do full view

	// Common updaters
	void OnUpdateManager(CCmdUI* cmdui);
	void OnUpdateLoggedIn(CCmdUI* cmdui);
	void OnUpdateLoggedInSelection(CCmdUI* cmdui);
	void OnUpdateLoggedInDSelection(CCmdUI* cmdui);
	void OnUpdateProperties(CCmdUI* cmdui);
	void OnUpdateOpenMailbox(CCmdUI* cmdui);
	void OnUpdateMailboxCreate(CCmdUI* cmdui);
	void OnUpdateMailboxSynchronise(CCmdUI* cmdui);
	void OnUpdateMailboxClearDisconnect(CCmdUI* cmdui);
	void OnUpdateFavouriteRemove(CCmdUI* cmdui);
	void OnUpdateFavouriteRemoveItems(CCmdUI* cmdui);
	void OnUpdateFavouriteWildcard(CCmdUI* cmdui);
	void OnUpdateFavouriteWildcardItems(CCmdUI* cmdui);
	void OnUpdateHierarchy(CCmdUI* cmdui);
	void OnUpdateResetHierarchy(CCmdUI* cmdui);
	void OnUpdateHierarchic(CCmdUI* cmdui);
	void OnUpdateLoginBtn(CCmdUI* cmdui);

	// General
	void DoNewLetter(bool option_key); // Create a new draft

	// Server related
	void DoServerProperties(void); // Get properties of server
	void DoOpenServers(void); // Open selected server windows
	bool OpenTheServer(TableIndexT row); // Open specific server window
	void DoLogonServers(void); // Logon selected server windows
	void DoLogoffServers(void); // Logoff selected server windows

	// Favourite related
	void DoNewFavourite(void); // Create new favourite
	void DoRenameFavourite(void); // Rename favourites
	void DoRemoveFavourite(void); // Rename favourites
	void DoRemoveFavouriteItems(void); // Remove favourite items

	void DoNewWildcardItem(void); // Create new favourite wildcard item
	bool NewWildcardItem(TableIndexT row, cdstring* name);	// Add new wildcard item
	void DoRenameWildcardItem(void); // Rename favourite wildcard item
	bool RenameWildcardItem(TableIndexT row); // Rename wildcard item

	// Hierarchy related
	void DoNewHierarchy(void);		// Create new hierarchy
	void DoRenameHierarchy(void);	// Rename hierarchy
	void DoDeleteHierarchy(void);	// Delete hierarchy
	void DoResetHierarchy(void);	// Reset hierarchy

	void DoFlatHierarchy(void); // Change to flat view of hierarchy
	void SetFlatHierarchy(bool hier); // Change to flat view of hierarchy

	// Mailbox related
	void DoMailboxProperties(void); // Get properties of selected mailboxes

	void SelectNextRecent(bool reverse); // Select next mailbox with recent messages


	void DoCheckMail(void); // Check mail on selected mailboxes

	bool CheckMailbox(CMbox* mbox, long* num_new, bool fast = false); // Check mail on the specified mailbox

	bool PickMailboxName(CMbox*& mbox); // Pick mailbox name

	void DoChooseMailbox(void); // Choose & open a mailbox

	virtual void PostCreateAction(CMbox* mbox); // Process mailbox after creation

	void DoOpenMailbox(void); // Open the selected mailboxes
	bool OpenTheMailbox(CMbox* mbox, bool subscribe); // Open named mailbox

	bool AddNamedSelectionToList(TableIndexT row, cdstrvect* list); // Add names of selected items to list
	bool SelectFromList(cdstrvect* list, CMboxList* add_list); // Select named items in list

	void DoPuntUnseenMailbox(void); // Punt unseen in selected mailboxes
	void PuntUnseenMailbox(CMbox* mbox); // Punt unseen in specified mailbox

	void DoDeleteMailbox(void); // Delete the selected mailboxes
	void DeleteMailbox(CMbox* mbox); // Delete the specified mailbox

	void DoRenameMailbox(void); // Rename the selected mailboxes
	bool RenameMailbox(CMbox* mbox); // Rename the specified mailbox

	void DoSynchroniseMailbox(); // Synchronise
	void DoClearDisconnectMailbox(); // Clear disconnected cache

	void DoSubscribeMailbox(void); // Subscribe to the selected mailboxes
	bool SubscribeMailbox(CMbox* mbox); // Subscribe to the specified mailbox

	void DoUnsubscribeMailbox(void);      // Unsubscribe to the selected mailboxes
	bool UnsubscribeMailbox(CMbox* mbox);    // Unsubscribe to the specified mailbox

	void OnSearchMailbox(void);       // Search the selected mailboxes
	void OnSearchAgainMailbox(void);      // Search again the selected mailboxes

public:
	void SetLogon(bool pushed);       // Set logon button
	void GetLogonState(bool& enabled, bool& pushed);		// Get state of logon/off
	bool LogonServer(void);      	// Logon to the server
	void LogonAction(CMboxProtocol* proto);    // Logon to the server
	void LogoffAction(CMboxProtocol* proto);    // Logoff from the server

	// Other commands
	void OnFileNewDraft(void);
	void OnEditProperties(void);       // Edit:Properties command

	void OnServerLogon(void);       // Server logon button
	void OnServerView(JIndex index);       // Server view popup
	void OnCreateMailbox(void);       // Create Mailbox

protected:
	virtual SColumnInfo GetColumnInfo(TableIndexT col);
	
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers); // Handle key press
	virtual void LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);							// Single-click
	virtual void LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers); // Double-clicked item
	virtual void DoSelectionChanged(void);


	virtual bool RenderSelectionData(CMulSelectionData* selection, Atom type);

	virtual bool GetSelectionAtom(CFlavorsList& atoms);
	virtual bool DoDrag(const JPoint& pt,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers );

	// Prevent drag if improper selection
	virtual bool	IsDropCell(JArray<Atom>& typeList, const STableCell& cell); // Can cell be dropped into
	virtual bool	IsDropAtCell(JArray<Atom>& typeList, STableCell& cell);// Can cell be dropped at (modify if not)
	virtual bool	ItemIsAcceptable(const JArray<Atom>& typeList); // Check its suitable

	virtual bool ValidDragSelection(void) const;        // Check for valid drag selection
	virtual bool CanDropExpand(const JArray<Atom>& typeList, TableIndexT row);  // Can row expand for drop

	virtual Atom GetDNDAction(const JXContainer* target, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers);

	virtual bool  DropData(Atom theFlavor,
							 unsigned char* drag_data,
							 unsigned long data_size);     // Drop data into whole table
	virtual bool  DropDataIntoCell(Atom theFlavor,
									 unsigned char* drag_data,
									 unsigned long data_size,
									 const STableCell& cell);        // Drop data into cell
	virtual bool  DropDataAtCell(Atom theFlavor,
								 unsigned char* drag_data,
								 unsigned long data_size,
								 const STableCell& before_cell);     // Drop data at cell
private:
	CServerView*	mTableView;					// Owning view

	CMbox* 		mCopyToMbox;    		// Mbox to copy to during drop
	CMbox*		mCopyFromMbox;    		// Mbox to copy from during drop
	ulvector	mCopyMsgNums;    		// Messages numbers to copy during drop
};

#endif
