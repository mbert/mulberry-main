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


// Header for CMboxProtocol class

#ifndef __CMBOXPROTOCOL__MULBERRY__
#define __CMBOXPROTOCOL__MULBERRY__

#include "CINETProtocol.h"

#include "CMailAccount.h"
#include "CMboxFwd.h"
#include "CMboxList.h"
#include "CMboxRefList.h"
#include "CMessageFwd.h"
#include "CMessageListFwd.h"
#include "CQuotas.h"
#include "CStreamTypeFwd.h"

#include "SBitFlags.h"

#include "cdstring.h"
#include "templs.h"

// Consts

// Types
typedef std::vector<CMboxList*> CHierarchies;

// Classes
class CACL;
class CAttachment;
class CDisplayItem;
class CEnvelope;
class CMailAccount;
class CMailRecord;
class CMbox;
class CMboxClient;
class CMessage;
class CProgress;
class CSearchItem;

class LStream;

class CMboxProtocol : public CINETProtocol
{
	friend class CMbox;
	friend class CIMAPClient;
	friend class CPOP3Client;
	friend class CLocalClient;
	friend class CMailRecord;

public:
	// Flags
	enum EMboxFlags
	{
		eShowSubscribed			= 1L << 16,
		eShowUnsubscribed		= 1L << 17,
		eWasUpdated				= 1L << 18,
		eHasACL					= 1L << 19,
		eACLDisabled			= 1L << 20,
		eHasQuota				= 1L << 21,
		eQuotaDisabled			= 1L << 22,
		eHasDisconnected		= 1L << 23
	};

	// Items that can be used in an IMAP FETCH
	enum EFetchItems
	{
		eFetchNone	= 0,
		eSummary	= 1L << 0,
		eEnvelope	= 1L << 1,
		eUID		= 1L << 2,
		eSize		= 1L << 3,
		eFlags		= 1L << 4
	};

	// Messages for broadcast
	enum
	{
		eBroadcast_BeginListChange = 'mpbl',
		eBroadcast_EndListChange = 'mpel',
		eBroadcast_BeginListUpdate = 'mpbu',
		eBroadcast_EndListUpdate = 'mpeu',
		eBroadcast_MailLogon = 'mpln',
		eBroadcast_MailLogoff = 'mplf',
		eBroadcast_MailMboxLogoff = 'mplo',
		eBroadcast_NewList = 'mpnl',
		eBroadcast_RemoveList = 'mprl'
	};

	enum ENamespace
	{
		ePersonal = 0,
		eShared = 1,
		ePublic
	};
	struct SNamespace
	{
		cdstrpairvect mItems[3];
		size_t size() const
			{ return mItems[0].size() + mItems[1].size() + mItems[2].size(); }
		size_t offset(int index)
			{ int j = 0; for(int i = 0; i < index; i++) j += mItems[i].size(); return j; }
	};

					CMboxProtocol(CMailAccount* account);
					CMboxProtocol(const CMboxProtocol& copy,
									bool force_local = false,
									bool force_remote = false);
	virtual 		~CMboxProtocol();

	virtual void	CreateClient();
	virtual void	CopyClient(const CINETProtocol& copy);
	virtual void	RemoveClient();
	virtual CINETProtocol*	CloneConnection();				// Create duplicate, empty connection

	virtual void	InitProtocol();							// Set up default mboxes etc
	
	virtual void	Open();									// Open connection to protocol server

	virtual const CMailAccount* GetMailAccount() const
		{ return static_cast<const CMailAccount*>(GetAccount()); }
	virtual CMailAccount* GetMailAccount()
		{ return static_cast<CMailAccount*>(GetAccount()); }

