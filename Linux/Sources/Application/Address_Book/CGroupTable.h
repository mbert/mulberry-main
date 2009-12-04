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

// Classes

class CAddressBook;
class CAddressBookView;
class CAddressBookWindow;
class CAddressList;
class CGroup;
class CGroupList;
class CGroupTableNewAction;
class CGroupTableEditAction;
class CGroupTableReplaceAction;
class CKeyModifiers;

struct SGroupTableItem
{
	bool		mIsGroup;
	CGroup*		mGroup;

	SGroupTableItem() { mIsGroup = true; }
	SGroupTableItem(bool is_group, CGroup* grp) { mIsGroup = is_group; mGroup = grp; }
	~SGroupTableItem() {};
};

class CGroupTable : public CHierarchyTableDrag
{
	friend class CAddressBookView;

public:
	CGroupTable(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual ~CGroupTable();

	virtual void OnCreate();

	virtual	void		SetDirty(bool dirty);							// Set dirty flag
	virtual bool		IsDirty();										// Is it dirty

	virtual	void		SetAddressBook(CAddressBook* adbk);					// Set the address book

			TableIndexT	InsertGroup(TableIndexT pos, CGroup* grp);			// Insert group into display
			void		DeleteGroup(TableIndexT pos);						// Delete group from display
			void		ReplaceGroup(CGroup* old_grp, CGroup* new_grp);		// Replace one group with another
			void		InsertedAddress(CGroup* grp, int num = 1);			// Insert address into display
			void		DeletedAddress(CGroup* grp, int num = 1);			// Deleted address from display

			void		ResetTable();										// Reset the table from the entry list
			void		ClearTable();									// Reset the table from the entry list

			void		CreateNewLetter(bool option_key);							// Create a new address
			void		CreateNewEntry();									// Create a new address
			void		DoEditEntry();										// Edit selected entries
			bool		EditEntry(TableIndexT row);									// Edit specified address
			void		DoDeleteEntry();									// Delete selected entries

			void		AddSelectionToList(CAddressList* list);				// Add selected entries to list
			void		AddGroupsFromList(CGroupList* grps);				// Add from list
			void		ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps);	// Change from list
			void		RemoveGroupsFromList(CGroupList* grps);				// Delete from list
			void		SelectGroups(CGroupList* grps);						// Select from list

			bool		AddAddressesToList(TableIndexT row, CAddressList* list);	// Add addresses to list

			bool		AddGroupToList(TableIndexT row, CGroupList* list);			// Add group to list

			bool		AddToText(TableIndexT row, cdstring* str);					// Add group/address to string

	// Common updaters
			void	OnUpdateEditPaste(JXTextMenu* menu, JIndex item);

	// Command handlers
			void	OnNewMessage();
	virtual void	OnEditCut();
	virtual void	OnEditCopy();
			bool	AddGroupText(TableIndexT row, cdstring* txt);					// Add groups as text to handle
	virtual void	OnEditPaste();
			bool	AddAddressTextToGroup(TableIndexT row, char* txt);			// Add address as text to handle
	virtual void	ActionDeleted();

			void	OnNewGroup();
			void	OnEditGroup();
			void	OnDeleteGroup();

protected:
	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual bool	HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual void	DoKeySelection();									// Select from key press
	virtual void	LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);
	virtual void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);

	virtual void	DoSelectionChanged();

	void	DoSingleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview();											// Do preview
	void	DoFullView();											// Do full view

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	// Drag & Drop
	virtual bool	GetSelectionAtom(CFlavorsList& atom);
	virtual bool	IsDropCell(JArray<Atom>& typeList, const STableCell& cell); // Can cell be dropped into
	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
	virtual bool	DropData(Atom theFlavor, unsigned char* drag_data,
								unsigned long data_size);						// Drop data into whole table
	virtual bool	DropDataIntoCell(Atom theFlavor, unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell);		// Drop data into cell

private:
	CAddressBook*				mAdbk;
	CAddressBookView*			mTableView;
	bool						mDirty;
	unsigned long				mLastTyping;							// Time of last typed character
	char						mLastChars[32];							// Last characters typed
	CGroupTableEditAction*		mEditAction;							// Action to store edits
	CGroupTableReplaceAction*	mReplaceAction;							// Action to store replacements

	bool				TestSelectionGroup(TableIndexT row);					// Test for selected item group
	bool				TestSameGroup(TableIndexT row, CGroup** test);			// Test for items in same group

	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,			// Draw the items
							 const JRect& inLocalRect);
};

#endif
