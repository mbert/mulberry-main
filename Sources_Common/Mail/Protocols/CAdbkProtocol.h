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


// CAdbkProtocol

#ifndef __CADBKPROTOCOL__MULBERRY__
#define __CADBKPROTOCOL__MULBERRY__

// CAdbkProtocol: Handles quotas for all resources

#include "CINETProtocol.h"
#include "CAdbkRecord.h"
#include "CAddressAccount.h"
#include "CAddressBook.h"

// Classes
class CACL;
class CAdbkClient;
class CAddressBook;

class CAdbkProtocol : public CINETProtocol
{
	friend class CAddressBook;
	friend class CRemoteAddressBook;
	friend class CAdbkRecord;

public:
	// Flags
	enum EAdbkFlags
	{
		eHasACL					= 1L << 16,
		eACLDisabled			= 1L << 17,
		eHasSync				= 1L << 18,
		eDidSyncTest			= 1L << 19
	};

	// Messages for broadcast
	enum
	{
		eBroadcast_ClearList = 'apcl',			// param = CAdbkProtocol*
		eBroadcast_RefreshList = 'aprl',		// param = CAdbkProtocol*
		eBroadcast_ClearSubList = 'apcs',		// param = CAddressBook*
		eBroadcast_RefreshSubList = 'aprs',		// param = CAddressBook*
		eBroadcast_RefreshNode = 'aprn'			// param = CAddressBook*
	};

			CAdbkProtocol(CINETAccount* account);
			CAdbkProtocol(const CAdbkProtocol& copy,
							bool force_local = false,
							bool force_remote = false);
	virtual ~CAdbkProtocol();

	virtual void	CreateClient();
	virtual void	CopyClient(const CINETProtocol& copy);
	virtual void	RemoveClient();

	virtual const CAddressAccount* GetAddressAccount() const
		{ return static_cast<const CAddressAccount*>(GetAccount()); }
	virtual CAddressAccount* GetAddressAccount()
		{ return static_cast<CAddressAccount*>(GetAccount()); }

	bool IsLocalAdbk() const
	{
		return (GetAccount()->GetServerType() == CINETAccount::eLocalAdbk) ||
				(GetAccount()->GetServerType() == CINETAccount::eOSAdbk);
	}
	bool IsRemoteAdbk() const
	{
		return (GetAccount()->GetServerType() == CINETAccount::eIMSP) ||
				(GetAccount()->GetServerType() == CINETAccount::eACAP);
	}
	bool IsReadComponentAdbk() const
	{
		return GetAccount()->GetServerType() == CINETAccount::eCardDAVAdbk;
	}
	bool IsWriteComponentAdbk() const
	{
		// IMSP, ACAP, CardDAV, OS X allow individual addresses to be set
		return (GetAccount()->GetServerType() == CINETAccount::eIMSP) ||
				(GetAccount()->GetServerType() == CINETAccount::eACAP) ||
				(GetAccount()->GetServerType() == CINETAccount::eCardDAVAdbk) ||
				(GetAccount()->GetServerType() == CINETAccount::eOSAdbk);
	}
	bool IsACLAdbk() const
	{
		return GetHasACL();
	}

	char GetDirDelim() const
	{
		return mDirDelim;
	}

	CAddressBook* GetStoreRoot()
	{
		return &mStoreRoot;
	}
	CAddressBook* GetNode(const cdstring& adbk, bool parent = false) const;
	CAddressBook* GetParentNode(const cdstring& adbk) const;

	// Account
	virtual void SetAccount(CINETAccount* account);
	virtual void DirtyAccount();

	// Offline
			bool	IsDisconnectedCache() const
			{
				return IsDisconnected() || (mCacheClient != NULL);
			}
	virtual void	InitDisconnect();
	virtual const cdstring& GetOfflineDefaultDirectory();
	virtual void	GoOffline();
	virtual void	GoOnline();
			void	SynchroniseRemoteAll(bool fast);
			bool	DoPlayback();
	virtual void	SetSynchronising();						// Set into synchronising mode
			bool	HasDisconnectedAdbks();

	virtual void	Open();								// Open connection to protocol server
	virtual void	Close();							// Close connection to protocol server
	virtual void	Logon();							// Logon to protocol server
	virtual void	Logoff();							// Logoff from protocol server

	virtual const char* GetUserPrefix() const;

	// Handle adbks
	
	// A D B K L I S T
			void	LoadList();									// Load adbk list from server
			void	LoadSubList(CAddressBook* adbk, bool deep);	// Load calendar list from server
			void	SyncList();									// Sync adbks with prefs options
			void	SyncSubList(CAddressBook* adbk);			// Sync adbks with prefs options
			void	RefreshList();								// Refresh adbk list from server
			void	RefreshSubList(CAddressBook* adbk);			// Refresh calendar list from server
			void	ListChanged();								// List was changed in some way

