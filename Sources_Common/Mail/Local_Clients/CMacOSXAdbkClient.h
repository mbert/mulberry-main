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


// Header for MacOSX Adbk client class

#ifndef __CMACOSXADBKCLIENT__MULBERRY__
#define __CMACOSXADBKCLIENT__MULBERRY__

#include "CAdbkClient.h"

#include "CAdbkRecord.h"

#include <AddressBook/ABAddressBookC.h>

// consts

class CMacOSXAdbkClient: public CAdbkClient
{

	// I N S T A N C E  V A R I A B L E S

private:
	CAdbkRecord*	mRecorder;						// Recording object
	unsigned long	mRecordID;						// Recording ID

	CAdbkAddress*	mTempAddr;						// Used to store address while searching
	CGroup*			mTempGrp;						// Used to store group while searching
	bool			mSearchMode;					// Search - do not add addresses

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CMacOSXAdbkClient(CAdbkProtocol* adbk_owner);
			CMacOSXAdbkClient(const CMacOSXAdbkClient& copy,
						CAdbkProtocol* adbk_owner);
	virtual	~CMacOSXAdbkClient();

private:
			void	InitAdbkClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	// S T A R T  &  S T O P
	virtual void	SetRecorder(CAdbkRecord* recorder)
		{ mRecorder = recorder; }

	virtual void	Open();									// Start TCP (specify server)
	virtual void	Reset();								// Reset account
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort

	// L O G I N  &  L O G O U T
	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server

	// P R O T O C O L
	virtual void	_Tickle(bool force_tickle);				// Do tickle
	virtual void	_PreProcess();							// About to start processing input
	virtual void	_PostProcess();							// Finished processing input

	// A D D R E S S B O O K S

	// Operations on address books
	virtual void	_ListAddressBooks(CAddressBook* root);
	virtual void	_FindAllAdbks(const cdstring& path);		// Find all adbks below this path
	virtual void	_CreateAdbk(const CAddressBook* adbk);		// Create adbk
	virtual bool	_TouchAdbk(const CAddressBook* adbk);		// Do touch
	virtual bool	_TestAdbk(const CAddressBook* adbk);		// Do test
	virtual void	_LockAdbk(const CAddressBook* adbk) {}
	virtual void	_UnlockAdbk(const CAddressBook* adbk) {}
	virtual bool	_AdbkChanged(const CAddressBook* adbk);
	virtual void	_UpdateSyncToken(const CAddressBook* adbk);
	virtual void	_DeleteAdbk(const CAddressBook* adbk);		// Delete adbk
	virtual void	_RenameAdbk(const CAddressBook* old_adbk,
								const cdstring& new_adbk);		// Rename adbk
	virtual void	_SizeAdbk(CAddressBook* adbk);

	// Operations with addresses
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
	// H A N D L E  E R R O R

	// L O C A L  O P S
	
			void	ScanAddressBook(CAddressBook* adbk);
			void	SearchAddressBook(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList* addr_list,
									CGroupList* grp_list,
									bool addresses,
									bool groups);
			void	ParseAddress(ABRecordRef abref);
			CAdbkAddress*	ParseAddressItem(ABRecordRef abref);
			void	ParseAddressEmail(ABRecordRef abref, CAdbkAddress* addr);
			void	ParseAddressAddress(ABRecordRef abref, CAdbkAddress* addr);
			void	ParseAddressPhone(ABRecordRef abref, CAdbkAddress* addr);
			void	ParseGroup(ABRecordRef abref);
			CGroup*	ParseGroupItem(ABRecordRef abref);
			long    	GetIntegerAttribute(ABRecordRef abref, CFStringRef attrib);
			cdstring	GetAddressAttribute(ABRecordRef abref, CFStringRef attrib);
			cdstring	GetAddressAttribute(ABRecordRef abref, CFStringRef attrib1, CFStringRef attrib2);
			cdstring	GetStringValue(CFStringRef text);
			cdstring	GetDictionaryValue(CFDictionaryRef dict, CFStringRef dictkey);

			void	Append(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps);
			void	Change(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps);
			void	Delete(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps);
			
			void	Fill(ABPersonRef person, const CAdbkAddress* addr);		// Copy address items into OS X record
			bool	FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EEmailType type, CFStringRef attrib);
			bool	FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EAddressType type, CFStringRef attrib);
			bool	FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EPhoneType type, CFStringRef attrib);

			void	Fill(ABAddressBookRef ab, ABGroupRef group, CAddressBook* adbk, const CGroup* grp);		// Copy group items into OS X record
};

#endif
