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
	CWebDAVVCardClient.h

	Author:			
	Description:	<describe the CWebDAVVCardClient class here>
*/

#ifndef CWebDAVVCardClient_H
#define CWebDAVVCardClient_H

#include "CAdbkClient.h"
#include "CWebDAVPropFindParser.h"
#include "CWebDAVSession.h"

#include "XMLNode.h"

namespace vcardstore {

class CWebDAVVCardClient : public CAdbkClient, public http::webdav::CWebDAVSession
{
public:
	CWebDAVVCardClient(CAdbkProtocol* owner);
	CWebDAVVCardClient(const CWebDAVVCardClient& copy, CAdbkProtocol* owner);
	virtual ~CWebDAVVCardClient();

private:
			void	InitWebDAVClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual void	Open();									// Open account
	virtual void	Reset();								// Reset account

	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server
	
protected:
	cdstring		mHostURL;
	cdstring		mBaseURL;
	cdstring		mBaseRURL;
	cdstrmap		mLockTokens;
	cdstring		mAuthUniqueness;

	virtual tcp_port GetDefaultPort();						// Get default port;

	virtual void	_Reset(const cdstring& baseRURL);		// Setup paths for this base URL

	// P R O T O C O L
	virtual void	_InitCapability();					// Initialise capability flags to empty set
	virtual void	_ProcessCapability();				// Handle capability response
	virtual void	_Tickle(bool force_tickle);			// Do tickle

	// A D D R E S S  B O O K
	virtual void	_ListAddressBooks(CAddressBook* root);
	virtual void	_FindAllAdbks(const cdstring& path);		// Find all adbks below this path
	
	virtual void	_CreateAdbk(const CAddressBook* adbk);
	virtual bool	_TestAdbk(const CAddressBook* adbk);
	virtual bool	_TouchAdbk(const CAddressBook* adbk);
	virtual void	_LockAdbk(const CAddressBook* adbk);
	virtual void	_UnlockAdbk(const CAddressBook* adbk);
	virtual bool	_AdbkChanged(const CAddressBook* adbk);
	virtual void	_UpdateSyncToken(const CAddressBook* adbk);
	virtual void	_DeleteAdbk(const CAddressBook* adbk);
	virtual void	_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk);
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

	// ACL related
	virtual void 	_SetACL(CAddressBook* adbk, CACL* acl);		// Set acl on server
	virtual void 	_DeleteACL(CAddressBook* adbk, CACL* acl);	// Delete acl on server
	virtual void 	_GetACL(CAddressBook* adbk);				// Get all acls for adbk from server
	virtual void 	_MyRights(CAddressBook* adbk);				// Get current user's rights to adbk

protected:
	// H A N D L E  E R R O R
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	virtual void OpenSession();
	virtual void CloseSession();
	void RunSession(CHTTPRequestResponse* request);
	void DoSession(CHTTPRequestResponse* request);

	virtual void SetServerType(unsigned long type);
	virtual void SetServerDescriptor(const cdstring& txt);
	virtual void SetServerCapability(const cdstring& txt);

	virtual CHTTPAuthorization* GetAuthorization(bool first_time, const cdstrvect& www_authenticate);
			bool CheckCurrentAuthorization() const;
	virtual void DoRequest(CHTTPRequestResponse* request);
	virtual void WriteRequestData(CHTTPRequestResponse* request);
	virtual void ReadResponseData(CHTTPRequestResponse* request);
	virtual void ReadResponseDataLength(CHTTPRequestResponse* request, unsigned long read_length);
	virtual unsigned long ReadResponseDataChunked(CHTTPRequestResponse* request);

	virtual void HandleHTTPError(CHTTPRequestResponse* request);
	virtual void DisplayHTTPError(CHTTPRequestResponse* request);

	virtual void ListAddressBooks(CAddressBook* root, const http::webdav::CWebDAVPropFindParser& parser);
	
	virtual void SizeAddressBook_DAV(CAddressBook* adbk);
	virtual void SizeAddressBook_HTTP(CAddressBook* adbk);

	void WriteFullAddressBook_Put(CAddressBook* adbk, const cdstring& lock_token = cdstring::null_str);
	void WriteFullAddressBook_Lock(CAddressBook* adbk);

	cdstring GetETag(const cdstring& rurl, const cdstring& lock_token = cdstring::null_str);
	cdstring GetProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property);
	cdstrvect GetHrefListProperty(const cdstring& rurl, const xmllib::XMLName& propname);
	bool GetProperties(const cdstring& rurl, const xmllib::XMLNameList& props, cdstrmap& results);
	bool GetSelfHrefs(const cdstring& rurl, cdstrvect& results);
	bool GetSelfPrincipalResource(const cdstring& rurl, cdstring& result);
	cdstring LockResource(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void UnlockResource(const cdstring& rurl, const cdstring& lock_token);

	virtual cdstring GetRURL(const CAddressBook* adbk, bool abs = false) const;
	virtual cdstring GetRURL(const cdstring& name, bool directory, bool abs = false) const;
	cdstring GetLockToken(const cdstring& rurl) const;
	bool	 LockToken(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void	 UnlockToken(const cdstring& rurl);
	
	void	WriteFullACL(CAddressBook* adbk);
	void	ParseACL(CAddressBook* adbk, const xmllib::XMLNode* aclnode);
	void	ParseMyRights(CAddressBook* adbk, const xmllib::XMLNode* rightsnode);
};

}	// namespace vcardstore

#endif	// CWebDAVVCardClient_H
