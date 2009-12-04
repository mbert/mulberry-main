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


// Header for CAddressBookView class

#ifndef __CADDRESSBOOKVIEW__MULBERRY__
#define __CADDRESSBOOKVIEW__MULBERRY__

#include "CTableView.h"
#include "CCommander.h"

#include "CAddressListFwd.h"
#include "CAddressTable.h"
#include "CAddressBookTitleTable.h"
#include "CGrayBackground.h"
#include "CGroupTable.h"
#include "CSplitterView.h"
#include "CTextButton.h"
#include "CTwister.h"

#include "cdmutexprotect.h"

#include <WIN_LUndoer.h>

// Classes
class CAdbkProtocol;
class CAddressBook;
class CAddressBookToolbar;
class CAddressBookWindow;
class CAddressView;
class CGroupTable;
class CSplitterView;
class CToolbar;

class CAddressBookView : public CTableView
{
	friend class CAddressBookWindow;

public:
	enum
	{
		eBroadcast_SelectAddress = 'abvb',
		eBroadcast_SelectGroup = 'abvc'
	};

	typedef vector<CAddressBookView*>	CAddressBookViewList;
	static cdmutexprotect<CAddressBookViewList> sAddressBookViews;	// List of windows (protected for multi-thread access)

					CAddressBookView();
	virtual 		~CAddressBookView();

	static CAddressBookView*	FindView(const CAddressBook* adbk);	// Find the corresponding window
	static unsigned long		CountViews(const CAddressBook* adbk);	// Count views containg this address book
	static bool	ViewExists(const CAddressBookView* wnd);			// Check for view

	virtual void ListenTo_Message(long msg, void* param);

	virtual void	MakeToolbars(CToolbarView* parent);

	virtual void	DoProtocolClearList(CAdbkProtocol* proto);
	virtual void	DoProtocolLogoff(CAdbkProtocol* proto);

	virtual bool	TestClose();
	virtual void	DoClose();

	CAddressBookWindow* GetAddressBookWindow() const;

	virtual void	FocusAddress(void)						// Focus on address
						{ mFocusAddress = true; }
	virtual void	FocusGroup(void)						// Focus on group
						{ mFocusAddress = false; }

	virtual LUndoer* GetUndoer(void)
						{ return &mUndoer; }

	virtual const CUserAction& GetPreviewAction() const;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const;		// Return user action data

	CAddressView*	GetPreview() const
		{ return mAddressView; }
	void	SetPreview(CAddressView* view)
		{ mAddressView = view; }

	bool	GetUsePreview() const
		{ return mUsePreview; }
	void	SetUsePreview(bool use_view);

	void			ViewAddressBook(CAddressBook* adbk);
	void			SetAddressBook(CAddressBook* adbk);
	CAddressBook* 	GetAddressBook() const
		{ return mAdbk; }
	void			ResetAddressBook();

	CAddressTable* GetAddressTable() const
		{ return static_cast<CAddressTable*>(mTable); }
	CGroupTable* GetGroupTable()
		{ return &mGroups; }
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable();								// Clear the entire table

	virtual void	InitColumns();								// Init columns and text
			void	UpdateCaptions();							// Update captions

	virtual int		GetSortBy(void);							// Get sort
	virtual void	SetSortBy(int sort);						// Force change of sort

	
protected:
	CAddressBook*	mAdbk;
	CAddressView*	mAddressView;
	bool			mUsePreview;

	CToolbar*			mToolbar;								// Its toolbar
	CSplitterView		mSplitter;								// Splitter
	CGrayBackground		mFooter;

	CGrayBackground		mAddressPane;								// Pane for addresses
	CTwister			mAddressTwist;								// Show/hide addresses
	CStatic				mShowAddresses;								// Show addresses caption
	CStatic				mHideAddresses;								// Hide addresses caption
	CTextButton			mAddressNew;								// New address
	CTextButton			mAddressDraft;								// Address draft
	CGrayBackground		mAddressesFocusRing;						// Focus ring
	CAddressTable		mAddresses;									// Address list
	CWndAlignment*		mAddressesAlign;							// Aligner for addresses table
	CAddressBookTitleTable	mAddressTitles;							// Address titles
	CGrayBackground		mGroupPane;									// Pane for groups
	CTwister			mGroupTwist;								// Show/hide groups
	CStatic				mShowGroups;								// Show groups caption
	CStatic				mHideGroups;								// Hide groups caption
	CTextButton			mGroupNew;									// New group
	CTextButton			mGroupDraft;								// Group draft
	CGrayBackground		mGroupFocusRing;							// Focus ring
	CGroupTable			mGroups;									// Group list
	CWndAlignment*		mGroupsAlign;								// Aligner for groups table
	CStatic				mAddressTotal;								// Number of addresses
	CStatic				mGroupTotal;								// Number of groups
	LUndoer				mUndoer;									// Undoable actions
	bool				mFocusAddress;								// Is address focussed

	long				mSplitPos;									// Split pos in both expanded mode
	EAddrColumn			mSortColumnType;							// Sort by
	bool				mAscending;									// Ascending sort

			void	DoAddressTwist();
			void	DoGroupTwist();

			ESortAddressBy MapColumnToSort(EAddrColumn col) const;
			EAddrColumn MapSortToColumn(ESortAddressBy sort) const;

public:
			void	AddAddressList(CAddressList* list);				// Add the address list to existing list
			void	AddGroupList(CGroupList* list);					// Add the group list to existing list

	virtual void	ResetState(bool force = false);					// Reset window state
	virtual void	SaveState(void);								// Save current state in prefs
	virtual void	SaveDefaultState();								// Save current state as default

	virtual void	ResetFont(CFont* font);							// Reset list font

protected:
	// Common updaters
	afx_msg void OnUpdateAlways(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDisconnectedSelection(CCmdUI* pCmdUI);			// Update command
	afx_msg void OnUpdateClearDisconnectedSelection(CCmdUI* pCmdUI);	// Update command
	afx_msg void OnUpdateAddrImport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAddrExport(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void		OnEditUndo(void);
	afx_msg void 		OnNewAddress(void);
	afx_msg void 		OnAddressDraft(void);
	afx_msg void 		OnNewGroup(void);
	afx_msg void 		OnGroupDraft(void);
	afx_msg void 		OnEditEntry(void);
	afx_msg void 		OnDeleteEntry(void);
	afx_msg void		OnAddrImport(UINT nID);
	afx_msg void		OnAddrExport(UINT nID);
	afx_msg void		OnSynchroniseAddressBook(void);				// Synchronise the selected address books
	afx_msg void		OnClearDisconnectAddressBook(void);			// Clear disconnect selected address books

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnAddressTwist(void);
	afx_msg void OnGroupTwist(void);

	DECLARE_MESSAGE_MAP()
};

#endif
