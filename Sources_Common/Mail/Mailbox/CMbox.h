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


// Header for Mailbox class

#ifndef __CMBOX__MULBERRY__
#define __CMBOX__MULBERRY__

#include "CTreeNode.h"
#include "CMboxFwd.h"

#include "SBitFlags.h"

#include "CMboxACL.h"
#include "CMessageFwd.h"
#include "CMessageList.h"
#include "CQuotas.h"
#include "CSearchItem.h"
#include "CStreamTypeFwd.h"

#include "cdomutex.h"
#include "templs.h"

class CAttachment;
class CIdentity;
class CMboxList;
class CMboxProtocol;
class CMessage;
class CMessageList;
class LStream;

using namespace NMbox;

class CMbox : public CMboxNode
{
public:

	typedef cdomutex<CMbox> mbox_mutex;						// Mailbox mutex type
	typedef cdomutex<CMbox>::lock_cdomutex mbox_lock;		// Mailbox mutex lock type
	typedef cdomutex<CMbox>::trylock_cdomutex mbox_trylock;	// Mailbox mutex try lock type
	static mbox_mutex	_smutex;							// Used for multithreaded locks

protected:

	class CMboxOpen
	{
		friend class CMbox;

		CMboxProtocol*		mMsgMailer;					// Connection for open mailbox
		CMessageList*		mMessages;					// List of server messages
		CMessageList*		mSortedMessages;			// List of sorted, cached messages
		unsigned long		mNumberCached;				// Number of messages cached
		unsigned long		mCacheIncrement;			// Number of messages added to cache each time we have a cache miss
		unsigned long		mCheckRecent;				// New recent messages during CHECK
		unsigned long		mMboxRecent;				// Current unseen recent messages
		EViewMode			mViewMode;					// View mode
		bool				mAutoViewMode;				// Automatic view mode in use
		CSearchItem			mViewCurrent;				// Current view specification
		ulvector			mViewSearchResults;			// View search results
		ESortMessageBy		mSortBy;					// Value of sort by
		EShowMessageBy		mShowBy;					// Value of show by
		unsigned long		mFirstNew;					// First new message
		ulvector			mFullLocal;					// List of message UIDs fully disconnected
		ulvector			mPartialLocal;				// List of message UIDs partially disconnected
		unsigned long		mRefCount;					// Reference count on open
		
		unsigned long		mPartialCount;				// Partial fetch count
		unsigned long		mPartialStart;				// Partial fetch count

		CMboxOpen();
		~CMboxOpen();
		
		static mbox_mutex	_mutex;						// Used for multithreaded Open/Close
	};

	class CMboxStatus
	{
		friend class CMbox;

		// Used for Properties
		unsigned long		mNumberExists;				// Number found in box on server
		unsigned long		mNumberRecent;				// Number new on server
		unsigned long		mNumberUnseen;				// Number unseen (STATUS)
		unsigned long		mSize;						// Size of mailbox
		unsigned long		mUIDValidity;				// UIDValidity
		unsigned long		mUIDNext;					// UIDNext
		unsigned long		mLastSync;					// UIDNext
		NMessage::EFlags	mAllowedFlags;				// Flags that can be changed
		ulvector			mSearchResults;				// SEARCH results
		SACLRight			mMyRights;					// User's rights on this mailbox
		CMboxACLList*		mACLs;						// List of ACLs on this mailbox
		cdstrvect*			mQuotas;					// List of quota root names

		CMboxStatus();
		~CMboxStatus() { delete mACLs; delete mQuotas;}
	};

	// I N S T A N C E  V A R I A B L E S

	CMboxProtocol*		mMailer;					// Mail protocol to use

	// Used when in list
	CMboxList*			mMboxList;					// Owning list
	SBitFlags			mFlags;						// Mailbox flags

	// Used for properties
	CMboxStatus*		mStatusInfo;

