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

#include "CAddressList.h"
#include "CTableDragAndDrop.h"

// Classes
class CAdbkAddress;
class CAddressBook;
class CAddressBookView;
class CAddressTableNewAction;
class CAddressTableEditAction;
class CKeyModifiers;

class CAddressTable : public CTableDragAndDrop
{
	friend class CAddressBookView;

	DECLARE_DYNCREATE(CAddressTable)

public:
					CAddressTable();
	virtual 		~CAddressTable();

	virtual void		ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual	void		SetDirty(bool dirty);						// Set dirty flag
	virtual bool		IsDirty(void);								// Is it dirty

			void		SetAddressBook(CAddressBook* adbk);			// Set the entry list

			int			GetSortBy() const;

			void		CreateNewLetter(bool option_key);			// Create letter from selection
			void		CreateNewEntry(void);						// Create a new address
			void		DoEditEntry(void);							// Edit selected entries
			bool		EditEntry(TableIndexT row);					// Edit specified address
			void		DoDeleteEntry(void);						// Delete selected entries

			void		AddAddressesFromList(CAddressList* addrs);		// Add from list
			void		ChangeAddressesFromList(CAddressList* old_addrs, CAddressList* new_addrs);	// Change from list
			void		RemoveAddressesFromList(CAddressList* addrs);	// Delete from list

			void		AddSelectionToList(CAddressList* list);		// Add selected entries to list
			bool		CopyEntryToList(TableIndexT row,
										CAddressList* list);		// Copy cell to list
			bool		AddEntryToList(TableIndexT row,
										CAddressList* list);		// Add cell to list
			void		ResetTable(void);							// Reset the table from the mbox
			void		ClearTable();								// Reset the table from the entry list

	// Common updaters
	afx_msg void	OnUpdateEditPaste(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnEditCut(void);
	afx_msg void	OnEditCopy(void);
			bool	AddAddressText(TableIndexT row, cdstring* txt);				// Add address as text
			bool	AddAdbkAddressText(TableIndexT row, cdstring* txt);			// Add address as text
	afx_msg void	OnEditPaste(void);
	afx_msg void	OnCmdActionDeleted(void);

protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void	DoKeySelection(void);										// Select from key press
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);								// Single-click
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void	DoSelectionChanged();

	void		DoSingleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle click user action
	void		DoDoubleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle double-click user action
	void		DoPreview();
	void		DoFullView();

	// Drag & Drop
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual bool	DropData(unsigned int theFlavor,
								char* drag_data,
								unsigned long data_size);				// Drop data

private:
	CAddressBookView*	mTableView;					// View that owns this
	CAddressBook*		mAdbk;						// List of sorted entries
	bool				mDirty;						// Dirty flag
	unsigned long		mLastTyping;				// Time of last typed character
	char				mLastChars[32];				// Last characters typed
	CAddressTableEditAction*	mEditAction;		// Action to store edits

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void 	OnNewMessage(void);
	afx_msg void 	OnNewMessageOption(void);
	afx_msg void	OnNewAddress(void);
	afx_msg void	OnEditAddress(void);
	afx_msg void	OnDeleteAddress(void);

	DECLARE_MESSAGE_MAP()
};

#endif
