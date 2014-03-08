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


// CMailAccountManager.h

// Header file for CMailAccountManager class that will handle all mailbox accounts

#ifndef __CMAILACCOUNTMANAGER__MULBERRY__
#define __CMAILACCOUNTMANAGER__MULBERRY__

#include "CBroadcaster.h"

#include "CIdentity.h"
#include "CMailAccount.h"
#include "CMboxFwd.h"
#include "CMboxProtocol.h"
#include "CMboxRefList.h"

#include <vector>
#include "templs.h"

#include <time.h>

// Classes

class CMailAccount;
class CMailNotification;
class CMbox;
class CMboxList;
class CMboxRef;
class CProgress;
class CServerView;
class CServerWindow;

typedef std::vector<CMboxProtocol*> CMboxProtocolList;
typedef std::vector<CMboxRefList*> CFavourites;

class CMailAccountManager : public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_BeginMailAccountChange = 'mabc',
		eBroadcast_EndMailAccountChange = 'maec',
		eBroadcast_NewMailAccount = 'mana',
		eBroadcast_RemoveMailAccount = 'mara',
		eBroadcast_NewFavourite = 'manf',
		eBroadcast_RemoveFavourite = 'marf',
		eBroadcast_MRUCopyToChange = 'mamc',
		eBroadcast_MRUAppendToChange = 'mama',
		eBroadcast_ChangedMbox = 'macm',
		eBroadcast_RefreshAll = 'maal'
	};

	// Indices for built in favourites
	enum EFavourite
	{
		eFavouriteStart = 0,
		eFavouriteAutoCheck = 0,
		eFavouriteOpenLogin,
		eFavouriteCopyTo,
		eFavouriteAppendTo,
		eFavouritePuntOnClose,
		eFavouriteAutoSync,
		eFavouriteNew,
		eFavouriteSubscribed,
		eFavouriteOthers
	};

	static CMailAccountManager* sMailAccountManager;

	CMailAccountManager();
	~CMailAccountManager();
	
	void CreateMainView();
	CServerView* GetMainView() const
		{ return mMainView; }
	void SetMainView(CServerView* view)
		{ mMainView = view; }

	CMboxProtocol* GetProtocol(const cdstring& name);		// Get protocol by name
	CMboxProtocolList& GetProtocolList()					// Get protocol list
		{ return mProtos; }
	unsigned long GetProtocolCount() const					// Number of protocols
		{ return mProtoCount; }

	CMboxProtocol* GetReusableProto() const
		{ return mReusableProto; }
	void SetReusableProto(CMboxProtocol* proto)
		{ mReusableProto = proto; }
	bool ReuseProto() const
		{ return mReuseProto; }

	// Find related items
	CServerWindow* GetProtocolWindow(const CMboxProtocol* proto) const;	// Get protocol window

	CMbox*	FindMbox(const char* mbox) const;				// Return mbox with this name
	CMbox*	FindMboxAccount(const char* url) const;			// Return mbox with this account name
	CMbox*	FindMboxURL(const char* url) const;				// Return mbox with this url

	void	FindWildcard(const cdstring& name,
							CMboxList& mboxes,
							bool dynamic,
							bool no_dir = false) const;		// Return matching mboxes

	CMbox*	GetNewMailbox(const CMbox* mbox, bool reverse = false);	// Get mailbox with new messages

	long GetProtocolIndex(const CMboxProtocol* proto) const;	// Get index of protocol

	// Visual position of items
	unsigned long FetchIndexOf(const CMboxProtocol* proto,
								bool adjust_single = false) const;
	unsigned long FetchIndexOf(const CMboxRefList* favourite) const;
	unsigned long FetchSiblingIndexOf(const CMboxRefList* favourite) const;
	unsigned long FetchServerOffset() const;
	unsigned long FetchIndexOf(const CMbox* mbox,
								bool adjust_single = false) const;
	void FetchIndexOf(const CMbox* mbox, ulvector& indices, unsigned long offset,
								bool adjust_single = false) const;
	void FetchFavouriteIndexOf(const CMbox* mbox, ulvector& indices) const;

	// Deal with accounts
	void SyncAccounts(const CMailAccountList& accts);	// Sync with changed accounts
	void AddProtocol(CMboxProtocol* proto);				// Add a new mbox protocol
	void ChangedProtocolWDs(CMboxProtocol* proto);		// Changed an mbox protocol's WDs
	void RemoveProtocol(CMboxProtocol* proto, bool full = false);			// Remove an mbox protocol

	bool HasOpenProtocols() const;					// Determine if any open protocols

	void OpenProtocol(CMboxProtocol* proto);			// Open protocol
	void StartProtocol(CMboxProtocol* proto, bool silent = true);			// Start protocol
	void StartedProtocol(CMboxProtocol* proto);			// Started protocol
	void StopProtocol(CMboxProtocol* proto);			// Stop protocol

	void MoveProtocol(long old_index, long new_index);	// Move protocol

	void InitNamespace(CMboxProtocol* proto, CMboxProtocol::SNamespace& wds);

	// Mailbox checking/notification
	void	SpendTime();							// Called during idle
	void	Reset();								// Reset check timer
	bool	CheckTimer();							// Check timer timeout
	void	Pause(bool pauseIt)
						{ mCheckPaused = pauseIt;
						  if (!mCheckPaused) Reset(); }		// Set check paused state
	bool	IsPaused() const { return mCheckPaused; }		// Return check paused state
	void	ForceMailCheck();								// Force check of new mail
	bool	DoMailCheck(bool allow_alert,					// Check for new mail
						bool automatic = false);
	long	RunMailCheck(EFavourite type,					// Do check of favourite
							const CMailNotification& notifier,
							bool silent);

	// Favourites
	void	InitFavourites();
	void	UpdateFavourites();
	void	RenameFavourites(const cdstrpairvect& renames);

	CFavourites& GetFavourites()
		{ return mFavourites; }

	EFavourite GetFavouriteType(const CMboxRefList* list) const;
	EFavourite GetFavouriteFromID(const cdstring& id) const;
	cdstring GetFavouriteID(EFavourite type) const;

	bool IsRemoveableFavouriteType(const CMboxRefList* list) const;
	bool IsRemoveableItemFavouriteType(const CMboxRefList* list) const;
	bool IsWildcardFavouriteType(const CMboxRefList* list) const;

	bool AddFavourite(const cdstring& name, unsigned long index = -1, bool update_prefs = true);
	void RenameFavourite(CMboxRefList* list, const cdstring& name, bool update_prefs = true);
	void RemoveFavourite(CMboxRefList* list, bool update_prefs = true);

	// Favourite Items
	bool AddFavouriteItem(EFavourite type, CMbox* mbox)
		{ return AddFavouriteItem(mFavourites.at(type), mbox); }
	bool AddFavouriteItem(CMboxRefList* list, CMbox* mbox);
	bool AddFavouriteItem(CMboxRefList* list, CMboxRef* mboxref);
	
	bool IsFavouriteItem(EFavourite type, const CMbox* mbox) const;
	bool GetFavouriteAutoCheck(const CMbox* mbox, EFavourite& type) const;
	bool GetNotifierIndex(EFavourite type, unsigned long& index) const;

	void RemoveFavouriteItem(EFavourite type, CMbox* mbox)
		{ RemoveFavouriteItem(mFavourites.at(type), mbox); }
	void RemoveFavouriteItem(CMboxRefList* list, CMbox* mbox);
	void RemoveFavouriteItem(CMboxRefList* list, CMboxRef* mboxref);

	void RemoveAllFavouriteItems(EFavourite type)
		{ RemoveAllFavouriteItems(mFavourites.at(type)); }
	void RemoveAllFavouriteItems(CMboxRefList* list);

	bool ProcessFavouriteItem(CMboxRefList* list, CMbox* mbox, bool add, bool act);
	bool ProcessFavouriteItem(CMboxRefList* list, CMboxRef* mbox_ref, bool add, bool act);

	void ChangeFavouriteItem(CMboxNode* mbox, EFavourite type, bool add);
	void RenameFavouriteItem(CMboxRefList* list, CMboxRef* mbox_ref, const cdstring& new_name);

	void SetHierarchic(CMboxList* list, bool hier);
	void SetFlag(CMboxRefList* list, CTreeNodeList::ETreeNodeListFlags flag, bool set);

	void SyncFavourite(const char* acct_name, char dir, EFavourite type, bool add);

	void RenamedMailbox(const cdstring& old_name, const cdstring& new_name);
	void RemovedMailbox(const char* mbox_name);

	// MRU mailboxes
	void InitMRUs();
	void RenameMRUs(const cdstrpairvect& renames);

	void AddMRUCopyTo(CMboxNode* mbox);
	void AddMRUAppendTo(CMboxNode* mbox);
	void CleanMRUHistory(unsigned long max_history);
	const CMboxRefList& GetMRUCopyTo() const
		{ return mMRUCopyTo; }
	const CMboxRefList& GetMRUAppendTo() const
		{ return mMRUAppendTo; }

	// O F F L I N E
	bool	HasDisconnectedProtocols() const;
	void	GoOffline(bool force, bool sync, bool fast, bool partial, unsigned long size, bool sync_list, CProgress* progress);
	void	DoOfflineSync(bool fast, bool partial, unsigned long size, CProgress* progress);
	void	GoOnline(bool playback, bool update, bool do_pop3, CProgress* acct_progress, CProgress* item_progress);

	void	SaveOpenMailboxes();
	void	RestoreOpenMailboxes();

	// Sleep controls
	void	Suspend();
	void	Resume();

private:
	CMboxProtocolList			mProtos;
	unsigned long				mProtoCount;
	CFavourites					mFavourites;
	CMboxRefList				mMRUCopyTo;
	CMboxRefList				mMRUAppendTo;
	CServerView*				mMainView;
	bool						mCheckPaused;		// Paused state
	bool						mHaltCheck;			// Forcibly stop current check
	CMboxProtocol*				mReusableProto;		// Proto to reuse during ops
	bool						mReuseProto;		// Reuse proto during ops
	cdstrvect					mSaveOpen3Pane;		// Save open 3-pane mailboxes
	cdstrvect					mSaveOpen1Pane;		// Save open 1-pane mailboxes
	
	bool	IsFavouriteMboxFlag(EFavourite type) const;
	NMbox::EFlags GetFavouriteMboxFlag(EFavourite type) const;
	bool	IsFavouriteStored(EFavourite type) const;
	bool	IsFavouriteUserDefined(EFavourite type) const;
	
	void	ResolveCabinetList(CMboxList& list, EFavourite type) const;
};

#endif
