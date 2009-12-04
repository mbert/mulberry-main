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

// Consts

// Panes

// Resources

// Messages

// Classes
class CKeyModifiers;
class CServerView;

class CServerTable : public CServerBrowse
{

	friend class CServerView;

	CServerView*		mTableView;					// Owning view

public:
	enum { class_ID = 'SeLi' };

					CServerTable();
					CServerTable(LStream *inStream);
	virtual 		~CServerTable();

private:
			void	InitServerTable(void);

protected:
	virtual void		FinishCreateSelf(void);			// Get details of sub-panes
public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);	// Respond to clicks in the icon buttons

protected:

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell

	virtual SColumnInfo	GetColumnInfo(TableIndexT col);
	virtual void	DoSelectionChanged(void);

public:
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
	void	DoRemoveFavourite(void);						// Remove favourites
	void	DoRemoveFavouriteItems(void);					// Remove favourite items

	void	DoNewWildcardItem(void);						// Create new favourite wildcard item
	bool	NewWildcardItem(TableIndexT row,
										cdstring* name);	// Add new wildcard item
	void	DoRenameWildcardItem(void);						// Rename favourite wildcard item
	bool	RenameWildcardItem(TableIndexT row);			// Rename wildcard item

	// Hierarchy related
	void	DoNewHierarchy(void);							// Create new hierarchy
	void	DoRenameHierarchy(void);						// Rename hierarchy
	void	DoDeleteHierarchy(void);						// Delete hierarchy
	void	DoResetHierarchy(void);							// Reset hierarchy

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

	void	OnSearchMailbox();							// Do mailbox search
	void	OnSearchAgainMailbox();						// Do mailbox search again

public:
	void	SetLogon(bool pushed);							// Set logon button
	void	GetLogonState(bool& enabled, bool& pushed);		// Get state of logon/off
	bool	LogonServer(void);								// Logon to the server
	void	LogonAction(CMboxProtocol* proto);				// Logon to the server
	void	LogoffAction(CMboxProtocol* proto);				// Logoff from the server

	// Handle changes to table
	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync
// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	virtual bool	ValidDragSelection(void) const;					// Check for valid drag selection
	virtual bool	CanDropExpand(DragReference inDragRef,
									TableIndexT woRow);				// Can row expand for drop
	virtual bool	IsCopyCursor(DragReference inDragRef);			// Use copy cursor?

	virtual bool	IsDropCell(DragReference inDragRef, STableCell row);// Can cell be dropped into
	virtual bool	IsDropAtCell(DragReference inDragRef, STableCell& aCell);	// Test drop at cell

	virtual void	DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size	);				// Drop data into table
	virtual void	DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);		// Drop data into cell
	virtual void	DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);	// Drop data at cell

};

#endif