	// Account
	virtual const CINETAccount* GetAccount() const
		{ return IsCloned() ? mCloneOwner->GetAccount() : CINETProtocol::GetAccount(); }
	virtual CINETAccount* GetAccount()
		{ return IsCloned() ? const_cast<CMboxProtocol*>(mCloneOwner)->GetAccount() : CINETProtocol::GetAccount(); }
	virtual const cdstring& GetAccountName() const
		{ return IsCloned() ? mCloneOwner->GetAccountName() : CINETProtocol::GetAccountName(); }
	virtual CINETAccount::EINETServerType GetAccountType() const
		{ return IsCloned() ? mCloneOwner->GetAccountType() : CINETProtocol::GetAccountType(); }
	virtual const cdstring& GetAccountUniqueness() const
		{ return IsCloned() ? mCloneOwner->GetAccountUniqueness() : CINETProtocol::GetAccountUniqueness(); }
	virtual void SetAccount(CINETAccount* account);

	virtual const cdstring& GetAuthenticatorUniqueness() const
		{ return IsCloned() ? mCloneOwner->GetAuthenticatorUniqueness() : CINETProtocol::GetAuthenticatorUniqueness(); }
	virtual void SetAuthenticatorUniqueness(const cdstring& uniqueness)
		{ if (!IsCloned()) CINETProtocol::SetAuthenticatorUniqueness(uniqueness); }

	// WD processing
			void	InitNamespace();								// Initialise namespace
			void	AutoNamespace();								// Automate namespace descovery
			bool	FlatWD() const									// Indicates single WD is flat
		{ return (GetHierarchies().size() == 2); }
			bool	AlwaysFlatWD() const;							// Indicates WD is always flat
			void	SyncWDs();										// Set the working directory prefixes
			void	AddWD(const CDisplayItem& wd,
							char dir_delim);						// Add the working directory prefix
			void	RenameWD(unsigned long pos,
								const cdstring& new_name);			// Rename the working directory
			void	RemoveWD(unsigned long pos);					// Remove the working directory prefix
			void	MoveWD(unsigned long old_pos,
							unsigned long new_pos);					// Move the working directory prefix
			void	SetFlagWD(CMboxList* list, CTreeNodeList::ETreeNodeListFlags flag, bool set);
			CMboxList*	GetINBOXRootWD() const;						// Get WD that is the INBOX root

	// Setters/Getters
			CMbox*	GetINBOX()
		{ return mINBOX; }
			CHierarchies& GetHierarchies()
		{ return mHierarchies; }
			const CHierarchies& GetHierarchies() const
		{ return mHierarchies; }
			long GetHierarchyIndex(const CMboxList* list) const;

			bool	ShowSubscribed() const
		{ return mFlags.IsSet(eShowSubscribed); }
			void	SetShowSubscribed(bool show, bool clear = true);
			bool	ShowUnsubscribed() const
		{ return mFlags.IsSet(eShowUnsubscribed); }
			void	SetShowUnsubscribed(bool show, bool clear = true);
	
			CMbox* GetCurrentMbox() const
		{ return mCurrent_mbox; }
			void	ClearCurrentMbox()
		{ mCurrent_mbox = NULL; }

	// Searching
			CMbox*	FindMbox(const char* mbox,							// Return mbox with this name
								bool to_open = false) const;
			CMbox*	FindMboxAccount(const char* acct) const;			// Return mbox with this account name
			void	FindMboxAccount(const char* acct,					// Return all mboxes with this account name
									CMboxList& list,
									bool no_dir = false) const;
			CMbox*	FindMboxURL(const char* url) const;					// Return mbox with this url
			void	FindMbox(const char* mbox,
								CMboxList& list,
								bool no_dir = false) const;				// Return all mboxes with this name
			CMbox*	FindOpenMbox(const char* mbox) const;				// Return mbox with this name that is open
			void	FindWildcard(const cdstring& pattern,
									CMboxList& list,
									bool dynamic,
									bool no_dir = false);				// Return all mboxes matching pattern
			CMboxList*	FindMatchingList(const CMbox* mbox) const;		// Find search hierarchy that matches mbox
			bool		FindRoot(const cdstring& mbox_name, CMboxList*& root_list, CMbox*& root_mbox);