	// Used when open
	CMboxOpen*			mOpenInfo;					// Information used when open

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

private:
	CMbox();									// Do not allow default construction
public:
	CMbox(const CMbox& copy);					// Construct copy
	explicit CMbox(CMboxProtocol* mailer,
			const char* itsFullName,
			char itsDirDelim,
			CMboxList* itsMboxList,
			bool subscribed = false);			// Construct with name, protocol, subscribed state
	virtual ~CMbox();

private:
	void 	InitMbox();
	void	InitStatusInfo()
		{ if (!mStatusInfo) mStatusInfo = new CMboxStatus; }

	// O T H E R  M E T H O D S
public:

	CMbox*			AddMbox();					// Add it to list

// Status
	void			SetProtocol(CMboxProtocol* proto);
	CMboxProtocol*	GetProtocol() const
						{ return mMailer; }						// Return owning mailer
	CMboxProtocol*	GetMsgProtocol() const
						{ return (mOpenInfo ? mOpenInfo->mMsgMailer : NULL); }	// Return message mailer
	bool			TestMailer(const CMboxProtocol* test) const					// Check whether mailer could be ours
						{ return (test == GetProtocol()) || (test == GetMsgProtocol()); }

	bool			HasStatus() const						// Sync status with another
						{ return mStatusInfo; }

	void SetMboxList(CMboxList* mbox_list)								// Set WD inde
				{ mMboxList = mbox_list;}
	CMboxList* GetMboxList() const							// Get WD index
				{ return mMboxList;}
	CMboxList* GetMboxList()								// Get WD index
				{ return mMboxList;}
	const cdstring&	GetWD() const;							// Get WD prefix

	virtual cdstring GetAccountName(bool multi = true) const;	// Get account relative mailbox name
	cdstring GetURL(bool full = false) const;					// Get URL of mailbox

	const CIdentity* GetTiedIdentity() const;							// Get identity associated with mailbox

	// Mailbox flags
	void	SetFlags(EFlags new_flags, bool add = true)			// Set flags
				{ mFlags.Set(new_flags, add); }
	void	SetListFlags(EFlags new_flags);						// Set list flags only
	EFlags	GetFlags() const								// Get flags
				{ return (EFlags) mFlags.Get(); }

	void	SetAllowedFlags(NMessage::EFlags new_flags)			// Set flags
				{ if (mStatusInfo) mStatusInfo->mAllowedFlags = new_flags; }
	NMessage::EFlags	GetAllowedFlags() const					// Get flags
				{ return mStatusInfo ? mStatusInfo->mAllowedFlags : NMessage::eAllPermanent; }
	bool	HasAllowedFlag(NMessage::EFlags test_flags) const			// Test flags
				{ return (GetAllowedFlags() & test_flags) != 0; }
	void	DetermineAllowedFlags();						// Update allowed flags status

	// Mailbox Status
	void	SetOpen(bool open)								// Set open state
				{ SetFlags(eOpen, open); SetOpenSomewhere(open); }
	bool	IsOpen() const									// Check open state
				{return mFlags.IsSet(eOpen);}

	void	SetFullOpen(bool open)							// Set full open state
				{ SetFlags(eFullOpen, open); }
	bool	IsFullOpen() const								// Check full open state
				{return mFlags.IsSet(eFullOpen);}
	bool	IsBeingOpened() const							// Currently being opened
				{return mFlags.IsSet(eBeingOpened);}

	void	SetOpenSomewhere(bool open);					// Set open somewhere state
	bool	IsOpenSomewhere() const							// Check open somewhere state
				{return mFlags.IsSet(eOpenSomewhere);}

	virtual bool	IsDirectory() const
				{ return !NoInferiors() && NoSelect(); }
	virtual bool	IsHierarchy() const
				{ return !NoInferiors(); }
	virtual void	SetHasInferiors(bool inferiors)
		{ SetFlags(eHasInferiors, inferiors); }
	virtual void	SetIsExpanded(bool expanded)
		{ SetFlags(eIsExpanded, expanded); }

	void	SetSubscribed(bool subs);							// Set subscribed state
	bool	IsSubscribed() const							// Get subscribed state
				{return mFlags.IsSet(eSubscribed);}

