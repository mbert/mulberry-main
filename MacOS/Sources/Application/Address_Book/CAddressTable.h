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


// Header for CAddressTable class

#ifndef __CADDRESSTABLE__MULBERRY__
#define __CADDRESSTABLE__MULBERRY__

#include "CTableDrag.h"

// Consts
const	ResIDT		Txtr_DefaultAddrList = 1000;

// Panes

// Resources

// Messages

// Classes

class CAdbkAddress;
class CAddressBook;
class CAddressBookView;
class CAddressList;
class CAddressTableNewAction;
class CAddressTableEditAction;
class CKeyModifiers;
class LArray;
class LHandleStream;

class CAddressTable : public CTableDrag
{
	friend class CAddressBookView;

private:
	CAddressBookView*	mTableView;					// View that owns this
	CAddressBook*		mAdbk;						// List of sorted entries
	bool				mDirty;						// Dirty flag
	unsigned long		mLastTyping;				// Time of last typed character
	char				mLastChars[32];				// Last characters typed

	       CAddressTableNewAction*	mNewAction;		// Action to store new items
	       CAddressTableEditAction*	mEditAction;	// Action to store edits
	       CAddressTableEditAction*	mPendingEditAction;	// Action to store pending edits

public:
	enum { class_ID = 'AdTb' };

					CAddressTable();
					CAddressTable(LStream *inStream);
	virtual 		~CAddressTable();

private:
	virtual void		InitAddressTable();

protected:
	virtual void		FinishCreateSelf();							// Get details of sub-panes

public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);		// Keep titles in sync

protected:
	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);

	virtual void		ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell
	virtual void		DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);		// Draw the address

public:
	void		SetDirty(bool dirty)
							{ mDirty = dirty; }						// Set dirty flag
	bool		IsDirty()
							{ return mDirty; }						// Is it dirty
	void		SetAddressBook(CAddressBook* adbk);			// Set the entry list
	void		ResetTable();							// Reset the table from the entry list
	void		ClearTable();							// Reset the table from the entry list

protected:
	virtual void	DoSelectionChanged();

	void		DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
	void		DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
	void		DoPreview();
	void		DoFullView();

	void		DoKeySelection();						// Select from key press

	void		CreateNewLetter(bool option_key);		// Create letter from selection
	void		CreateNewEntry();						// Create a new address

	void		DoEditEntry();							// Edit selected entries
	bool		EditEntry(TableIndexT row);					// Edit specified address
	void		DoDeleteEntry();						// Delete selected entries

	void		CutItems();								// Cut items from table
	void		CopyItems();							// Copy items from table
	bool		AddAddressText(TableIndexT row,
										LHandleStream* txt);		// Add address as text to handle
	bool		AddAdbkAddressText(TableIndexT row,
										LHandleStream* txt);		// Add address as text to handle
	void		PasteItems();							// Paste items from table

public:
	void		AddAddressesFromList(CAddressList* addrs);					// Add from list
	void		ChangeAddressesFromList(CAddressList* old_addrs, CAddressList* new_addrs);	// Change from list
	void		RemoveAddressesFromList(CAddressList* addrs);				// Delete from list

	void		AddSelectionToList(CAddressList* list);		// Add selected entries to list
	bool		CopyEntryToList(TableIndexT row,
										CAddressList* list);		// Copy cell to list
	bool		AddEntryToList(TableIndexT row,
										CAddressList* list);		// Add cell to list
protected:

// Drag methods
	virtual bool		IsCopyCursor(DragReference inDragRef);		// Use copy cursor?
	virtual void		AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect);			// Add address to drag
	virtual void		DoDragReceive(DragReference inDragRef);		// Receive all data
	virtual void		DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size);			// Drop data
	virtual void		DoDragSendData(FlavorType inFlavor,
										ItemReference inItemRef,
										DragReference inDragRef);	// Other flavor requested by receiver
};

#endif