			unsigned long FetchIndexOf(const CMboxList* list) const;
			unsigned long FetchIndexOf(const CMbox* mbox,
										bool adjust_single = false) const;
			void	 FetchIndexOf(const CMbox* mbox,
										ulvector& indices,
										unsigned long offset,
										bool adjust_single = false) const;
			unsigned long CountHierarchy(bool adjust_single = false) const;	// Count all items in visual hierarchy

	// Check state

			bool	IsCloned() const
		{ return (mCloneOwner != nil); }
			const CMboxProtocol* GetCloneOwner() const
		{ return mCloneOwner; }

			bool UsingACLs() const							// Indicates that client can use ACLs
		{ return GetHasACL() && !GetACLDisabled(); }

			bool UsingQuotas() const						// Indicates that client can use quotas
		{ return GetHasQuota() && !GetQuotaDisabled(); }

			void	ClearCache();						// Delete all mbox cache info

	// L O G I N  &  L O G O U T
	virtual void	Close();							// Close connection to protocol server
	virtual void	Logon();							// Logon to protocol server
	virtual void	Logoff();							// Logoff from protocol server
	virtual void	Forceoff();							// Forced close

			void	RecoverClone();							// Recover a cloned connection

	// O F F L I N E
	virtual void	InitDisconnect();
	virtual const cdstring& GetOfflineDefaultDirectory();
	virtual void	GoOffline(bool force, bool sync_list);
	virtual void	GoOnline(bool updating_only = false);
			bool	DoPlayback(CProgress* progress);
	virtual void	SetSynchronising();						// Set into synchronising mode


	// M B O X L I S T
			void	SetSorted(bool keep_sorted);				// Keep mbox list sorted
			void	Sort();									// Sort list now
			void	Refresh();								// Force refresh of entire list
			void	ClearList();							// Clear mbox list after account change
			void	LoadList(bool deep = false);			// Load mbox list from server
			void	LoadRemoteList(bool deep = false);
			void	LoadSubList(const CMbox* mbox,
								bool deep);					// Load mbox sublist from server
			void	LoadRemoteSubList(const CMbox* mbox,
								bool deep);					// Load mbox sublist from server
			void	LoadMbox(CMboxList* list,
								const cdstring& mbox_name);	// Load named mbox from server
			void	DeepSubList(CMboxList* mbox);			// Deep expand mailbox list

			void	SyncMbox();								// Reset various mailbox flags
			void	SyncFlag(const cdstrpairvect& mboxes,	// Reset mailbox flag
								NMbox::EFlags flag);

			void	AddSingleton(CMbox* mbox);				// Add singleton mailbox
			void	RemoveSingleton(CMbox* mbox);			// Remove singleton mailbox

			void	AutoCreate(const cdstring& mbox_name,	// Do possible auto create of this mailbox
								bool subscribe);

protected:
	CMbox*				mINBOX;								// INBOX
	CHierarchies		mHierarchies;						// List of lists (first is subscribed)
	CMboxList			mSingletons;						// List of mailboxes that don't belong in a heirarchy and will delete on close
	const CMboxProtocol* mCloneOwner;						// Owner of this clone (never touched hence const)
	CMbox*				mCurrent_mbox;						// Currently selected mbox
	CMboxClient*		mClient;							// The client
	CQuotaRootList		mRoots;								// List of quota roots known about on this server
	
	CMailRecord*		mRecorder;							// Recorder for mail ops

			void	FindAllSubsMbox(CMboxList* mboxes);		// Find subscribed mboxes
			void	FindAllMbox(CMboxList* mboxes);			// Find all mboxes for specific hierarchy

	// M B O X
			CMbox* 	AddMbox(CMbox* mbox);					// Add mbox to list

