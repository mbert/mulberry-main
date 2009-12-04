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


// CRemoteAddressBook.cp

// Class to handle remote address books

#include "CRemoteAddressBook.h"

#include "CAdbkProtocol.h"

#include "CAddressBookManager.h"
#include "CLog.h"
#include "CMailAccount.h"
#include "CMailControl.h"
#include "CURL.h"

CRemoteAddressBook::CRemoteAddressBook(CAdbkProtocol* server)
{
	SetProtocol(server);
}

CRemoteAddressBook::CRemoteAddressBook(CAdbkProtocol* server, const char* name) :
	CAddressBook(name)
{
	SetProtocol(server);
}

// Copy constructor
CRemoteAddressBook::CRemoteAddressBook(const CRemoteAddressBook& copy) :
	CAddressBook(copy)
{
	mProtocol = copy.mProtocol;
}

// Destructor
CRemoteAddressBook::~CRemoteAddressBook()
{
	Close();
	mProtocol = NULL;
}

#pragma mark ____________________________Opening/Closing

// Get name with account prefix
cdstring CRemoteAddressBook::GetAccountName() const
{
	cdstring name = mProtocol->GetAccountName();
	name += cMailAccountSeparator;
	name += GetName();

	return name;
}

// Get URL for address book
cdstring CRemoteAddressBook::GetURL(bool full) const
{
	cdstring url = mProtocol->GetURL(full);
	url += cURLHierarchy;
	url += cIMSPURLAddress;

	cdstring ename = GetName();
	ename.EncodeURL();
	url += ename;

	return url;
}

// Set server
void CRemoteAddressBook::SetProtocol(CAdbkProtocol* adbk)
{
	mProtocol= adbk;
	if (mProtocol)
	{
		SetFlags(eLocalAdbk, mProtocol->IsOffline());
		SetFlags(eCachedAdbk, mProtocol->IsOffline() && !mProtocol->IsDisconnected());
	}
}

