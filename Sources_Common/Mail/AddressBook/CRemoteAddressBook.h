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


// CRemoteAddressBook.h

// Class to handle remote address books

#ifndef __CREMOTEADDRESSBOOK__MULBERRY__
#define __CREMOTEADDRESSBOOK__MULBERRY__

#include "CAddressBook.h"

// Classes
class CAdbkProtocol;

class CRemoteAddressBook: public CAddressBook
{
public:
		explicit CRemoteAddressBook(CAdbkProtocol* server);
		explicit CRemoteAddressBook(CAdbkProtocol* server, const char* name);
		CRemoteAddressBook(const CRemoteAddressBook& copy);

	virtual ~CRemoteAddressBook();

	virtual cdstring GetAccountName() const;
	virtual cdstring GetURL(bool full = false) const;

	virtual CAdbkProtocol* GetProtocol()					// Get server
		{ return mProtocol; }
	virtual const CAdbkProtocol* GetProtocol() const		// Get server
		{ return mProtocol; }
	virtual void SetProtocol(CAdbkProtocol* adbk);			// Set server

	// Opening/closing
	virtual void	New();									// New visual address book on source
	virtual void	Open();									// Open visual address book from source
	virtual void	Read();									// Read in addresses
	virtual void	Save();									// Save addresses
	virtual void	Close();								// Close visual address book

	virtual void	Rename(cdstring& new_name);				// Rename
	virtual void	Delete();								// Delete
	virtual void	Empty();								// Empty

	virtual void	Synchronise(bool fast);					// Synchronise to local
	virtual void	ClearDisconnect();						// Clear disconnected cache
	virtual void	SwitchDisconnect(CAdbkProtocol* local);	// Switch into disconnected mode

	// Adding/removing items
	virtual void AddAddress(CAddressList* addrs, bool sorted = false);
	virtual void AddUniqueAddresses(CAddressList& add);						// Add unique addresses from list
	virtual void UpdateAddress(CAddressList* addrs, bool sorted = false);
	virtual void RemoveAddress(CAddressList* addrs);

	virtual void AddGroup(CGroupList* grps, bool sorted = false);
	virtual void AddUniqueGroups(CGroupList& add);							// Add unique groups from list
	virtual void UpdateGroup(CGroupList* grps, bool sorted = false);		// Group changed
	virtual void RemoveGroup(CGroupList* grps);

	// Lookup
	virtual	bool	FindNickName(const char* nick_name,
										CAdbkAddress*& addr);	// Find address from nick-name

	virtual	bool	FindGroupName(const char* grp_name,
										CGroup*& grp);			// Find group from group-name

	virtual void	SearchAddress(const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									CAdbkAddress::EAddressField field,
									CAddressList& addr_list);	// Do search

	// ACLs
	void	CheckMyRights();								// Get user's rights from server

	void	AddACL(const CAdbkACL* acl);						// Add ACL to list
	void	SetACL(CAdbkACL* acl);								// Set ACL on server
	void	DeleteACL(CAdbkACL* acl);							// Delete ACL on server
	void	CheckACLs();									// Get ACLs from server

private:
	CAdbkProtocol*	mProtocol;							// Remote server
};

#endif
