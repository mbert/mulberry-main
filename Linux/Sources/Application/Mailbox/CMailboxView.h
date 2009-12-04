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


// Header for CMailboxView class

#ifndef __CMAILBOXVIEW__MULBERRY__
#define __CMAILBOXVIEW__MULBERRY__

#include "CTableView.h"

#include "CMailboxTable.h"
#include "CMailboxTitleTable.h"
#include "CMessageView.h"

#include "cdmutexprotect.h"

// Constants

// Classes
class CMailboxToolbarPopup;
class CMailboxWindow;
class CServerView;
class CToolbar;
class JXMultiImageCheckbox;

class CMailboxView : public CTableView
{
	friend class CMailboxWindow;
	friend class CMailboxTable;

public:
	enum
	{
		eBroadcast_SelectMessage = 'mbvb',
		eBroadcast_AddSubstitute = 'mbva',
		eBroadcast_SelectSubstitute = 'mbvs',
		eBroadcast_ChangeSubstitute = 'mbvc',
		eBroadcast_ChangeIconSubstitute = 'mbvi',
		eBroadcast_MoveSubstitute = 'mbvm',
		eBroadcast_RemoveSubstitute = 'mbvr'
	};

	struct SBroadcast_AddSubstitute
	{
		const char*		mName;
		bool			mUnseenIcon;
	};

	struct SBroadcast_ChangeSubstitute
	{
		unsigned long	mIndex;
		const char*		mName;
		bool			mUnseenIcon;
	};

	struct SBroadcast_ChangeIconSubstitute
	{
		unsigned long	mIndex;
		bool			mUnseenIcon;
	};

	struct SBroadcast_MoveSubstitute
	{
		unsigned long	mOldIndex;
		unsigned long	mNewIndex;
	};

	typedef std::vector<CMailboxView*>	CMailboxViewList;
	static cdmutexprotect<CMailboxViewList> sMailboxViews;	// List of windows (protected for multi-thread access)

	struct SMailboxSubstitute
	{
	public:
		CMbox*							mMailbox;
		bool							mLocked;
		bool							mDynamic;
		long							mScrollHpos;
		long							mScrollVpos;
		ulset							mSelected;
		unsigned long					mPreviewUID;
		unsigned long					mLastAccess;
		CMessageView::SMessageViewState	mMessageView;

		SMailboxSubstitute()
			{ _init(); }
		SMailboxSubstitute(const SMailboxSubstitute& copy)
			{ _copy(copy); }

		SMailboxSubstitute& operator=(const SMailboxSubstitute& copy)			// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }


	private:
		void _init()
			{ mMailbox = NULL;
			  mLocked = false;
			  mDynamic = false;
			  mScrollHpos = -1;
			  mScrollVpos = -1;
			  mPreviewUID = 0;
			  mLastAccess = 0; }
		void _copy(const SMailboxSubstitute& copy)
			{ mMailbox = copy.mMailbox;
			  mLocked = copy.mLocked;
			  mDynamic = copy.mDynamic;
			  mScrollHpos = copy.mScrollHpos;
			  mScrollVpos = copy.mScrollVpos;
			  mSelected = copy.mSelected;
			  mPreviewUID = copy.mPreviewUID;
			  mLastAccess = copy.mLastAccess;
			  mMessageView = copy.mMessageView; }
	};
	typedef std::vector<SMailboxSubstitute>	SMailboxSubstituteList;