// New visual address book on source
void CRemoteAddressBook::New()
{
	try
	{
		// Create on server
		mProtocol->CreateAdbk(this);
		mProtocol->GetAdbkList()->push_back(this, false);

		// Try to read in remote details first
		Read();

		// Do inherited
		CAddressBook::New();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Open visual address book from source
void CRemoteAddressBook::Open()
{
	// Bump reference count and open only if not already done
	if (!OpenCount())
		return;

	try
	{
		// Try to read in remote details first
		Read();

		// Do inherited
		CAddressBook::Open();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Read in addresses
void CRemoteAddressBook::Read()
{
	// Must remove any cached address lookups
	mAddresses.clear();
	mGroups.clear();

	// Now open
	mProtocol->OpenAdbk(this);

	// Do inherited
	CAddressBook::Read();
}

// Save addresses
void CRemoteAddressBook::Save()
{
	mProtocol->AdbkChanged(this);
}

// Close visual address book
void CRemoteAddressBook::Close()
{
	// Only if already open
	if (!IsOpen())
		return;

	// Bump reference count and close only if not open elsewhere
	if (!CloseCount())
		return;

	mProtocol->CloseAdbk(this);

	// Do inherited
	CAddressBook::Close();
}

// Rename address book
void CRemoteAddressBook::Rename(cdstring& new_name)
{
	mProtocol->RenameAdbk(this, new_name);
	CAddressBook::Rename(new_name);
}

// Rename address book
void CRemoteAddressBook::Delete()
{
	// Do inherited first
	CAddressBook::Delete();

	// Now delete on server
	mProtocol->DeleteAdbk(this);

	// Remove from manager
	CAddressBookManager::sAddressBookManager->RemoveAddressBook(this);
}

// Empty
void CRemoteAddressBook::Empty()
{
	// Delete and recreate
	mProtocol->DeleteAdbk(this);
	mProtocol->CreateAdbk(this);
	
	// Do inherited
	CAddressBook::Empty();
}

// Synchronise to local
void CRemoteAddressBook::Synchronise(bool fast)
{
	// Tell protocol to synchronise it
	mProtocol->SynchroniseRemote(this, fast);
}

// Clear disconnected cache
void CRemoteAddressBook::ClearDisconnect()
{
	mProtocol->ClearDisconnect(this);
}

// Switch into disconnected mode
void CRemoteAddressBook::SwitchDisconnect(CAdbkProtocol* local)
{
	// See if going into disconnected mode (i.e. local != NULL)
	if (local)
	{
		// Set local flag
		SetFlags(CAddressBook::eLocalAdbk);

		// See if it exists locally
		if (local->TestAdbk(this))
			SetFlags(CAddressBook::eCachedAdbk);
	}
	else
		// Remove disconnected flags
		SetFlags(static_cast<CAddressBook::EFlags>(CAddressBook::eLocalAdbk | CAddressBook::eCachedAdbk), false);
}

#pragma mark ____________________________Adding/removing


void CRemoteAddressBook::AddAddress(CAddressList* addrs, bool sorted)
{
	mProtocol->AddAddress(this, addrs);
	CAddressBook::AddAddress(addrs, sorted);

	// Do change notification
	CMailControl::AddressAdded(this, addrs);
}

// Add unique addresses from list
void CRemoteAddressBook::AddUniqueAddresses(CAddressList& add)
{
	CAddressList unique;
	unique.set_delete_data(false);

	// Count unique items first
	for(CAddressList::iterator iter = add.begin(); iter != add.end(); iter++)
	{
		if (!mAddresses.IsDuplicate(*iter))
			unique.push_back(new CAdbkAddress(**iter));
	}

	// Add unique items
	if (unique.size())
		AddAddress(&unique);
}

void CRemoteAddressBook::UpdateAddress(CAddressList* addrs, bool sorted)
{
	// This is strange: the protocol needs to have the old address data in order to do the actual delete,
	// since it keys off the data and not the pointer. The internal address book uses the pointer of the
	// original item (now the new item). So we must get the use of old_addr and new_addr correct.

	// Remove it then add it
	mProtocol->ChangeAddress(this, addrs);
	CAddressBook::UpdateAddress(addrs);

	// Do change notification
	CMailControl::AddressChanged(this, addrs);
}

void CRemoteAddressBook::RemoveAddress(CAddressList* addrs)
{
	mProtocol->RemoveAddress(this, addrs);
	CAddressBook::RemoveAddress(addrs);

	// Do change notification
	CMailControl::AddressRemoved(this, addrs);
}

void CRemoteAddressBook::AddGroup(CGroupList* grps, bool sorted)
{
	mProtocol->AddGroup(this, grps);
	CAddressBook::AddGroup(grps, sorted);

	// Do change notification
	CMailControl::GroupAdded(this, grps);
}

// Add unique groups from list
void CRemoteAddressBook::AddUniqueGroups(CGroupList& add)
{
	CGroupList unique;
	unique.set_delete_data(false);

	// Count unique items first
	for(CGroupList::const_iterator iter = add.begin(); iter != add.end(); iter++)
	{
		if (!mGroups.IsDuplicate(*iter))
			unique.push_back(new CGroup(**iter));
	}

	// Add unique items
	if (unique.size())
		AddGroup(&unique);
}

void CRemoteAddressBook::UpdateGroup(CGroupList* grps, bool sorted)
{
	// Remove it then add it
	mProtocol->ChangeGroup(this, grps);
	CAddressBook::UpdateGroup(grps);

	// Do change notification
	CMailControl::GroupChanged(this, grps);
}

void CRemoteAddressBook::RemoveGroup(CGroupList* grps)
{
	mProtocol->RemoveGroup(this, grps);
	CAddressBook::RemoveGroup(grps);

	// Do change notification
	CMailControl::GroupRemoved(this, grps);
}

#pragma mark ____________________________Lookup

// Find nick-name
bool CRemoteAddressBook::FindNickName(const char* nick_name, CAdbkAddress*& addr)
{
	// Resolve from cache
	bool result = CAddressBook::FindNickName(nick_name, addr);

	// Do remote lookup if not open
	if (!result && !IsOpen())
	{
		// Lookup
		mProtocol->ResolveAddress(this, nick_name, addr);

		// Resolveb from cache again
		result = CAddressBook::FindNickName(nick_name, addr);
	}

	return result;
}

// Find group nick-name
bool CRemoteAddressBook::FindGroupName(const char* grp_name, CGroup*& grp)
{
	// Resolve from cache
	bool result = CAddressBook::FindGroupName(grp_name, grp);

	// Do remote lookup if not open
	if (!result && !IsOpen())
	{
		// Lookup
		mProtocol->ResolveGroup(this, grp_name, grp);

		// Resolveb from cache again
		result = CAddressBook::FindGroupName(grp_name, grp);
	}

	return result;
}

// Do search
void CRemoteAddressBook::SearchAddress(const cdstring& name, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field, CAddressList& addr_list)
{
	// Do remote lookup only if not open
	if (!IsOpen())
		mProtocol->SearchAddress(this, name, match, field, addr_list);
	else
		CAddressBook::SearchAddress(name, match, field, addr_list);
}

#pragma mark ____________________________ACLs

// Get user's rights from server
void CRemoteAddressBook::CheckMyRights()
{
	mProtocol->MyRights(this);
}

// Add ACL to list
void CRemoteAddressBook::AddACL(const CAdbkACL* acl)
{
	// Create list if it does not exist
	if (!mACLs)
		mACLs = new CAdbkACLList;

	// Add
	mACLs->push_back(*acl);
}

// Set ACL on server
void CRemoteAddressBook::SetACL(CAdbkACL* acl)
{
	// Try to set on server
	try
	{
		mProtocol->SetACL(this, acl);

		// Create list if it does not exist
		if (!mACLs)
			mACLs = new CAdbkACLList;

		// Search for existing ACL
		CAdbkACLList::iterator found = ::find(mACLs->begin(), mACLs->end(), *acl);

		// Add if not found
		if (found == mACLs->end())
			mACLs->push_back(*acl);
		else
			// Replace existing
			*found = *acl;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Set ACL on server
void CRemoteAddressBook::DeleteACL(CAdbkACL* acl)
{
	// Try to delete on server
	try
	{
		mProtocol->DeleteACL(this, acl);

		// Search for existing ACL
		CAdbkACLList::iterator found = ::find(mACLs->begin(), mACLs->end(), *acl);

		// Remove it
		if (found != mACLs->end())
			mACLs->erase(found);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Get ACLs from server
void CRemoteAddressBook::CheckACLs()
{
	// Save existing list in case of failure
	CAdbkACLList* save = (mACLs ? new CAdbkACLList(*mACLs) : NULL);

	try
	{
		// Delete everything in existing list
		if (mACLs)
			mACLs->clear();

		mProtocol->GetACL(this);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Replace failed list with old one
		if (save)
			*mACLs = *save;
		delete save;

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	delete save;
}