	// A D B K S
			void	AddAdbk(CAddressBook* adbk);			// Add adbk to protocol list
			void	CreateAdbk(CAddressBook* adbk);			// Create a new adbk
			void	TouchAdbk(CAddressBook* adbk);			// Touch adbk
			bool	TestAdbk(CAddressBook* adbk);			// Test adbk
			void	DeleteAdbk(CAddressBook* adbk);			// Delete adbk
			void	RenameAdbk(CAddressBook* adbk,
								const char* adbk_new);		// Rename adbk

			void	OpenAdbk(CAddressBook* adbk);			// Open adbk on server
			void	SyncFromServer(CAddressBook* adbk);
			void	SyncFullFromServer(CAddressBook* adbk);
			void	SyncComponentsFromServer(CAddressBook* adbk);
			void	SyncComponentsFromServerSlow(CAddressBook* adbk);
			void	SyncComponentsFromServerFast(CAddressBook* adbk);
			void	CloseAdbk(CAddressBook* adbk);			// Close existing mailbox
			void	SizeAdbk(CAddressBook* adbk);

			void	ReadFullAddressBook(CAddressBook* adbk);
			void	WriteFullAddressBook(CAddressBook* adbk);

			void	AddAddress(CAddressBook* adbk,
								CAddressList* addrs);		// Add addresses
			void	AddGroup(CAddressBook* adbk,
								CGroupList* grps);			// Add groups
			void	ChangeAddress(CAddressBook* adbk,
								CAddressList* addrs);		// Change addresses
			void	ChangeGroup(CAddressBook* adbk,
								CGroupList* grps);			// Change groups
			void	RemoveAddress(CAddressBook* adbk,
								CAddressList* addrs);		// Remove addresses
			void	RemoveGroup(CAddressBook* adbk,
								CGroupList* grps);			// Remove groups
			void	RemovalOfAddress(CAddressBook* adbk);	// Address/Group was removed

			void	ResolveAddress(CAddressBook* adbk,
									const char* nick_name,
									CAdbkAddress*& addr);	// Resolve address nick-name
			void	ResolveGroup(CAddressBook* adbk,
									const char* nick_name,
									CGroup*& grp);			// Resolve group nick-name

			void	SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list);	// Do search

	// Status of ACL support
	void SetHasACL(bool has_acl)
		{ mFlags.Set(eHasACL, has_acl); }
	bool GetHasACL() const
		{ return mFlags.IsSet(eHasACL); }
	void SetACLDisabled(bool disabled)
		{ mFlags.Set((EFlags) eACLDisabled, disabled); }
	bool GetACLDisabled() const
		{ return mFlags.IsSet(eACLDisabled); }
	bool UsingACLs() const							// Indicates that client can use ACLs
		{ return GetHasACL() && !GetACLDisabled(); }

	// ACL commands
	void SetACL(CAddressBook* adbk, CACL* acl);			// Set acl on server
	void DeleteACL(CAddressBook* adbk, CACL* acl);		// Delete acl on server
	void GetACL(CAddressBook* adbk);					// Get all acls for mailbox from server
	void MyRights(CAddressBook* adbk);					// Get current user's rights to mailbox
	
	// Sync'ing
	void SetHasSync(bool has_sync)
		{ mFlags.Set(eHasSync, has_sync); mFlags.Set(eDidSyncTest, true); }
	bool GetHasSync() const
		{ return mFlags.IsSet(eHasSync); }
	bool GetDidSyncTest() const
		{ return mFlags.IsSet(eDidSyncTest); }
	
protected:
	CAdbkClient*		mClient;							// Its client
	CAdbkClient*		mCacheClient;						// Its client
	bool				mCacheIsPrimary;					// Use the cache client ahead of the main one
	CAddressBook		mStoreRoot;
	char				mDirDelim;							// Directory delimiter
	bool				mSyncingList;
	bool				mListedFromCache;

	CAdbkRecord*		mRecorder;							// Recorder for mail ops

	// Full write operations
	bool DoWriteFull(CAddressBook* adbk);
	void DidComponentWrite(CAddressBook* adbk);

	// Offline
			void DumpAddressBooks();
			bool ReadAddressBooks(bool only_if_current=false);
			void RecoverAddressBooks();
	
			void SynchroniseRemote(CAddressBook* adbk, bool fast);
			void SyncRemote(CAddressBook* remote, CAddressBook* local, bool fast);

			void ClearDisconnect(CAddressBook* adbk);

};

#endif
