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
#include "CWindowStatesFwd.h"

#include "UNX_LUndoer.h"

#include "CAddressTable.h"

#include "cdmutexprotect.h"

// Classes
class CAdbkProtocol;
class CAddressBook;
class CAddressBookWindow;
class CAddressView;
class CFocusBorder;
class CGroupList;
class CGroupTable;
class CSplitterView;
class CTableScrollbarSet;
class CToolbar;
class CTwister;
class JXFlatRect;
class JXIconTextPushButton;
class JXStaticText;

class CAddressBookView : public CTableView
{
	friend class CAddressBookWindow;

public:
	enum
	{
		eBroadcast_SelectAddress = 'abvb',
		eBroadcast_SelectGroup = 'abvc'
	};

	typedef std::vector<CAddressBookView*>	CAddressBookViewList;
	static cdmutexprotect<CAddressBookViewList> sAddressBookViews;	// List of windows (protected for multi-thread access)

					CAddressBookView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressBookView();

	static CAddressBookView*	FindView(const CAddressBook* adbk);		// Find the corresponding window
	static unsigned long		CountViews(const CAddressBook* adbk);	// Count views containg this address book
	static bool	ViewExists(const CAddressBookView* wnd);				// Check for view

	virtual void	OnCreate();

			LUndoer* GetUndoer()
		{ return &mUndoer; }

	virtual void ListenTo_Message(long msg, void* param);

	virtual void	MakeToolbars(CToolbarView* parent);

	virtual void	DoProtocolClearList(CAdbkProtocol* proto);
	virtual void	DoProtocolLogoff(CAdbkProtocol* proto);

	virtual bool	TestClose();
	virtual void	DoClose();

	CAddressBookWindow* GetAddressBookWindow() const;

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
	CGroupTable* GetGroupTable() const
		{ return mGroupTable; }
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable();								// Clear the entire table

	virtual void	InitColumns();								// Init columns and text
			void	UpdateCaptions();							// Update captions

	virtual int		GetSortBy();								// Get sort
	virtual void	SetSortBy(int sort);						// Force change of sort

	
protected:
	CAddressBook*	mAdbk;
	CAddressView*	mAddressView;
	bool			mUsePreview;
	
// begin JXLayout1

    CSplitterView* mPartition;
    JXStaticText*  mAddrNumber;
    JXStaticText*  mGroupNumber;

// end JXLayout1
// begin JXLayout2

    CFocusBorder*         mAddressPane;
    CTwister*             mAddrTwister;
    JXStaticText*         mShowAddrTitle;
    JXStaticText*         mHideAddrTitle;
    JXIconTextPushButton* mAddressNew;
    JXIconTextPushButton* mAddressDraft;
    CTableScrollbarSet*   mAddresses;

// end JXLayout2
// begin JXLayout3

    CFocusBorder*         mGroupPane;
    CTwister*             mGroupTwister;
    JXStaticText*         mShowGroupTitle;
    JXStaticText*         mHideGroupTitle;
    JXIconTextPushButton* mGroupNew;
    JXIconTextPushButton* mGroupDraft;
    CTableScrollbarSet*   mGroups;

// end JXLayout3

	CToolbar*		mToolbar;									// Its toolbar

	CAddressTable*		mAddressTable;
	CGroupTable*		mGroupTable;
	long				mSplitPos;
	EAddrColumn			mSortColumnType;						// Sort by
	LUndoer				mUndoer;								// Undoable actions
	bool				mAscending;								// Ascending sort

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	// Common updaters
			void OnUpdateEditUndo(CCmdUI* pCmdUI);
			void OnUpdateDisconnectedSelection(CCmdUI* pCmdUI);			// Update command
			void OnUpdateClearDisconnectedSelection(CCmdUI* pCmdUI);	// Update command
			void OnUpdateAddrImport(CCmdUI* pCmdUI);
			void OnUpdateAddrExport(CCmdUI* pCmdUI);

	// Command handlers
			void		OnEditUndo();
			void 		OnNewAddress();
			void 		OnAddressDraft();
			void 		OnNewGroup();
			void 		OnGroupDraft();
			void		OnAddrImport(JXTextMenu* menu, JIndex nID);
			void		OnAddrExport(JXTextMenu* menu, JIndex nID);
			void		OnSynchroniseAddressBook();				// Synchronise the selected address books
			void		OnClearDisconnectAddressBook();			// Clear disconnect selected address books

			void	DoAddressTwist();
			void	DoGroupTwist();

			void	AddAddressList(CAddressList* list);		// Add the address list to existing list
			void	AddGroupList(CGroupList* list);			// Add the group list to existing list

			ESortAddressBy MapColumnToSort(EAddrColumn col) const;
			EAddrColumn MapSortToColumn(ESortAddressBy sort) const;

public:
	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

	virtual void	ResetFont(const SFontInfo& finfo);			// Reset text traits
};

#endif