	bool	IsReadOnly() const								// Get read only
				{ return mFlags.IsSet(eReadOnly);}
	bool	IsExamine() const								// Get examine state
				{ return mFlags.IsSet(eExamine);}
	bool	IsNoCache() const								// Get no cache state
				{ return mFlags.IsSet(eNoCache);}

	bool	IsAutoCheck() const
				{ return mFlags.IsSet(eAutoCheck);}
	bool	IsCopyTo() const
				{ return mFlags.IsSet(eCopyTo);}
	bool	IsAppendTo() const
				{ return mFlags.IsSet(eAppendTo);}
	bool	IsPuntOnClose() const
				{ return mFlags.IsSet(ePuntOnClose);}
	bool	IsAutoSync() const
				{ return mFlags.IsSet(eAutoSync);}
	bool	IsLocalMbox() const
				{ return mFlags.IsSet(eLocalMbox);}
	bool	IsCachedMbox() const
				{ return mFlags.IsSet(eCachedMbox);}
	bool	IsSynchronising() const
				{ return mFlags.IsSet(eSynchronising);}
	bool	IsNoCabinet() const
				{ return mFlags.IsSet(eNoCabinet);}
	bool	IsExternalSort() const
				{ return mFlags.IsSet(eExternalSort);}

	bool	NoInferiors() const								// Check NoInferiors flag
				{return mFlags.IsSet(eNoInferiors);}
	bool	NoSelect() const								// Check NoSelect flag
				{return mFlags.IsSet(eNoSelect);}
	bool	Marked() const									// Check Marked flag
				{return mFlags.IsSet(eMarked);}
	bool	Unmarked() const								// Check Unmarked flag
				{return mFlags.IsSet(eUnMarked);}
	bool	Error() const									// Check Error flag
				{return mFlags.IsSet(eError);}
	virtual bool	HasInferiors() const					// Check HasInferiors flag
				{return mFlags.IsSet(eHasInferiors);}
	virtual bool	IsExpanded() const						// Check IsExpanded flag
				{return mFlags.IsSet(eIsExpanded);}
	bool	HasExpanded() const								// Check HasExpanded flag
				{return mFlags.IsSet(eHasExpanded);}

	// Properties
	void	UpdateStatus(const CMbox* copy);							// Copy status information from another mailbox

	void	SetNumberFound(const unsigned long num)						// Set number of messages in mailbox on server
				{ if (mStatusInfo) { mStatusInfo->mNumberExists = num; } }
	unsigned long	GetNumberFound() const								// Get number of messages in mailbox on server
				{ return mStatusInfo ? mStatusInfo->mNumberExists : 0; }
	unsigned long	GetNumberMessages()	const							// Get number of cached messages
				{ return (IsFullOpen() ? mOpenInfo->mSortedMessages->size() : 0); }

	void	SetNumberRecent(const unsigned long num)					// Set number of recent messages in mailbox on server
				{ if (mStatusInfo) { mStatusInfo->mNumberRecent = num; } }
	unsigned long	GetNumberRecent() const								// Get number of recent messages in mailbox on server
				{ return mStatusInfo ? mStatusInfo->mNumberRecent : 0; }

	void	SetCheckRecent(const unsigned long num)						// Set number of new recent messages in mailbox on server
				{ if (IsFullOpen()) mOpenInfo->mCheckRecent = num; }
	unsigned long	GetCheckRecent() const								// Get number of new' recent messages in mailbox on server
				{ return (IsFullOpen() ? mOpenInfo->mCheckRecent : 0); }

	void	SetMboxRecent(const unsigned long num)						// Set number of new recent messages in mailbox on server
				{ if (IsFullOpen()) mOpenInfo->mMboxRecent = num; }
	unsigned long	GetMboxRecent() const								// Get number of new' recent messages in mailbox on server
				{ return (IsFullOpen() ? mOpenInfo->mMboxRecent : 0); }

	unsigned long	GetNumberViewRecent() const							// Get number of 'user' recent messages in mailbox on server
				{ return (IsFullOpen() ? mOpenInfo->mMboxRecent : GetNumberRecent()); }

