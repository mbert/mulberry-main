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


// Header for IMSP client class

#ifndef __CIMSPCLIENT__MULBERRY__
#define __CIMSPCLIENT__MULBERRY__

#include "COptionsClient.h"
#include "CAdbkClient.h"
#include "CIMSPCommon.h"

// Others
enum EIMSPVersion
{
	eIMSP = 1
};

const char cType_IMSP[] = "IMSP";

class CIMSPClient: public COptionsClient, public CAdbkClient
{

	// I N S T A N C E  V A R I A B L E S

private:
	EIMSPVersion	mVersion;						// Version of server
	cdstrvect		mTempList;						// Used to store address names while fetching all
	bool			mSearchMode;					// Search - do not add addresses
	CGroupList*		mMembersGroups;					// Groups that need member resolution (for Simeon)

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

public:
			CIMSPClient(COptionsProtocol* options_owner, CAdbkProtocol* adbk_owner);
			CIMSPClient(const CIMSPClient& copy,
						COptionsProtocol* options_owner,
						CAdbkProtocol* adbk_owner);
	virtual	~CIMSPClient();

private:
			void	InitIMSPClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual int		ProcessString(cdstring& str);			// Process for output (maybe quote or literalise)

protected:
	virtual tcp_port GetDefaultPort();						// Get default port;

	// P R O T O C O L
	virtual void	_InitCapability();					// Initialise capability flags to empty set
	virtual void	_ProcessCapability();				// Handle capability response
	virtual void	_NoCapability();					// Handle failed capability response

	virtual void	_PreProcess();						// About to start processing input
	virtual void	_PostProcess();						// Finished processing input

	virtual void	_ParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// O P T I O N S
	virtual void	_FindAllAttributes(const cdstring& entry);		// Find all attributes in entry
	virtual void	_SetAllAttributes(const cdstring& entry);		// Set all attributes in entry
	virtual void	_DeleteEntry(const cdstring& entry);			// Delete the entire entry
	virtual void	_GetAttribute(const cdstring& entry,			// Get attribute
									const cdstring& attribute);
	virtual void	_SetAttribute(const cdstring& entry,			// Set attribute
									const cdstring& attribute,
									const cdstring& value);
	virtual void	_DeleteAttribute(const cdstring& entry,			// Delete attribute
									const cdstring& attribute);


	// A D D R E S S B O O K S

	// Operations on address books
	virtual void	_ListAddressBooks(CAddressBook* root);
	virtual void	_FindAllAdbks(const cdstring& path);		// Find all adbks below this path
	virtual void	_CreateAdbk(const CAddressBook* adbk);		// Create adbk
	virtual bool	_TouchAdbk(const CAddressBook* adbk)		// Do touch
		{ return false; }
	virtual bool	_TestAdbk(const CAddressBook* adbk)			// Do test
		{ return true; }
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
	cdstring	GetKeyPrefix(const cdstring& entry);

	// H A N D L E  E R R O R
	virtual void	INETRecoverReconnect();						// Force reconnect
	virtual void	INETRecoverDisconnect();					// Force disconnect
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	// H A N D L E  R E S P O N S E
	void 	IMSPParseResponse(char** txt,
							CINETClientResponse* response);		// Parse text sent by server

	// P A R S I N G  I M A P  I N F O
	void	IMSPParseOption(char** txt);						// Parse IMSP OPTION reply

	void	IMSPParseAddressBook(char** txt);					// Parse IMSP ADDRESSBOOK reply
	void	IMSPParseSearchAddress(char** txt);					// Parse IMSP SEARCHADDRESS reply
	void	IMSPParseFetchAddress(char** txt);					// Parse IMSP FETCHADDRESS reply

	void	IMSPParseACLAddressBook(char** txt);				// Parse IMSP ACL ADDRESSBOOK reply
	void	IMSPParseMyRightsAddressBook(char** txt);			// Parse IMSP MYRIGHTS ADDRESSBOOK reply

private:
	void	BufferedFetchAddress(CAddressBook* adbk);

	void	_StoreAddress(CAddressBook* adbk,
							const CAdbkAddress* addr);				// Store address
	void	_StoreGroup(CAddressBook* adbk,
									const CGroup* grp);				// Store group
	void	_DeleteAddress(CAddressBook* adbk,
									const CAdbkAddress* addr);		// Delete address
	void	_DeleteGroup(CAddressBook* adbk,
									const CGroup* grp);				// Delete address
};

#endif
