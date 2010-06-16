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
	CCardDAVVCardClient.h

	Author:			
	Description:	<describe the CCardDAVVCardClient class here>
*/

#ifndef CCardDAVVCardClient_H
#define CCardDAVVCardClient_H

#include "CWebDAVVCardClient.h"

namespace vCard
{
	class CVCardAddressBook;
	class CVCardComponent;
	class CVCardComponentDB;
	class CVCardVCard;
}

namespace vcardstore {

class CCardDAVVCardClient : public CWebDAVVCardClient
{
public:
	CCardDAVVCardClient(CAdbkProtocol* owner);
	CCardDAVVCardClient(const CCardDAVVCardClient& copy, CAdbkProtocol* owner);
	virtual ~CCardDAVVCardClient();

private:
			void	InitCardDAVClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection
	
	virtual bool Initialise(const cdstring& host, const cdstring& base_uri);

	// P R O T O C O L

	// A D D R E S S B O O K
	virtual void _GetPrincipalDetails(const cdstring& puri, bool reset_home=false);

	virtual void _CreateAdbk(const CAddressBook* adbk);
	virtual bool _AdbkChanged(const CAddressBook* adbk);
	virtual void _UpdateSyncToken(const CAddressBook* adbk);

	virtual void	_TestFastSync(const CAddressBook* adbk);
	virtual void	_FastSync(const CAddressBook* adbk, cdstrmap& changed, cdstrset& removed, cdstring& synctoken);
	virtual void	_ReadFullAddressBook(CAddressBook* adbk);			// Find all addresses in adbk
	virtual void	_WriteFullAddressBook(CAddressBook* adbk);			// Write all addresses in adbk

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

	virtual bool _CanUseComponents() const;
	virtual void _GetComponentInfo(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, cdstrmap& comps);
	virtual void _AddComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);
	virtual void _ChangeComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);
	virtual void _RemoveComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);
	virtual void _RemoveComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstring& rurl);
	virtual void _ReadComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstrvect& rurls);
	virtual vCard::CVCardVCard* _ReadComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstring& rurl);

protected:
	virtual void ListAddressBooks(CAddressBook* root, const http::webdav::CWebDAVPropFindParser& parser);

	virtual void ReadAddressBookComponents(CAddressBook* adbk, const http::webdav::CWebDAVPropFindParser& parser, vCard::CVCardAddressBook& vadbk);
	virtual void ReadAddressBookComponents(CAddressBook* adbk, const cdstrvect& hrefs, vCard::CVCardAddressBook& vadbk);
	virtual void GetAddressBookComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const http::webdav::CWebDAVPropFindParser& parser, cdstrmap& compinfo, bool last_path);
	virtual vCard::CVCardVCard* ReadAddressBookComponent(const cdstring& url, vCard::CVCardAddressBook& adbk);
	
	virtual void WriteAddressBookComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardComponentDB& components);
	virtual void WriteAddressBookComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);

	virtual void AddComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);
	virtual void ChangeComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component);
	virtual void WriteComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component, bool new_item = false);

	virtual void SizeAddressBook_DAV(CAddressBook* adbk);
	virtual void SizeAddressBook_HTTP(CAddressBook* adbk);

			void SearchAddressBook (CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList* addr_list,
									CGroupList* grp_list);

	virtual cdstring GetRURL(const CAddressBook* adbk, bool abs = false) const;
	virtual cdstring GetRURL(const cdstring& name, bool directory, bool abs = false) const;

};

}	// namespace vcardstore

#endif	// CCardDAVVCardClient_H