	void	SetNumberUnseen(const unsigned long num)					// Set number of unseen messages in mailbox on server
				{ if (mStatusInfo) mStatusInfo->mNumberUnseen = num; }
	unsigned long	GetNumberUnseen()	const							// Get number of unseen messages
				{ return (IsFullOpen() ?
							CountFlags(NMessage::eNone, NMessage::eSeen) :
							(mStatusInfo ? mStatusInfo->mNumberUnseen : 0)); }
	bool	AnySeen() const												// Are there any seen but not deleted messages?
				{ return AnyFlags(NMessage::eSeen, NMessage::eDeleted); }

	bool	AnyNew() const;												// Are there any new messages

	unsigned long	GetNumberMatch() const								// Get number of matching messages
				{ return (IsFullOpen() ?
							mOpenInfo->mViewSearchResults.size() : 0); }

	unsigned long	GetSize() const
				{ return (mStatusInfo ? mStatusInfo->mSize : ULONG_MAX); }
	void	CheckSize();
	void	SetSize(unsigned long size)
				{ if (mStatusInfo) mStatusInfo->mSize = size; }

	unsigned long	GetUIDValidity() const
		{ return (mStatusInfo ? mStatusInfo->mUIDValidity : 0); }
	void	SetUIDValidity(unsigned long uidv)
		{ if (mStatusInfo) mStatusInfo->mUIDValidity = uidv; }
	void	ChangeUIDValidity(unsigned long uidv);

	unsigned long	GetUIDNext() const
		{ return (mStatusInfo ? mStatusInfo->mUIDNext : 0); }
	void	SetUIDNext(unsigned long uidn)
		{ if (mStatusInfo) mStatusInfo->mUIDNext = uidn; }
	void	ChangeUIDNext(unsigned long uidn);

	unsigned long	GetLastSync();
	void	SetLastSync(unsigned long sync)
		{ if (mStatusInfo) mStatusInfo->mLastSync = sync; }
	void	ChangeLastSync(unsigned long sync);

	unsigned long GetPartialCount() const
		{ return mOpenInfo ? mOpenInfo->mPartialCount : 0; }
	void SetPartialCount(unsigned long count) const
		{ if (mOpenInfo) mOpenInfo->mPartialCount = count; }

	unsigned long GetPartialStart() const
		{ return mOpenInfo ? mOpenInfo->mPartialStart : 0; }
	void SetPartialStart(unsigned long start) const
		{ if (mOpenInfo) mOpenInfo->mPartialStart = start; }

	// Searching
	EViewMode GetViewMode() const
		{ return (IsFullOpen() ? mOpenInfo->mViewMode : eViewMode_All); }
	bool SetViewMode(EViewMode mode);

	bool GetAutoViewMode() const
		{ return (IsFullOpen() ? mOpenInfo->mAutoViewMode : false); }
	void SetAutoViewMode(bool mode)
		{ if (IsFullOpen()) mOpenInfo->mAutoViewMode = mode; }

	bool	SetViewSearch(const CSearchItem* spec, bool update = false);
	const CSearchItem* GetViewSearch() const
		{ return (mOpenInfo != NULL) ? &mOpenInfo->mViewCurrent : NULL; }

	void	Search(const CSearchItem* spec, ulvector* results = NULL, bool uids = false, bool no_flags = false);
	
	bool	OpenSearch();
	bool	TransferSearch();
	void	SetSearchFlags(const ulvector& matches);
	const ulvector& GetSearchResults() const					// Warning this is NOT protected against NULL mStatusInfo
		{ return mStatusInfo->mSearchResults; }

	// Access Control
	void	SetMyRights(SACLRight rights)						// Set user's rights to this mailbox
		{ if (mStatusInfo) mStatusInfo->mMyRights = rights; }
	SACLRight	GetMyRights() const
		{ return (mStatusInfo ? mStatusInfo->mMyRights : SACLRight()); }
	void	CheckMyRights();									// Get user's rights from server