			void	CreateMbox(CMbox* mbox);				// Create a new mbox
			void	TouchMbox(CMbox* mbox);					// Touch mbox
			bool	TestMbox(CMbox* mbox);					// Test mbox
			void	RebuildMbox(CMbox* mbox);				// Rebuild mbox cache
			void	OpenMbox(CMbox* mbox,
								bool update = true,
								bool examine = false);		// Open mailbox on server
			void	OpeningMbox(CMbox* mbox);				// Mailbox about to be opened elsewhere
			void	CloseMbox(CMbox* mbox);					// Close existing mailbox

			void	SetUIDValidity(unsigned long uidv);		// Set UID validity
			void	SetUIDNext(unsigned long uidn);			// Set UID next
			void	SetLastSync(unsigned long sync);		// Set last sync time
			void	GetLastSync(CMbox* mbox);				// Get last sync of remote mailbox

			void	SetCurrentMbox(CMbox* mbox,
									bool update,
									bool examine);			// Make mbox current
			void	ForgetMbox(CMbox* mbox);				// Deselect mbox

			void	UpdateMbox(CMbox* mbox);				// Read in new envelopes

			long	CheckMbox(CMbox* mbox,					// Check mbox
								bool fast = false);
			void	ProcessCheckMbox(CMbox* mbox);			// Process a mailbox check change

			void	MailboxSize(CMbox* mbox);				// Read in size
			bool	DoesMailboxSize() const;				// Does server do size?

			void	SearchMbox(CMbox* mbox,					// Search for messages in mbox
								const CSearchItem* spec,
								ulvector* results,
								bool uids);

			void	ExpungeMbox(CMbox* mbox, bool closing);	// Clear (EXPUNGE) mbox

			void	DeleteMbox(CMbox* mbox, bool remove = true);	// Delete mbox
			void	RemoveMbox(CMbox* mbox);						// Remove mbox from list

			void	RenameMbox(CMbox* mbox,
								const char* mbox_new);		// Rename mbox

			void	SubscribeMbox(CMbox* mbox,
									bool update = true);	// Subscribe mbox

			void	UnsubscribeMbox(CMbox* mbox,
									bool update = true);	// Unsubscribe mbox

			void	StartAppend(CMbox* mbox);				// Append message to mbox
			void	StopAppend(CMbox* mbox);				// Append message to mbox
			void	AppendMbox(CMbox* mbox,					// Append message to mbox
								CMessage* theMsg,
								unsigned long& new_uid,
								bool dummy_files = false,
								bool doMRU = true);

	// M E S S A G E S
	//virtual CMessage*	GetMessage(unsigned long msg_num)			// Get message
	//	{return (mCurrent_mbox ? mCurrent_mbox->GetMessage(msg_num) : nil);}

			unsigned long GetMessageLocalUID(unsigned long uid);

			void	FetchItems(const ulvector& nums,
									bool uids,
									EFetchItems items);		// Do fetch envelopes
			void	ReadHeaders(const ulvector& nums,
									bool uids,
									const cdstring& hdrs);	// Get messages header text from server
			void	ReadHeader(CMbox* mbox,
								CMessage* msg);				// Get message header text from server
			void	ReadAttachment(CMbox* mbox,
								unsigned long msg_num,
								CAttachment* attach,
								LStream* aStream,
								bool peek = false,
								unsigned long count = 0,
								unsigned long start = 1);			// Get attachment data into stream
			void	CopyAttachment(CMbox* mbox,
								unsigned long msg_num,
								CAttachment* attach,
								costream& aStream,
								bool peek = false,
								unsigned long count = 0,
								unsigned long start = 1);			// Copy raw attachment data into stream

			void	RemapUID(CMbox* mbox,
									unsigned long local_uid,
									unsigned long new_uid);

			void	MapLocalUIDs(CMbox* mbox,						// Map local to remote UIDs
									const ulvector& uids,
									ulvector* missing,
									ulmap* local_map = NULL);

