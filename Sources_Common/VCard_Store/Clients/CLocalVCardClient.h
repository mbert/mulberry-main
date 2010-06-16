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

/* 
	CLocalVCardClient.h

	Author:			
	Description:	<describe the CLocalVCardClient class here>
*/

#ifndef CLocalVCardClient_H
#define CLocalVCardClient_H

#include "CAdbkClient.h"

class CAdbkRecord;

namespace vcardstore {

class CLocalVCardClient: public CAdbkClient
{
public:
	CLocalVCardClient(CAdbkProtocol* owner);
	CLocalVCardClient(const CLocalVCardClient& copy, CAdbkProtocol* owner);
	virtual ~CLocalVCardClient();

	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual void	SetRecorder(CAdbkRecord* recorder)
		{ mRecorder = recorder; }

	virtual void	Open();									// Start
	virtual void	Reset();								// Reset account
protected:
	virtual void	CheckCWD();								// Check CWD
public:
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort
	virtual void	Forceoff();								// Forced close

	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server
	
	virtual void	_Tickle(bool force_tickle);			// Do tickle

	virtual void	_ListAddressBooks(CAddressBook* root);
	virtual void	_FindAllAdbks(const cdstring& path);		// Find all adbks below this path
	
	virtual void	_CreateAdbk(const CAddressBook* adbk);
	virtual bool	_TestAdbk(const CAddressBook* adbk);
	virtual bool	_TouchAdbk(const CAddressBook* adbk);
	virtual void	_LockAdbk(const CAddressBook* adbk) {}
	virtual void	_UnlockAdbk(const CAddressBook* adbk) {}
	virtual bool	_AdbkChanged(const CAddressBook* adbk);
	virtual void	_UpdateSyncToken(const CAddressBook* adbk);
	virtual void	_DeleteAdbk(const CAddressBook* adbk);
	virtual void	_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk);
	virtual void	_SizeAdbk(CAddressBook* adbk);

	// Operations with addresses
	virtual void	_TestFastSync(const CAddressBook* adbk);
	virtual void	_FastSync(const CAddressBook* adbk, cdstrmap& changed, cdstrset& removed, cdstring& synctoken);
	virtual void	_ReadFullAddressBook(CAddressBook* adbk);		// Find all addresses in adbk
	virtual void	_WriteFullAddressBook(CAddressBook* adbk);		// Write all addresses in adbk
	virtual void	_FindAllAddresses(CAddressBook* adbk);			// Find all addresses in adbk
	virtual void	_FetchAddress(CAddressBook* adbk,
									const cdstrvect& names);		// Fetch named addresses
	virtual void	_StoreAddress(CAddressBook* adbk,
									const CAddressList* addrs);		// Store address
	virtual void	_StoreGroup(CAddressBook* adbk,
									const CGroupList* grps);		// Store group
	virtual void	_ChangeAddress(CAddressBook* adbk,
									const CAddressList* addrs);		// Store address
	virtual void	_ChangeGroup(CAddressBook* adbk,
									const CGroupList* grps);		// Store group
	virtual void	_DeleteAddress(CAddressBook* adbk,
									const CAddressList* addrs);		// Delete address
	virtual void	_DeleteGroup(CAddressBook* adbk,
									const CGroupList* grps);		// Delete address

	virtual void	_ResolveAddress(CAddressBook* adbk,
									const char* nick_name,
									CAdbkAddress*& addr);			// Resolve address nick-name
	virtual void	_ResolveGroup(CAddressBook* adbk,
									const char* nick_name,
									CGroup*& grp);				// Resolve group nick-name

	virtual void	_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list);	// Do search

	// These must be implemented by specific client
	virtual void 	_SetACL(CAddressBook* adbk, CACL* acl);		// Set acl on server
	virtual void 	_DeleteACL(CAddressBook* adbk, CACL* acl);	// Delete acl on server
	virtual void 	_GetACL(CAddressBook* adbk);				// Get all acls for adbk from server
	virtual void 	_MyRights(CAddressBook* adbk);				// Get current user's rights to adbk

protected:
	bool				mCaching;						// Use extra cache files
	CAdbkRecord*		mRecorder;						// Recording object
	unsigned long		mRecordID;						// Recording ID
	cdstring	mCWD;		// Path to main address book hierarchy
	
	virtual bool IsCaching() const;

	// H A N D L E  E R R O R
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	cdstring MapAdbkName(const CAddressBook* adbk) const;
	cdstring MapDirName(const CAddressBook* adbk) const;
	cdstring MapAdbkName(const cdstring& node_name) const;
	cdstring MapDirName(const cdstring& node_name) const;
	cdstring MapAdbkCacheName(const CAddressBook* adbk) const;
	cdstring MapAdbkCacheName(const cdstring& node_name) const;

	void ListAddressBooks(CAddressBook* root, const cdstring& path);
	void Append(CAddressBook* adbk, const CAddressList* add_addrs, const CGroupList* add_grps);

	void SearchAddressBook (CAddressBook* adbk,
							const cdstring& pattern,
							const CAdbkAddress::CAddressFields& fields,
							CAddressList* addr_list,
							CGroupList* grp_list);


private:
			void	InitLocalClient();

};

}	// namespace vcardstore

#endif	// CLocalVCardClientLocal_H