	void	AddACL(const CMboxACL* acl);						// Add ACL to list
	void	SetACL(CMboxACL* acl);								// Set ACL on server
	void	DeleteACL(CMboxACL* acl);							// Delete ACL on server
	void	ResetACLs(const CMboxACLList* acls);				// Reset all acls
	CMboxACLList*	GetACLs() const
		{ return (mStatusInfo ? mStatusInfo->mACLs : NULL); }
	void	CheckACLs();									// Get ACLs from server

	// Quota
	void	AddQuotaRoot(const char* txt);					// Add quotaroot name to list
	CQuotaRootList* GetQuotas();							// Get list of quotaroots relevant to this mailbox
	void	CheckQuotas();									// Get Quotas from server

	// Open state
	bool	SortBy(ESortMessageBy sort_by);					// Set sorting
	bool	ReSort();										// Set sorting
	bool	SortRemove(CMessage* msg);						// Adjust for message removal
	ESortMessageBy GetSortBy() const						// Get sorting
				{ return mOpenInfo ? mOpenInfo->mSortBy : cSortMessageNumber; }
	bool	ValidSort(ESortMessageBy sort_by);				// Determine if sorting is valid

	void	ShowBy(EShowMessageBy show_by);					// Set sorting method
	EShowMessageBy GetShowBy() const						// Get sorting method
				{ return mOpenInfo ? mOpenInfo->mShowBy : cShowMessageAscending; }

	// Cache details
	void	SetNumberCached(unsigned long num)				// Set the number of cached messages
				{ if (mOpenInfo) mOpenInfo->mNumberCached = num; }
	unsigned long	GetNumberCached() const					// Get the number of cached messages 
				{ return mOpenInfo ? mOpenInfo->mNumberCached : 0; }
	bool	FullyCached() const;

	void	CachedMessage(CMessage* aMsg)					// Message in this mailbox was cached
		{ if (mOpenInfo) mOpenInfo->mNumberCached++; }
	void	UncachedMessage(CMessage* aMsg)					// Message in this mailbox was uncached
		{ if (mOpenInfo) mOpenInfo->mNumberCached--; }

	void	SetCacheIncrement(unsigned long increment)
		{ if (mOpenInfo) mOpenInfo->mCacheIncrement = increment; }
	unsigned long	GetCacheIncrement() const
		{ return mOpenInfo ? mOpenInfo->mCacheIncrement : 0; }

	unsigned long GetLOCache() const;
	void SetLOCache(unsigned long value) const;

	unsigned long GetHICache() const;
	void SetHICache(unsigned long value) const;
	bool GetUseHICache() const;

	unsigned long GetSortCache() const;

	unsigned long	GetNumberDeleted()	const						// Get number of cached deleted
				{ return CountFlags(NMessage::eDeleted); }
	bool	AnyDeleted() const										// Are there any deleted messages?
				{return AnyFlags(NMessage::eDeleted); }

	void	SetFirstNew(unsigned long num)								// Set the first new message
				{ if (mOpenInfo) mOpenInfo->mFirstNew = num; }
	unsigned long	GetFirstNew() const									// Get the first new message
				{ return mOpenInfo ? mOpenInfo->mFirstNew : 0; }

	void	DoInitialSearch();								// Find unseen/deleted/recent and return first unseen
	void	DoInitialSearchUsingSearch();					// Find unseen/deleted/recent and return first unseen
	void	SyncDisconnectedMessage(CMessage* msg);			// Sync with full and partial uids

	void	ClearCheckRecent();								// Clear new recent pseudo flag from all messages
	void	ClearSmartAddress();							// Clear smart address flag from all messages

