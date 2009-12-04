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


// CAddressBookManager.h

// Header file for CAddressBookManager class that will handle all address books,
// local and remote (on multiple servers). This class will be used for global
// nick-name resolution and searches. It can also be serialized to prefs and recovered.

#ifndef __CADDRESSBOOKMANAGER__MULBERRY__
#define __CADDRESSBOOKMANAGER__MULBERRY__

#include "CBroadcaster.h"

#include "CAddressAccount.h"
#include "CAddressBook.h"
#include "ptrvector.h"
#include "cdstring.h"

// Classes

class CAdbkProtocol;
class CAddrLookupProtocol;
class CAddressList;
class CMessage;
class CMessageList;

typedef ptrvector<CAdbkProtocol> CAdbkProtocolList;
typedef ptrvector<CAddrLookupProtocol> CAddrLookupProtocolList;

typedef std::pair<cdstring, CAddressList*> CAddressSearchResult;
typedef std::vector<CAddressSearchResult*> CAddressSearchResultList;

class CAddressBookManager : public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_NewAdbkAccount = 'abna',
		eBroadcast_InsertAdbkAccount = 'abni',
		eBroadcast_RemoveAdbkAccount = 'abra',
		eBroadcast_InsertNode = 'abin',
		eBroadcast_RemoveNode = 'abrn',
		eBroadcast_DeleteNode = 'abdn',
		eBroadcast_ChangedNode = 'abch'
	};

	enum EAddrLookup
	{
		eMulberryLookup = 1,
		eLDAPLookup,
		eWhoisppLookup,
		eFingerLookup
	};

	static CAddressBookManager* sAddressBookManager;

	CAddressBookManager();
	~CAddressBookManager();
	
	void UpdateWindows();
	void SyncAccounts();								// Sync account changes

	void AddProtocol(CAdbkProtocol* proto);				// Add a new adbk remote protocol
	void RemoveProtocol(CAdbkProtocol* proto);			// Remove an adbk remote protocol
	CAdbkProtocol* GetProtocol(const cdstring& name);	// Get protocol by name
	CAdbkProtocolList& GetProtocolList()				// Get protocol list
		{ return mProtos; }
	bool HasMultipleProtocols() const
	{
		return mProtos.size() > 1;
	}
	unsigned long GetProtocolCount() const					// Number of protocols
		{ return mProtoCount; }

	CAddrLookupProtocolList& GetLookupList()				// Get lookup list
		{ return mLookups; }

	bool CanSearch() const									// Check for searchable adbks/lookups
		{ return (mAdbkSearch.size() > 0) || (mLookups.size() > 0); }
	void ClearSearch();										// Clear search results
	void ClearSearchItem(CAddressSearchResult* item);		// Clear search results
	void ClearSearchItemAddress(CAddressSearchResult* item, CAddress* addr);	// Clear search results
	const CAddressSearchResultList& GetSearchResultList() const
		{ return mSearchResultList; }

private:
	void SyncProtos();										// Sync with changed accounts
	void SyncLookups();										// Sync with changed accounts

public:	
	void StartProtocol(CAdbkProtocol* proto, bool silent = true);			// Start protocol
	void UpdateProtocol(CAdbkProtocol* proto);			// Update protocol
	void StopProtocol(CAdbkProtocol* proto);			// Stop protocol

	long GetProtocolIndex(const CAdbkProtocol* proto) const;		// Get index of protocol
	bool FindProtocol(const CAdbkProtocol* proto, unsigned long& pos) const;

	void MoveProtocol(long old_index, long new_index);	// Move protocol

	CAddressBook& GetRoot()
	{
		return mRoot;
	}

	CAddressBookList& GetNodes()
	{
		return *mRoot.GetChildren();
	}

	const CAddressBook* GetNode(const cdstring& adbk) const;

	// Managing the store
	CAddressBook* NewAddressBook(CAdbkProtocol* proto, const cdstring& name, bool directory);
	void RenameAddressBook(CAddressBook* node, const cdstring& new_name);
	void DeleteAddressBook(CAddressBook* node);

	void SyncAddressBook(CAddressBook* adbk, bool add);

	void GetCurrentAddressBookList(cdstrvect& adbks, bool url = true) const;
	
	bool ResolveNickName(const char* nick_name, CAddressList* list);	// Resolve nick-name
	bool ResolveGroupName(const char* grp_name,
							CAddressList* list, short level = 0);		// Resolve group-name

	void SearchAddress(const cdstring& name,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field,
						const cdstring& field_name);					// Do search
	void SearchAddress(const cdstring& name,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field,
						CAddressList& results);							// Do search
	void SearchAddress(const cdstring& name,
						CAdbkAddress::EAddressMatch match,
						const CAdbkAddress::CAddressFields& fields,
						CAddressList& results);							// Do search
	void SearchAddress(EAddrLookup method,
						bool expanding,
						const cdstring& item,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field);				// Do search
	void SearchAddress(EAddrLookup method,
						bool expanding,
						const cdstring& item,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field,
						CAddressList& results);							// Do search

	void ExpandAddresses(const cdstring& expand, cdstrvect& results);	// Do address expansion
	void ExpandCalendarAddresses(const cdstring& expand, cdstrvect& results);	// Do address expansion

	void CaptureAddress(const cdstring& expand);						// Do address capture
	void CaptureAddress(const CMessage& msg);							// Do address capture
	void CaptureAddress(const CMessageList& msgs);						// Do address capture
	void CaptureAddress(CAddressList& addrs);							// Do address capture
	void ExtractCaptureAddress(const CMessage& msg, CAddressList& addrs);		// Extract address captures

	// O F F L I N E
	void	GoOffline(bool force, bool sync, bool fast);
	void	DoOfflineSync(bool fast);
	void	GoOnline(bool sync);

	// Sleep controls
	void	Suspend();
	void	Resume();

private:
	CAdbkProtocol*				mLocalProto;
	CAdbkProtocol*				mOSProto;
	CAdbkProtocolList			mProtos;
	unsigned long				mProtoCount;
	CAddrLookupProtocolList		mLookups;
	CAddressBook				mRoot;
	CAddressBookList			mAdbkNickName;
	CAddressBookList			mAdbkSearch;
	
	CAddressSearchResultList	mSearchResultList;
};

#endif
