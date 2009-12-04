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


// Header for CGroupTable class

#ifndef __CGROUPTABLE__MULBERRY__
#define __CGROUPTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

// Consts

// Panes

// Resources

// Messages

// Classes

class CGroup;
class CKeyModifiers;
class LHandleStream;

struct SGroupTableItem
{
	bool		mIsGroup;
	CGroup*		mGroup;

	SGroupTableItem() { mIsGroup = true; }
	SGroupTableItem(bool is_group, CGroup* grp) { mIsGroup = is_group; mGroup = grp; }
	~SGroupTableItem() {};
};

class CAddressBook;
class CAddressBookView;
class CAddressList;
class CGroupList;
class CGroupTableNewAction;
class CGroupTableEditAction;
class CGroupTableReplaceAction;
class StHandleBlock;

class CGroupTable : public CHierarchyTableDrag
{
	friend class CAddressBookView;

private:
	CAddressBook*			mAdbk;
	CAddressBookView*		mTableView;
	bool					mDragGroup;					// Group being dragged
	CGroupTableNewAction*	mNewAction;					// Action to store new items
	CGroupTableEditAction*	mEditAction;				// Action to store edits
	CGroupTableEditAction*	mPendingEditAction;			// Action to store edits during edit dialog
	CGroupTableReplaceAction*	mReplaceAction;			// Action to store replacements
	bool					mDirty;						// Dirty flag
	unsigned long			mLastTyping;				// Time of last typed character
	char					mLastChars[32];				// Last characters typed

public:
	enum { class_ID = 'GpTb' };

					CGroupTable();
					CGroupTable(LStream *inStream);
	virtual 		~CGroupTable();

private:
	virtual void		InitGroupTable();

protected:
	virtual void		FinishCreateSelf();								// Get details of sub-panes

public:
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

protected:
	virtual void		ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);		// Click in the cell
	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);		// Make it target first
	virtual void		DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);				// Draw the message info

	virtual void		CalcCellFlagRect(const STableCell &inCell,
										Rect &outRect);						

public:
	void		SetDirty(bool dirty)
							{ mDirty = dirty; }					// Set dirty flag
	bool		IsDirty()
							{ return mDirty; }					// Is it dirty

	virtual void		AdaptToNewSurroundings();						// Adjust column widths
	void		SetAddressBook(CAddressBook* adbk);					// Set the address book

	TableIndexT	InsertGroup(short pos, CGroup* grp);				// Insert group into display
	void		DeleteGroup(short pos);								// Delete group from display
	void		ReplaceGroup(CGroup* old_grp, CGroup* new_grp);		// Replace one group with another
	void		InsertedAddress(CGroup* grp,
										short num = 1);						// Insert address into display
	void		DeletedAddress(CGroup* grp,
										short num = 1);						// Deleted address from display

	void		ResetTable();									// Reset the table from the entry list
	void		ClearTable();									// Reset the table from the entry list

protected:
	virtual void	DoSelectionChanged();

	void	DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview();											// Do preview
	void	DoFullView();											// Do full view

	void		DoKeySelection();								// Select from key press

	void		CreateNewLetter(bool option_key);				// Create letter from selection
	void		CreateNewEntry();								// Create a new address
	void		DoEditEntry();									// Edit selected entries
	bool		EditEntry(TableIndexT row);							// Edit specified address
	void		DoDeleteEntry();								// Delete selected entries

	void		CutItems();										// Cut items from table
	void		CopyItems();									// Copy items from table
	bool		AddGroupText(TableIndexT row,
										LHandleStream* txt);				// Add groups as text to handle
	void		PasteItems();									// Paste items from table
	bool		AddAddressTextToGroup(TableIndexT row,
												StHandleBlock* txt);		// Add address as text to handle

public:
	void		AddSelectionToList(CAddressList* list);				// Add selected entries to list
	void		AddGroupsFromList(CGroupList* grps);				// Add from list
	void		ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps);	// Change from list
	void		RemoveGroupsFromList(CGroupList* grps);				// Delete from list
	void		SelectGroups(CGroupList* grps);						// Select from list

	bool		AddAddressesToList(TableIndexT row,
										CAddressList* list);				// Add addresses to list

	bool		AddGroupToList(TableIndexT row,
										CGroupList* list);					// Add group to list
protected:

// Drag methods
private:
	virtual bool		IsCopyCursor(DragReference inDragRef);				// Use copy cursor?
	virtual void		AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect);					// Add group to drag
	bool				TestSelectionGroup(TableIndexT row);				// Test for selected item group
	bool				TestCellSameGroup(TableIndexT row,
												CGroup** test);				// Test for selected item group

protected:
	virtual void		EnterDropArea(DragReference inDragRef,
										Boolean inDragHasLeftSender);		// Determine scroll mode
	virtual bool		IsDropCell(DragReference inDragRef, STableCell row);// Can cell be dropped into

	virtual void		DoDragReceive(DragReference inDragRef);				// Receive all data
	virtual void		DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size);					// Drop data into whole table
	virtual void		DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);			// Drop data into cell
	virtual void		DoDragSendData(FlavorType inFlavor,
										ItemReference inItemRef,
										DragReference inDragRef);			// Other flavor requested by receiver




};

#endif