	// Mailbox Ops
	void	Create();										// Create it
	void	Open(CMboxProtocol* proto = NULL,
					bool update = true,
					bool examine = false,
					bool full = true);						// Open it
	void	Reopen();										// Open it with existing connection
	void	Recover();										// Recover from closed connection
	bool	OpenIfOpen();									// Open it if already open!
	void	CloseSilent(bool force = false)
		{ Close(false, true, force, false); }
	void	CloseNoPunt()
		{ Close(false, false, false, false); }
	void	CloseMboxOnly(CMboxProtocol** reuse,
							bool silent = false,
							bool force = false)
		{ Close(true, silent, force, true, reuse); }
	void	Close(bool grab_proto = false,
					bool no_check = false,
					bool force = false,
					bool allow_punt = true,
					CMboxProtocol** resuse = NULL);			// Close it
	bool	SafeToClose() const;							// Can call close
	void	Abort(bool allow_recover = false);				// Program initiated abort of command in progress
	void	Rename(const char* new_name);					// Rename it
	void	MoveMbox(const CMbox* dir, bool sibling);		// Move it to another directory or as sibling
	void	Delete();										// Delete it
	void	Remove();										// Remove it
	void	Expunge(bool closing);							// Expunge messages
	long	Check(bool fast = false, bool silent = false);  // Check for new messages
	void	ProcessCheck();									// Process check for new messages
	void	Subscribe(bool update = true);					// Subscribe to it
	void	Unsubscribe(bool update = true);				// Unsubscribe from it

	void	Rebuild();										// Rebuild from cache
	void	Synchronise(bool fast, bool partial, unsigned long size);			// Synchronise to local
	void	ClearDisconnect();								// Clear disconnected cache
	void	SwitchDisconnect(CMboxProtocol* local);			// Switch into disconnected mode

	CMbox*	GetFirstChild() const;							// Get the first child mailbox of this one
	CMbox*	GetNextSibling() const;							// Get next sibling of this mailbox

	// Message ops
	void	RenumberMessages()								// Renumber messages
		{ if (IsFullOpen()) mOpenInfo->mMessages->Renumber(); }
	void	CacheAllMessages();								// Force all messages into cache
	void	CacheMessage(unsigned long msg_num,				// Force message into cache
							unsigned long count = 0);
	void	CacheMessage(const ulvector& nums, bool uids);	// Force messages into cache
	void	CacheUIDs(const ulvector& nums);				// Force message UIDs into cache
	void	UncacheMessages();								// Force messages out of cache
	void	UncacheMessage(unsigned long msg_num);			// Force a message out of cache
	void	LoadMessages();									// Load messages from server

	void	SearchCount(unsigned long start, unsigned long ctr);	// Search for unsee/deleted to ensure counts are up to date

	void	LoadReferences();								// Load all references headers (needed for local threading)

	void	SortMessages()									// Sort messages after change
				{ if (IsFullOpen()) ReSort(); }

	void	StartAppend();
	void	StopAppend();
	void	AppendMessage(CMessage* msg,
							unsigned long& new_uid,
							bool dummy_files = false,
							bool doMRU = true);				// Append the specified message

	CMessage*	GetMessage(unsigned long msg_num,
							bool sorted = false);			// Get the specified message
	CMessage*	GetMessageUID(unsigned long uid);			// Get the specified message
	CMessage*	GetMessageLocalUID(unsigned long uid);		// Get the specified message
	CMessage*	GetCacheMessage(unsigned long msg_num,
							bool sorted = false);			// Get the specified message and force into cache

	unsigned long 	CountFlags(NMessage::EFlags set,			// Get number of messages with matching flags
								NMessage::EFlags not_set = NMessage::eNone) const
		{ return (IsFullOpen() ? mOpenInfo->mMessages->CountFlags(set, not_set) : 0); }
	bool 	AnyFlags(NMessage::EFlags set,						// Check for at least one flag set/unset
								NMessage::EFlags not_set = NMessage::eNone) const
		{ return (IsFullOpen() ? mOpenInfo->mMessages->AnyFlags(set, not_set) : false); }

	CMessage*	GetPrevMessage(CMessage* aMsg,
								bool sorted = false,
								bool cache = false);			// Get previous message
	CMessage*	GetNextMessage(CMessage* aMsg,
								bool sorted = false,
								bool cache = false);			// Get next message
	CMessage*	GetNextFlagMessage(CMessage* aMsg,
									NMessage::EFlags set_flag,
									NMessage::EFlags unset_flag,
									bool sorted = false,
									bool ascending_seq = true,
									bool cache = false);		// Get next message with flag set/unset
	unsigned long	GetMessageIndex(const CMessage* aMsg,		// Get index of message
								bool sorted = false) const
		{ return (sorted) ?
			mOpenInfo->mSortedMessages->FetchIndexOf(aMsg) :
			mOpenInfo->mMessages->FetchIndexOf(aMsg); }

