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


// CAdbkClient

// Abstract base class that encapsulates the functionality of a remote address book store.
// This is used by the high level CAddressProtocol object to control the address book store.
// Derived classes will implement the backend address book client.
// e.g. CIMSPClient is derived from this class. Another class could be CACAPClient.
// This includes a mixin class for address book ACLs

#ifndef __CADBKCLIENT__MULBERRY__
#define __CADBKCLIENT__MULBERRY__

#include "CINETClient.h"

#include "CAdbkProtocol.h"

#include "cdstring.h"

// Classes
class CACL;
class CAdbkAddress;
class CAddressBook;
class CGroup;

class CAdbkClient : virtual public CINETClient
{
public:
		CAdbkClient(CAdbkProtocol* owner)
			: CINETClient(owner)
			{ mActionAdbk = NULL; mSearchResults = NULL; }
		CAdbkClient(const CAdbkClient& copy, CAdbkProtocol* owner)
			: CINETClient(copy, owner)
			{ mActionAdbk = NULL; mSearchResults = NULL; }
	virtual ~CAdbkClient() { mActionAdbk = NULL; mSearchResults = NULL; }

	// Operations on address books
	virtual void	_ListAddressBooks(CAddressBook* root) = 0;
	virtual void	_FindAllAdbks(const cdstring& path) = 0;		// Find all adbks below this path

	virtual void	_CreateAdbk(const CAddressBook* adbk) = 0;		// Create adbk
	virtual bool	_TouchAdbk(const CAddressBook* adbk) = 0;		// Do touch
	virtual bool	_TestAdbk(const CAddressBook* adbk) = 0;		// Do test
	virtual void	_LockAdbk(const CAddressBook* adbk) = 0;
	virtual void	_UnlockAdbk(const CAddressBook* adbk) = 0;
	virtual bool	_AdbkChanged(const CAddressBook* adbk) = 0;
	virtual void	_UpdateSyncToken(const CAddressBook* adbk) = 0;
	virtual void	_DeleteAdbk(const CAddressBook* adbk) = 0;		// Delete adbk
	virtual void	_RenameAdbk(const CAddressBook* old_adbk,
								const cdstring& new_adbk) = 0;		// Rename adbk
	virtual void	_SizeAdbk(CAddressBook* adbk) = 0;

	// Operations with addresses
	virtual void	_ReadFullAddressBook(CAddressBook* adbk) = 0;		// Find all addresses in adbk
	virtual void	_WriteFullAddressBook(CAddressBook* adbk) = 0;		// Write all addresses in adbk
	virtual void	_FindAllAddresses(CAddressBook* adbk) = 0;			// Find all addresses in adbk
	virtual void	_FetchAddress(CAddressBook* adbk,
									const cdstrvect& names) = 0;		// Fetch named addresses
	virtual void	_StoreAddress(CAddressBook* adbk,
									const CAddressList* addrs) = 0;		// Store address
	virtual void	_StoreGroup(CAddressBook* adbk,
									const CGroupList* grps) = 0;		// Store group
	virtual void	_ChangeAddress(CAddressBook* adbk,
									const CAddressList* addrs) = 0;		// Store address
	virtual void	_ChangeGroup(CAddressBook* adbk,
									const CGroupList* grps) = 0;		// Store group
	virtual void	_DeleteAddress(CAddressBook* adbk,
									const CAddressList* addrs) = 0;		// Delete address
	virtual void	_DeleteGroup(CAddressBook* adbk,
									const CGroupList* grps) = 0;		// Delete address

	virtual void	_ResolveAddress(CAddressBook* adbk,
									const char* nick_name,
									CAdbkAddress*& addr) = 0;			// Resolve address nick-name
	virtual void	_ResolveGroup(CAddressBook* adbk,
									const char* nick_name,
									CGroup*& grp) = 0;					// Resolve group nick-name

	virtual void	_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list) = 0;	// Do search

	// These must be implemented by specific client
	virtual void _SetACL(CAddressBook* adbk, CACL* acl) = 0;		// Set acl on server
	virtual void _DeleteACL(CAddressBook* adbk, CACL* acl) = 0;		// Delete acl on server
	virtual void _GetACL(CAddressBook* adbk) = 0;					// Get all acls for adbk from server
	virtual void _MyRights(CAddressBook* adbk) = 0;					// Get current user's rights to adbk

protected:
	CAddressBook*		mActionAdbk;								// Address book to act on
	CAddressList*		mSearchResults;								// Results of search

	CAdbkProtocol*		GetAdbkOwner() const						// Return type-cast owner
		{ return static_cast<CAdbkProtocol*>(mOwner); }

};

#endif