	CMailboxView(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual 		~CMailboxView();

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	static CMailboxView*	FindView(const CMbox* mbox);	// Find the corresponding window
	static unsigned long	CountViews(const CMbox* mbox);	// Count how many views have the mailbox open
	static bool	ViewExists(const CMailboxView* wnd);		// Check for window

	virtual void	ListenTo_Message(long msg, void* param);

	CMailboxWindow* GetMailboxWindow() const;

	virtual const CUserAction& GetPreviewAction() const;		// Return user action data
	virtual const CUserAction& GetFullViewAction() const;		// Return user action data

	CToolbar* GetToolbar() const
		{ return mToolbar; }

	CServerView*	GetOwnerView() const
		{ return mServerView; }
	void	SetOwnerView(CServerView* view)
		{ mServerView = view; }
	void	FocusOwnerView();

	CMessageView*	GetPreview() const
		{ return mMessageView; }
	void	SetPreview(CMessageView* view);

	bool	GetUsePreview() const
		{ return mUsePreview; }
	void	SetUsePreview(bool use_view);

	bool	GetUseSubstitute() const
		{ return mUseSubstitute; }
	virtual void	SetUseSubstitute(bool use_substitute);

	virtual CMailboxToolbarPopup* GetCopyBtn() const
		{ return NULL; }

	JXMultiImageCheckbox* GetSortBtn() const
		{ return mSortBtn; }

			void	ForceClose(CMbox* mbox);
			void	ForceClose(CMboxProtocol* proto);
	virtual void	DoProtocolLogoff(const CMboxProtocol* proto) = 0;
	virtual void	DoRemoveWD(const CMboxList* wd) = 0;

			bool	ContainsMailbox(const CMbox* mbox);							// Is mailbox one of the substitutes
			bool	ContainsProtocol(const CMboxProtocol* proto);				// Is proto used by any substitutes

			bool	HasSubstituteItems() const										// Are there ay items
				{ return mSubsList.size() != 0; }
			bool	IsSubstitute(const CMbox* mbox) const;							// Existing substitution?
			bool	IsSubstitute(const CMboxProtocol* proto) const;					// Existing substitution?
			bool	IsSubstitute(const cdstring& acctname) const;					// Existing substitution?
			unsigned long	GetSubstituteIndex() const								// Get index of current item
				{ return mSubstituteIndex; }
			unsigned long	GetSubstituteIndex(const CMbox* mbox) const;			// Get index of item
			unsigned long	GetSubstituteIndex(const CMboxProtocol* proto) const;	// Get index of item
			unsigned long	GetSubstituteIndex(const CMboxList* wd) const;			// Get index of item
			unsigned long	GetDynamicIndex() const;							// Get index of item
			bool	HasDynamicItem() const;										// Get index of item
			void	OpenSubstitute(CMbox* mbox,									// New or existing substitution
									bool select);
			void	SaveSubstitute(CMbox* mbox);								// Save substitution state
			void	ChangeSubstitute(CMbox* oldmbox, CMbox* newmbox);			// Change substitution item
			void	ChangeIconSubstitute(CMbox* mbox, bool icon);				// Change substitution item icon
			void	MoveSubstitute(unsigned long oldindex,						// Move substitution item
									unsigned long newindex);
			void	RenameSubstitute(unsigned long index);						// Rename substitution item
			void	RenamedSubstitute(unsigned long index);						// Renamed substitution item
			bool	IsSubstituteLocked(unsigned long index) const;				// Is it locked
			void	LockSubstitute(unsigned long index);						// Lock substitution item
			void	LockAllSubstitutes();										// Lock all substitution items
			bool	IsSubstituteDynamic(unsigned long index) const;				// Is it dynamic
			void	DynamicSubstitute(unsigned long index);						// Make substitution item dynamic
			void	SaveSubstitutes();											// Save substitution items
			void	RestoreSubstitutes();										// Restore substitution items
			void	CloseSubstitute(CMbox* mbox);								// Close substitution state
			void	SetSubstitute(unsigned long index);							// Set this as the current substitute view
			void	CycleSubstitute(bool forward);								// Cycle to next tab in the list
			void	GetSubstituteItems(cdstrvect& items) const;					// Get list of current items
			void	SetSubstituteItems(const cdstrvect& items);					// Set list of items

			void	GetStaticItems(cdstrvect& items) const;						// Get list of current (non-tab) items
			void	SetStaticItems(const cdstrvect& items);						// Set list of (non-tab) items

	virtual void	ViewMbox(CMbox* anMbox,
						bool use_dynamic = true,
						bool is_search = false) = 0;		// Set the mbox for viewing
	virtual void	SetMbox(CMbox* anMbox,
						bool is_search = false) = 0;		// Set the mbox
	virtual bool	Recycle(CMbox* anMbox,					// Recycle to new mbox
						bool is_search = false) = 0;
	virtual bool	Substitute(CMbox* anMbox,				// Replace current displayed mailbox with another
						bool select = true,
						bool is_search = false) = 0;
	CMbox*	GetMbox() const;								// Get its mbox

	CMailboxTable* GetTable() const
		{ return static_cast<CMailboxTable*>(mTable); }
	virtual CMailboxTitleTable* GetTitles()
		{ return static_cast<CMailboxTitleTable*>(mTitles); }
	virtual void	ResetTable();							// Reset the table
	virtual void	ResetTableNew();						// Reset the table
	virtual void	ClearTable()							// Clear the table
		{ GetTable()->ClearTable(); }

	virtual void	RenamedMailbox(const CMbox* mbox) = 0;	// Mailbox was renamed

	void	ChangedMessage(const CMessage* aMsg);			// Message status changed
	void	SelectMessage(const CMessage* aMsg);			// Select msg cell in table
	virtual void	SelectNextNew(bool backwards = false);	// Select next new message

#ifdef __use_speech
	virtual void	SpeakRecentMessages();					// Speak recent messages in mailbox
#endif

	virtual void	InitColumns();							// Init columns and text
	
	virtual int		GetSortBy();							// Get sort
	virtual void	SetSortBy(int sort);					// Force change of sort

	virtual int		GetShowBy();							// Get sort direction
	virtual void	SetShowBy(int sort);					// Force change of sort direction
	virtual void	ToggleShowBy();							// Force toggle of sort direction

	void	RefreshSmartAddress();							// Refresh Smart Address column
	
protected:
	CToolbar*				mToolbar;
	CServerView*			mServerView;
	CMessageView*			mMessageView;
	
	bool					mUsePreview;
	bool					mPreviewInit;
	bool					mUseSubstitute;
	unsigned long			mSubstituteIndex;
	unsigned long			mSubstituteAccess;
	bool					mUseDynamicTab;
	SMailboxSubstituteList	mSubsList;
	JXMultiImageCheckbox*	mSortBtn;
};

#endif
