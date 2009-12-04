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

	DECLARE_DYNCREATE(CGroupTable)

public:
					CGroupTable();
	virtual 		~CGroupTable();

	virtual	void		SetDirty(bool dirty);							// Set dirty flag
	virtual bool		IsDirty(void);										// Is it dirty

			void		SetAddressBook(CAddressBook* adbk);					// Set the address book

			TableIndexT	InsertGroup(TableIndexT pos, CGroup* grp);			// Insert group into display
			void		DeleteGroup(TableIndexT pos);						// Delete group from display
			void		ReplaceGroup(CGroup* old_grp, CGroup* new_grp);		// Replace one group with another
			void		InsertedAddress(CGroup* grp, int num = 1);			// Insert address into display
			void		DeletedAddress(CGroup* grp, int num = 1);			// Deleted address from display

			void		ResetTable(void);									// Reset the table from the entry list
			void		ClearTable();									// Reset the table from the entry list

			void		CreateNewLetter(bool option_key);			// Create letter from selection
			void		CreateNewEntry(void);								// Create a new address
			void		DoEditEntry(void);									// Edit selected entries
			bool		EditEntry(TableIndexT row);									// Edit specified address
			void		DoDeleteEntry(void);								// Delete selected entries

			void		AddSelectionToList(CAddressList* list);				// Add selected entries to list
			void		AddGroupsFromList(CGroupList* grps);				// Add from list
			void		ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps);	// Change from list
			void		RemoveGroupsFromList(CGroupList* grps);				// Delete from list
			void		SelectGroups(CGroupList* grps);						// Select from list

			bool		AddAddressesToList(TableIndexT row, CAddressList* list);	// Add addresses to list

			bool		AddGroupToList(TableIndexT row, CGroupList* list);			// Add group to list

			bool		AddToText(TableIndexT row, cdstring* str);					// Add group/address to string

	// Common updaters
	afx_msg void	OnUpdateEditPaste(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void 	OnNewMessage(void);
	afx_msg void 	OnNewMessageOption(void);
	afx_msg void	OnEditCut(void);
	afx_msg void	OnEditCopy(void);
			bool	AddGroupText(TableIndexT row, cdstring* txt);					// Add groups as text to handle
	afx_msg void	OnEditPaste(void);
			bool	AddAddressTextToGroup(TableIndexT row, char* txt);			// Add address as text to handle
	afx_msg void	OnCmdActionDeleted(void);

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	DoKeySelection(void);									// Select from key press
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);							// Single-click
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void	DoSelectionChanged();

	void	DoSingleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle click user action
	void	DoDoubleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle double-click user action
	void	DoPreview();											// Do preview
	void	DoFullView();											// Do full view

	// Drag & Drop
	virtual void		SetDragFlavors(TableIndexT row);
	virtual bool		ItemIsAcceptable(COleDataObject* pDataObject);
	virtual bool		IsDropCell(COleDataObject* pDataObject, const STableCell& cell);	// Draw drag row frame
	virtual BOOL		OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual bool		DropData(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size);							// Drop data into whole table
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);							// Drop data into cell
private:
	CAddressBook*		mAdbk;
	CAddressBookView*	mTableView;
	CView*				mParentView;
	bool				mDirty;
	unsigned long		mLastTyping;				// Time of last typed character
	char				mLastChars[32];				// Last characters typed
	CGroupTableEditAction*		mEditAction;			// Action to store edits
	CGroupTableReplaceAction*	mReplaceAction;			// Action to store replacements

	bool				TestSelectionGroup(TableIndexT row);			// Test for selected item group
	bool				TestSameGroup(TableIndexT row, CGroup** test);	// Test for items in same group

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void	OnNewGroup(void);
	afx_msg void	OnEditGroup(void);
	afx_msg void	OnDeleteGroup(void);

	DECLARE_MESSAGE_MAP()

};

#endif