			void	SetFlagMessage(CMbox* mbox,
									const ulvector& nums,
									bool uids,
									NMessage::EFlags flags,
									bool deleted);			// Change flags

			void	CopyMessage(CMbox* mbox_from,			// Copy message to mailbox
								const ulvector& nums,
								bool uids,
								CMbox* mbox_to,
								ulmap& copy_uids,
								bool doMRU = true);

			void	CopyMessage(CMbox* mbox,
								unsigned long msg_num,
								bool uids,
								costream& aStream,
								unsigned long count = 0,
								unsigned long start = 1);			// Copy message to stream
			bool	DoesCopy() const;						// Does server do copies?

			void	ExpungeMessage(const ulvector& nums, bool uids);		// Do message expunge
			bool	DoesExpungeMessage() const;					// Does server handle copy?

	// Sort/thread support
			bool	DoesSort(ESortMessageBy sortby) const;				// Does server-side sorting
			void    Sort(ESortMessageBy sortby,						// Do server-side sort
							EShowMessageBy show_by,
							const CSearchItem* search,
							ulvector* results,
							bool uids);
	
			bool	DoesThreading(EThreadMessageBy threadby) const;	// Does server-side threading
			void    Thread(EThreadMessageBy threadby,					// Do server-side thread
							const CSearchItem* search,
							threadvector* results,
							bool uids);

	// Status of ACL support
			void SetHasACL(bool has_acl)
		{ mFlags.Set(eHasACL, has_acl); }
			bool GetHasACL() const
		{ return mFlags.IsSet(eHasACL); }
			void SetACLDisabled(bool disabled)
		{ mFlags.Set(eACLDisabled, disabled); }
			bool GetACLDisabled() const
		{ return mFlags.IsSet(eACLDisabled); }

	// ACL commands
			void SetACL(CMbox* mbox, CACL* acl);			// Set acl on server
			void DeleteACL(CMbox* mbox, CACL* acl);			// Delete acl on server
			void GetACL(CMbox* mbox);						// Get all acls for mailbox from server
			void ListRights(CMbox* mbox, CACL* acl);		// Get allowed rights for user
			void MyRights(CMbox* mbox);						// Get current user's rights to mailbox

	// Status of Quota support
			void SetHasQuota(bool has_quota)
		{ mFlags.Set(eHasQuota, has_quota); }
			bool GetHasQuota() const
		{ return mFlags.IsSet(eHasQuota); }
			void SetQuotaDisabled(bool disabled)
		{ mFlags.Set(eQuotaDisabled, disabled); }
			bool GetQuotaDisabled() const
		{ return mFlags.IsSet(eQuotaDisabled); }

	// Quota commands
			void AddQuotaRoot(CQuotaRoot* root);			// Add unique quotaroot
			CQuotaRoot* FindQuotaRoot(const char* txt);		// Find quotaroot from title

			void SetQuota(CQuotaRoot* root);				// Set new value on server
			void GetQuota(CQuotaRoot* root);				// Get value from server
			void GetQuotaRoot(CMbox* mbox);					// Get quota roots for a mailbox

	// Other flags
			void SetHasDisconnected(bool has_disconnected)
		{ mFlags.Set(eHasDisconnected, has_disconnected); }
			bool GetHasDisconnected() const
		{ return mFlags.IsSet(eHasDisconnected); }

	// Offline
			void DumpMailboxes(bool full_sync);
			void ReadMailboxes();
			void SwitchMailboxes(CMboxProtocol* local);

			void SynchroniseRemote(CMbox* mbox, bool fast, bool partial, unsigned long size, const ulvector& uids);
			void SyncRemote(CMbox* remote, CMbox* local, bool fast, bool partial, unsigned long size, const ulvector& uids);

			void ClearDisconnect(CMbox* mbox, const ulvector& uids);
		
			void GetDisconnectedMessageState(CMbox* remote, ulvector& full, ulvector& partial);
};

#endif