	void	ReadCache(CMessage* msg);							// Get the specified message cache
	void	ReadSize(CMessage* msg);							// Get the specified message size

	void	ReadHeader(unsigned long msg_num,
						bool sorted = false);					// Get the specified message header
	void	ReadHeader(CMessage* msg);							// Get the specified message header

	void	ReadAttachment(unsigned long msg_num,
							CAttachment* attach,
							LStream* aStream,
							bool peek = false,
							bool sorted = false);				// Get the specified message attachment
	void	ReadAttachment(CMessage* msg,
							CAttachment* attach,
							LStream* aStream,
							bool peek = false);					// Get the specified message attachment

	void	CopyAttachment(unsigned long msg_num,
							CAttachment* attach,
							costream* aStream,
							bool peek = false,
							unsigned long count = 0,
							unsigned long start = 1,
							bool sorted = false);				// Get the specified message attachment
	void	CopyAttachment(CMessage* msg,
							CAttachment* attach,
							costream* aStream,
							bool peek = false,
							unsigned long count = 0,
							unsigned long start = 1);			// Get the specified message attachment

	void	RemoveMessage(unsigned long msg_num);				// Remove the specified message
	void	RemoveMessageUID(unsigned long uid);				// Remove the specified message

	void	RemapUID(unsigned long local_uid,					// Set message UID from local UID
							unsigned long new_uid);
	void	MapLocalUIDs(const ulvector& uids,					// Map from local to remote uids
							ulvector* missing = NULL,
							ulmap* local_map = NULL);

	void	ReadMessageFlags(CMessage* msg);					// Get the specified message flags
	void	SetFlagMessage(unsigned long msg_num,
							bool uids,
							NMessage::EFlags flags,
							bool set);							// Change the specified message flags
	void	SetFlagMessage(const ulvector& nums,
							bool uids,
							NMessage::EFlags flags,
							bool set,
							bool sorted = false);				// Change specified message flags

	void	MatchMessageFlags(NMessage::EFlags set_flag,		// Find messages with matching flags set/unset
								NMessage::EFlags unset_flag,
								ulvector& matched,
								bool uids,
								bool sorted = false) const;

	void	PuntUnseen();										// Punt unseen messages

	void	CopyMbox(CMbox* mbox_to);							// Copy this mbox to another mbox

	void	CopyMessage(unsigned long msg_num,
						bool uids,
						CMbox* mbox_to);						// Copy the specified message to another mbox
	void	CopyMessage(const ulvector& nums,
						bool uids,
						CMbox* mbox_to,
						ulmap& copy_uids,
						bool sorted = false);					// Copy the specified sequence to another mbox

	void	CopyMessage(unsigned long msg_num,
						bool uids,
						costream* aStream,
						bool sorted = false);					// Get the specified message
	void	CopyMessage(CMessage* msg, costream* aStream);		// Get the specified message

	void	ExpungeMessage(unsigned long msg_num,
							bool uids);							// Expunge the specified message
	void	ExpungeMessage(const ulvector& nums,
							bool uids,
							bool sorted = false);				// Expunge the specified sequence

	void	SynchroniseMessage(const ulvector& nums,
							bool uids,
							bool full,
							bool partial,
							unsigned long size,
							bool sorted = false);				// Synchonise the specified sequence
	void	ClearDisconnectMessage(const ulvector& nums,
							bool uids,
							bool sorted = false);				// Clear disconnected cache of the specified sequence

	void	MapSorted(ulvector& actual_nums,
						const ulvector& nums,
						bool sorted);							// Map sorted message positions to message numbers

	void	MapUIDs(const ulvector& nums,						// Map sorted message numbers to uids
						ulvector& uids,
						bool sorted = false);
	
	void	GetUIDMap(ulmap& map) const;						// Map UIDs to sequence numbers

protected:
	virtual CMessage* CreateMessage();							// Create message suitable for this mailbox
};

#endif
