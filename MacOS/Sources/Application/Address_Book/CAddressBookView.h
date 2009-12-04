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

#include "CAddressTable.h"
#include "CAddressListFwd.h"
#include "CWindowStatesFwd.h"

#include "cdmutexprotect.h"

// Constants
const	PaneIDT		paneid_AddressBookViewSplitter = 'SPLT';
const	PaneIDT		paneid_AddressBookViewAddresses = 'APAN';
const	PaneIDT		paneid_AddressBookViewAddressTwist = 'ATWS';
const	PaneIDT		paneid_AddressBookViewAddressShow = 'ASHW';
const	PaneIDT		paneid_AddressBookViewAddressHide = 'AHID';
const	PaneIDT		paneid_AddressBookViewAddressAdd = 'ADDS';
const	PaneIDT		paneid_AddressBookViewAddressDraft = 'COMS';
const	PaneIDT		paneid_AddressBookViewAddressPanel = 'AFOC';
const	PaneIDT		paneid_AddressBookViewGroups = 'GPAN';
const	PaneIDT		paneid_AddressBookViewGroupTwist = 'GTWS';
const	PaneIDT		paneid_AddressBookViewGroupShow = 'GSHW';
const	PaneIDT		paneid_AddressBookViewGroupHide = 'GHID';
const	PaneIDT		paneid_AddressBookViewGroupAdd = 'ADDG';
const	PaneIDT		paneid_AddressBookViewGroupDraft = 'COMG';
const	PaneIDT		paneid_AddressBookViewGroupPanel = 'GFOC';
const	PaneIDT		paneid_AddressBookViewGroupTable = 'GTAB';
const	PaneIDT		paneid_AddressBookViewAddressTotal = 'ATOT';
const	PaneIDT		paneid_AddressBookViewGroupTotal = 'GTOT';

// Messages
const	MessageT	msg_AddressBookViewAddressTwist = 'ATWS';
const	MessageT	msg_AddressBookViewAddressNew = 'ADDS';
const	MessageT	msg_AddressBookViewAddressDraft = 'COMS';
const	MessageT	msg_AddressBookViewGroupTwist = 'GTWS';
const	MessageT	msg_AddressBookViewGroupNew = 'ADDG';
const	MessageT	msg_AddressBookViewGroupDraft = 'COMG';

// Resources

// Classes
class CAdbkProtocol;
class CAddressBook;
class CAddressBookWindow;
class CAddressView;
class CGroupList;
class CGroupTable;
class CSplitterView;
class CToolbar;
class LDisclosureTriangle;
class CStaticText;

class CAddressBookView : public CTableView,
						 public LListener
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

	enum { class_ID = 'AbVi' };

					CAddressBookView();
					CAddressBookView(LStream *inStream);
	virtual 		~CAddressBookView();

	static CAddressBookView*	FindView(const CAddressBook* adbk);		// Find the corresponding window
	static unsigned long		CountViews(const CAddressBook* adbk);	// Count views containg this address book
	static bool	ViewExists(const CAddressBookView* wnd);				// Check for view

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
		{ return mGroups; }
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
	
	CSplitterView*	mSplitter;									// Splitter
	LDisclosureTriangle*	mAddressTwist;						// Show/hide addresses
	CStaticText*	mShowAddresses;								// Show addresses caption
	CStaticText*	mHideAddresses;								// Hide addresses caption
	LView*			mAddressPanel;								// Panel for address list
	LDisclosureTriangle*	mGroupTwist;						// Show/hide groups
	CStaticText*	mShowGroups;								// Show groups caption
	CStaticText*	mHideGroups;								// Hide groups caption
	LView*			mGroupPanel;								// Panel for group list
	CGroupTable*	mGroups;									// Group list
	CStaticText*	mAddressTotal;								// Number of addresses
	CStaticText*	mGroupTotal;								// Number of groups

	CToolbar*		mToolbar;									// Its toolbar

	long				mSplitPos;								// Split pos in both expanded mode
	EAddrColumn			mSortColumnType;						// Sort by
	bool				mAscending;								// Ascending sort

	virtual void	FinishCreateSelf();

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);		// Respond to clicks in the icon buttons

			void	OnNewAddress();								// New address
			void	OnNewGroup();								// New group
			void	OnSynchroniseAddressBook();					// Synchronise disconnected cache
			void	OnClearDisconnectAddressBook();				// Clear disonnected cache
			void	DoAddressTwist();
			void	DoGroupTwist();

			void	DoImport(long item);
			void	DoExport(long item);

			void	AddAddressList(CAddressList* list);			// Add the address list to existing list
			void	AddGroupList(CGroupList* list);				// Add the group list to existing list

			ESortAddressBy MapColumnToSort(EAddrColumn col) const;
			EAddrColumn MapSortToColumn(ESortAddressBy sort) const;
		
public:
	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState(void);							// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits);	// Reset text traits
};

#endif
