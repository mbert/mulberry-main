/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
	CCardDAVVCardClient.cpp

	Author:			
	Description:	<describe the CCardDAVVCardClient class here>
*/

#include "CCardDAVVCardClient.h"

#include "CAddressAccount.h"
#include "CAddressBook.h"
#include "CAdbkProtocol.h"
//#include "CDisplayItem.h"
#include "CGeneralException.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamUtils.h"
#include "CURL.h"

#include "CHTTPAuthorizationBasic.h"
#include "CHTTPAuthorizationDigest.h"
#include "CHTTPDataString.h"
#include "CHTTPDefinitions.h"
#include "CHTTPRequestResponse.h"
#include "CWebDAVDefinitions.h"
#include "CWebDAVDelete.h"
#include "CWebDAVGet.h"
#include "CWebDAVMakeCollection.h"
#include "CWebDAVPropFind.h"
#include "CWebDAVPut.h"
#include "CCardDAVMakeAdbk.h"
#include "CCardDAVMultigetReport.h"
#include "CCardDAVQueryReport.h"
#include "CCardDAVReportParser.h"

#include "CVCardMapper.h"

#include "CVCardAddressBook.h"
#include "CVCardDefinitions.h"
#include "CVCardVCard.h"

#include "ctrbuf.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include <memory>
#include <strstream>

using namespace vcardstore; 

// no locking
#define ORACLE_FIX_3	1

CCardDAVVCardClient::CCardDAVVCardClient(CAdbkProtocol* owner) :
	CWebDAVVCardClient(owner)
{
	// Init instance variables
	InitCardDAVClient();
}

CCardDAVVCardClient::CCardDAVVCardClient(const CCardDAVVCardClient& copy, CAdbkProtocol* owner) :
	CWebDAVVCardClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitCardDAVClient();
}

CCardDAVVCardClient::~CCardDAVVCardClient()
{
}

void CCardDAVVCardClient::InitCardDAVClient()
{
}

// Create duplicate, empty connection
CINETClient* CCardDAVVCardClient::CloneConnection()
{
	// Copy construct this
	return new CCardDAVVCardClient(*this, GetAdbkOwner());

}

#pragma mark ____________________________Protocol

// For CardDAV we need to do this twice - once on the base URI passed in, and then,
// if we know access control is supported, a second time on the principal resource
// which we know must indicate caldav capability levels.
bool CCardDAVVCardClient::Initialise(const cdstring& host, const cdstring& base_uri)
{
	cdstring actual_base_uri = base_uri;
	if (actual_base_uri.length() == 0)
		actual_base_uri = "/";
	
	if (CWebDAVVCardClient::Initialise(host, actual_base_uri) && HasDAVACL())
	{
		bool no_preport = false;
#ifdef NO_PRINCIPAL_REPORT
		no_preport = (GetAccount()->GetName().find("nopreport") != cdstring::npos);
#endif
		if (no_preport)
			return true;
		else
		{
			cdstring puri;
			if (GetSelfPrincipalResource(actual_base_uri, puri))
			{
				bool result = CWebDAVVCardClient::Initialise(host, puri);
				if (result)
				{
					// May need to reset calendar-home path
					if (GetAdbkOwner()->GetAddressAccount()->GetBaseRURL().empty())
					{
						// Get calendar-home-set from principal resource
						_GetPrincipalDetails(puri);
					}
				}
				return result;
			}
		}
	}
	
	return false;
}

void CCardDAVVCardClient::_GetPrincipalDetails(const cdstring& puri, bool reset_home)
{
	cdstrmap result;
	xmllib::XMLNameList props;
	props.push_back(http::carddav::cProperty_addressbook_home_set);
	if (GetProperties(puri, props, result))
	{
		cdstrmap::const_iterator found = result.find(http::carddav::cProperty_addressbook_home_set.FullName());
		if (found != result.end() && reset_home)
		{
			// May need to reset calendar-home path
			if (GetAdbkOwner()->GetAddressAccount()->GetBaseRURL().empty())
			{
				_Reset((*found).second);
			}
		}
		return;
	}
}

void CCardDAVVCardClient::_CreateAdbk(const CAddressBook* adbk)
{
	// Start UI action
	StINETClientAction _action(this, "Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook", adbk->GetName());

	// Determine URL
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');

	// Use MKCOL for directories
	if (adbk->IsDirectory())
	{
		// Create WebDAV MKCOL (overwrite not allowed)
		std::auto_ptr<http::webdav::CWebDAVMakeCollection> request(new http::webdav::CWebDAVMakeCollection(this, rurl));

		// Process it
		RunSession(request.get());
	
		// Check response status
		switch(request->GetStatusCode())
		{
		case http::eStatus_OK:
		case http::eStatus_Created:
			break;
		default:
			HandleHTTPError(request.get());
			return;
		}
	}
	else
	{
		// Create CardDAV MKADBK (overwrite not allowed)
		std::auto_ptr<http::carddav::CCardDAVMakeAdbk> request(new http::carddav::CCardDAVMakeAdbk(this, rurl));

		// Process it
		RunSession(request.get());
	
		// Check response status
		switch(request->GetStatusCode())
		{
		case http::eStatus_OK:
		case http::eStatus_Created:
		case http::webdav::eStatus_MultiStatus:
			break;
		default:
			HandleHTTPError(request.get());
			return;
		}
	}
}

bool CCardDAVVCardClient::_AdbkChanged(const CAddressBook* adbk)
{
	// Start UI action
	StINETClientAction _action(this, "Status::IMSP::Checking", "Error::IMSP::OSErrCheck", "Error::IMSP::NoBadCheck", adbk->GetName());

	// Determine URL and lock
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current CTag
	cdstring ctag = GetProperty(rurl, lock_token, http::calendarserver::cProperty_getctag);
	
	// Changed if ctags are different
	return ctag.empty() || (ctag != adbk->GetVCardAdbk()->GetETag());
}

void CCardDAVVCardClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// Start UI action
	StINETClientAction _action(this, "Status::IMSP::Checking", "Error::IMSP::OSErrCheck", "Error::IMSP::NoBadCheck", adbk->GetName());

	// Determine URL and lock
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current CTag
	cdstring ctag = GetProperty(rurl, lock_token, http::calendarserver::cProperty_getctag);
	
	const_cast<CAddressBook*>(adbk)->GetVCardAdbk()->SetETag(ctag);
}

void CCardDAVVCardClient::ListAddressBooks(CAddressBook* root, const http::webdav::CWebDAVPropFindParser& parser)
{
	cdstring nodeURL = root->GetName();
	nodeURL.EncodeURL('/');
	if (root->IsProtocol())
		nodeURL = mBaseRURL;
	cdstring nodeURLNoSlash = nodeURL;
	if (nodeURLNoSlash.compare_end("/"))
		nodeURLNoSlash.erase(nodeURLNoSlash.length() - 1);
	else
	{
		nodeURLNoSlash = nodeURL;
		nodeURL += "/";
	}

	cdstring relBase = nodeURL;
	cdstring relBaseNoSlash = nodeURLNoSlash;

	// Look at each propfind result and determine type of address book
	for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
	{
		// Get child element name (decode URL)
		cdstring rpath((*iter)->GetResource());
		
		// Strip of path prefix
		if (rpath.compare_start(mHostURL))
			rpath.erase(0, mHostURL.length());
		
		// Ignore root
		if ((rpath == relBase) || (rpath == relBaseNoSlash))
			continue;
		rpath.DecodeURL();

		// Determine type of element
		bool is_dir = false;
		bool is_adbk = false;
		if ((*iter)->GetNodeProperties().count(http::webdav::cProperty_resourcetype.FullName()) != 0)
		{
			bool is_col = false;

			// Get the XML node for resourcetype
			const xmllib::XMLNode* node = (*(*iter)->GetNodeProperties().find(http::webdav::cProperty_resourcetype.FullName())).second;
			
			// Look at each child element
			for(xmllib::XMLNodeList::const_iterator iter2 = node->Children().begin(); iter2 != node->Children().end(); iter2++)
			{
				// Look for collection
				if ((*iter2)->CompareFullName(http::webdav::cProperty_collection))
					is_col = true;
				
				// Look for address book
				if ((*iter2)->CompareFullName(http::carddav::cProperty_carddavadbk))
					is_adbk = true;
			}

			// Only allow address book if also a collection
			is_adbk = is_adbk && is_col;
			
			// Directory if a collection and not a address book
			is_dir = is_col && !is_adbk;
		}

		// Only add a node that is a normal collection or a address book - ignore everything else
		if (is_dir || is_adbk)
		{
			// Create the new node and add to parent
			CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), root, !is_dir, is_dir, rpath);
			root->AddChild(adbk);
			
			// Cannot determine the size of the address book collection without another PROPFIND, so ignore for now

			if ((*iter)->GetTextProperties().count(http::webdav::cProperty_displayname.FullName()) != 0)
			{
				cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_displayname.FullName())).second;
				// Temporary FIX for getting GUIDs back from the server
				if (result.length() == 36 && result.c_str()[8] == '-')
					result = cdstring::null_str;
				adbk->SetDisplayName(result);
			}
		}
	}
}

void CCardDAVVCardClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	// Start UI action
	StINETClientAction _action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());

	// Policy:
	//
	// Do PROPFIND Depth 1 to get all enclosed items
	// Extract HREFs for each item
	// Get each item found and parse into calendar one at a time whilst caching HREF
	
	// Create WebDAV propfind
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	props.push_back(http::webdav::cProperty_getetag);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth1, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());
		ReadAddressBookComponents(adbk, parser, *adbk->GetVCardAdbk());
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
	
	// Now map VCards into internal addresses
	vcardstore::MapFromVCards(adbk);
}

void CCardDAVVCardClient::ReadAddressBookComponents(CAddressBook* adbk, const http::webdav::CWebDAVPropFindParser& parser, vCard::CVCardAddressBook& vadbk)
{
	// Get all the component urls
	cdstrmap comps;
	GetAddressBookComponents(adbk, vadbk, parser, comps, false);

	// Get list of hrefs
	cdstrvect hrefs;
	for(cdstrmap::const_iterator iter = comps.begin(); iter != comps.end(); iter++)
	{
		hrefs.push_back((*iter).first);
	}

	// Run the AddressBook-multiget report
	ReadAddressBookComponents(adbk, hrefs, vadbk);
}

void CCardDAVVCardClient::ReadAddressBookComponents(CAddressBook* adbk, const cdstrvect& hrefs, vCard::CVCardAddressBook& vadbk)
{
	// Don't bother with this if empty (actually spec requires at least one href)
	if (hrefs.empty())
		return;

	// Run the adbk-multiget report
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');

	// Create WebDAV REPORT
	std::auto_ptr<http::carddav::CCardDAVMultigetReport> request(new http::carddav::CCardDAVMultigetReport(this, rurl, hrefs));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// Check response status
	switch(request->GetStatusCode())
	{
	case http::webdav::eStatus_MultiStatus:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	http::carddav::CCardDAVReportParser parser(*adbk->GetVCardAdbk());
	parser.ParseData(dout.GetData());
}

void CCardDAVVCardClient::GetAddressBookComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const http::webdav::CWebDAVPropFindParser& parser, cdstrmap& compinfo, bool last_path)
{
	CURL base(mHostURL + adbk->GetName(), true);
	
	// Look at each propfind result and determine type of address book
	for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
	{
		// Get child element name (decode URL)
		CURL name((*iter)->GetResource(), true);
	
		// Ignore if base
		if (base.Equal(name) || base.EqualRelative(name))
			continue;
		
		// Verify that its a text/vcard type
		if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getcontenttype.FullName()) != 0)
		{
			// Get content-type
			cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getcontenttype.FullName())).second;
			
			// Strip down to actual value
			result.erase(result.find(';'));
			result.trimspace();
			
			// Now look for text/vcard
			if (result.compare("text/vcard") != 0)
				continue;
		}
		
		// Get ETag for item
		cdstring etag;
		if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getetag.FullName()) != 0)
		{
			etag = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getetag.FullName())).second;
			
			// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
			if (!etag.isquoted())
				etag.quote(true);
		}

		// Store the component
		compinfo.insert(cdstrmap::value_type(name.ToString(last_path ? CURL::eLastPath : CURL::eRelative), etag));
	}
}

vCard::CVCardVCard* CCardDAVVCardClient::ReadAddressBookComponent(const cdstring& rurl, vCard::CVCardAddressBook& adbk)
{
	// Create WebDAV GET
	std::auto_ptr<http::webdav::CWebDAVGet> request(new http::webdav::CWebDAVGet(this, rurl));
	http::CHTTPOutputDataString dout;
	request->SetData(&dout);

	// Process it
	RunSession(request.get());

	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return NULL;
	}

	// Get last segment of RURL path
	cdstring last_path(rurl);
	if (rurl.find('/') != cdstring::npos)
		last_path.assign(rurl, rurl.rfind('/') + 1, cdstring::npos);
	
	// Update ETag
	cdstring etag;
	if (request->GetNewETag() != NULL)
	{
		etag = *request->GetNewETag();

		// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
		if (!etag.isquoted())
			etag.quote(true);
	}
	else
		etag = cdstring::null_str;

	// Read vcard component(s) from file
	cdstring data = dout.GetData();
	std::istrstream is(data.c_str());
	return adbk.ParseComponent(is, last_path, etag);
}

void CCardDAVVCardClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	// Start UI action
	StINETClientAction _action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress", adbk->GetName());

	// Policy:
	//
	// Write each component group to server if it has changed or is new
	// We do not attempt to delete components on the server if they are not present locally

	// No longer write timezones as those are included with each respective event/to-do etc as needed	
	//WriteCalendarComponents(node, cal, cal.GetVTimezone());

	WriteAddressBookComponents(adbk, *adbk->GetVCardAdbk(), adbk->GetVCardAdbk()->GetVCards());
}

void CCardDAVVCardClient::WriteAddressBookComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardComponentDB& components)
{
	for(vCard::CVCardComponentDB::const_iterator iter = components.begin(); iter != components.end(); iter++)
		WriteAddressBookComponent(adbk, vadbk, *static_cast<const vCard::CVCardVCard*>((*iter).second));
}

void CCardDAVVCardClient::WriteAddressBookComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// We do not write recurrence instances, only non-recurring or master instance components.
	// Writing the master will result in the instances being added automatically.
	if (!component.CanGenerateInstance())
		return;

	// Check whether it is new or existing
	if (component.GetRURL().empty())
		AddComponent(adbk, vadbk, component);
	else
		ChangeComponent(adbk, vadbk, component);
}

bool CCardDAVVCardClient::_CanUseComponents() const
{
	// Can handle components separately
	return true;
}

void CCardDAVVCardClient::_GetComponentInfo(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, cdstrmap& comps)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", adbk->GetName());

	// Policy:
	//
	// Do PROPFIND Depth 1 to get all enclosed items
	// Extract HREFs for each item
	// Get each item found and parse into address book one at a time whilst caching HREF
	
	// Create WebDAV propfind
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	props.push_back(http::webdav::cProperty_getetag);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth1, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());
		GetAddressBookComponents(adbk, vadbk, parser, comps, true);
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

void CCardDAVVCardClient::_AddComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Adding", "Error::Calendar::OSErrAddingToCalendar", "Error::Calendar::NoBadAddingToCalendar", adbk->GetName());

	// Create new resource via lock NULL
	AddComponent(adbk, vadbk, component);
}

void CCardDAVVCardClient::_ChangeComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Changing", "Error::Calendar::OSErrChangingCalendar", "Error::Calendar::NoBadChangingCalendar", adbk->GetName());

	// We will assume that we always overwrite the server even if the server copy has changed
	ChangeComponent(adbk, vadbk, component);
}

void CCardDAVVCardClient::_RemoveComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// Must have valid component URL
	if (component.GetRURL().empty())
		return;

	// Do removal
	_RemoveComponent(adbk, vadbk, component.GetRURL());
}

void CCardDAVVCardClient::_RemoveComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstring& comp_rurl)
{
	// NB We really ought to check whether the item has changed before we do the delete (ETag comparison),
	// but we will ignore that for now

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Removing", "Error::Calendar::OSErrRemoveFromCalendar", "Error::Calendar::NoBadRemoveFromCalendar", adbk->GetName());

	// Must have valid component URL
	if (comp_rurl.empty())
		return;

	// Determine URL
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	rurl += comp_rurl;

	// Create WebDAV DELETE
	std::auto_ptr<http::webdav::CWebDAVDelete> request(new http::webdav::CWebDAVDelete(this, rurl));

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Accepted:
	case http::eStatus_NoContent:
		break;
	case http::webdav::eStatus_MultiStatus:
		break;
	default:
		HandleHTTPError(request.get());
		return;
	}
}

// Read single component from server
void CCardDAVVCardClient::_ReadComponents(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstrvect& rurls)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", adbk->GetName());

	// Determine URLs
	cdstrvect hrefs;
	cdstring root_url = adbk->GetName();
	root_url.EncodeURL('/');
	for(cdstrvect::const_iterator iter = rurls.begin(); iter != rurls.end(); iter++)
	{
		cdstring rurl = root_url;
		rurl += *iter;
		hrefs.push_back(rurl);
	}

	// Read it all in	
	return ReadAddressBookComponents(adbk, hrefs, vadbk);
}


// Read single component from server
vCard::CVCardVCard* CCardDAVVCardClient::_ReadComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const cdstring& comp_rurl)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", adbk->GetName());

	// Determine URL
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	rurl += comp_rurl;

	// Read it in	
	return ReadAddressBookComponent(rurl, vadbk);
}

void CCardDAVVCardClient::AddComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// Create new resource via lock NULL
	
	// Policy:
	//
	// Generate new unique URL for component
	// Get LOCK NULL on that - if it exists, regenerate unique URL
	// Once we have the lock, PUT it to create it
	// Then unlock
	
	cdstring rurl;
	cdstring lock_token;
	while(true)
	{
		// Generate unqiue RURL
		const_cast<vCard::CVCardVCard&>(component).GenerateRURL();

		// Determine URL
		rurl = adbk->GetName();
		rurl.EncodeURL('/');
		rurl += component.GetRURL();

#ifndef ORACLE_FIX_3
		// Do LOCK NULL request (5 minute timeout)
		lock_token = LockResource(rurl, 300, true);

		if (lock_token != "?")
#endif
			break;
	}
	
	// Make sure we got the lock
#ifndef ORACLE_FIX_3
	if (lock_token.empty())
		return;
	
	// Add lock token to list
	mLockTokens.insert(cdstrmap::value_type(rurl, lock_token));
#endif

	// Now PUT the resource
	WriteComponent(adbk, vadbk, component, true);

	// Now unlock
#ifndef ORACLE_FIX_3
	UnlockToken(rurl);
#endif
}

void CCardDAVVCardClient::ChangeComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component)
{
	// Determine URL
#ifndef ORACLE_FIX_3
	cdstring rurl = adbk.GetName();
	rurl.EncodeURL('/');
	rurl += component.GetRURL();

	// Lock it
	bool locked = LockToken(rurl, 300);
#endif

	// Now PUT the resource
	WriteComponent(adbk, vadbk, component, false);

	// Now unlock
#ifndef ORACLE_FIX_3
	if (locked)
		UnlockToken(rurl);
#endif
}

void CCardDAVVCardClient::WriteComponent(CAddressBook* adbk, vCard::CVCardAddressBook& vadbk, const vCard::CVCardVCard& component, bool new_item)
{
	// Determine URL
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	rurl += component.GetRURL();
	cdstring lock_token = GetLockToken(rurl);

	// Now PUT the resource

	// Write address book file to stream
	std::ostrstream os;
	vadbk.GenerateOne(os, component);
	os << std::ends;
	cdstring data;
	data.steal(os.str());

	// Create WebDAV PUT
	std::auto_ptr<http::webdav::CWebDAVPut> request(new http::webdav::CWebDAVPut(this, rurl, lock_token));
	http::CHTTPInputDataString din(data, "text/vcard; charset=utf-8");
	http::CHTTPOutputDataString dout;
	
	// Use lock if present, otherwise ETag
	request->SetData(&din, &dout, new_item);

	// Process it
	RunSession(request.get());	
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Created:
	case http::eStatus_NoContent:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	// Get new location
	cdstring location = request->GetResponseHeader(cHeaderLocation);
	if (!location.empty())
	{
		// Check for "redirect" on PUT
		
		// We will assume that we only get redirected to another resource in the same collection
		cdstrvect splits;
		location.split("/", splits);
		cdstring new_path = splits.back();
		if (new_path != component.GetRURL())
		{
			const_cast<vCard::CVCardVCard&>(component).SetRURL(new_path);
			rurl = adbk->GetName();
			rurl.EncodeURL('/');
			rurl += component.GetRURL();
		}
	}

	// Update ETag
	if (request->GetNewETag() != NULL)
	{
		cdstring temp(*request->GetNewETag());

		// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
		if (!temp.isquoted())
			temp.quote(true);
		
		const_cast<vCard::CVCardVCard&>(component).SetETag(temp);
	}
	else
		const_cast<vCard::CVCardVCard&>(component).SetETag(GetETag(rurl));
	
	const_cast<vCard::CVCardVCard&>(component).SetChanged(false);
}

// Fetch named addresses
void CCardDAVVCardClient::_FetchAddress(CAddressBook* adbk, const cdstrvect& names)
{
	// We do not do this for now
}

// Store address
void CCardDAVVCardClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Adding", "Error::Calendar::OSErrAddingToCalendar", "Error::Calendar::NoBadAddingToCalendar", adbk->GetName());

	// Address book may or may not be open
	bool is_open = (adbk->GetVCardAdbk() != NULL);
	vCard::CVCardAddressBook* vadbk = NULL;
	std::auto_ptr<vCard::CVCardAddressBook> vadbk_auto;
	if (adbk->GetVCardAdbk())
		vadbk = adbk->GetVCardAdbk();
	else
	{
		vadbk = new vCard::CVCardAddressBook();
		vadbk_auto.reset(vadbk);
	}
	

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		// If its open use the vCard from the address bookm otherwise create a temp vCard for this operation only
		const vCard::CVCardVCard* vcard = NULL;
		std::auto_ptr<vCard::CVCardVCard> vcard_auto;
		if (is_open)
			vcard = vadbk->GetCardByKey(static_cast<const CAdbkAddress*>(*iter)->GetEntry());
		else
		{
			vcard_auto.reset(vcardstore::GenerateVCard(vadbk->GetRef(), static_cast<const CAdbkAddress*>(*iter), true));
			vcard = vcard_auto.get();
		}
		AddComponent(adbk, *vadbk, *vcard);
	}
}

// Store group
void CCardDAVVCardClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// We do not do this for now
}

// Store address
void CCardDAVVCardClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Address book must be open
	vCard::CVCardAddressBook* vadbk = adbk->GetVCardAdbk();
	if (vadbk == NULL)
		return;

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Adding", "Error::Calendar::OSErrAddingToCalendar", "Error::Calendar::NoBadAddingToCalendar", adbk->GetName());

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		const vCard::CVCardVCard* vcard = vadbk->GetCardByKey(static_cast<const CAdbkAddress*>(*iter)->GetEntry());
		if (vcard != NULL)
			ChangeComponent(adbk, *vadbk, *vcard);
	}
}

// Store group
void CCardDAVVCardClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// We do not do this for now
}

// Delete address
void CCardDAVVCardClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Address book must be open
	vCard::CVCardAddressBook* vadbk = adbk->GetVCardAdbk();
	if (vadbk == NULL)
		return;

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Adding", "Error::Calendar::OSErrAddingToCalendar", "Error::Calendar::NoBadAddingToCalendar", adbk->GetName());

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		const vCard::CVCardVCard* vcard = vadbk->GetCardByKey(static_cast<const CAdbkAddress*>(*iter)->GetEntry());
		if (vcard != NULL)
			_RemoveComponent(adbk, *vadbk, *vcard);
	}
}

// Delete address
void CCardDAVVCardClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// We do not do this for now
}

// Resolve address nick-name
void CCardDAVVCardClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all matching addresses
		CAdbkAddress::CAddressFields fields;
		fields.push_back(CAdbkAddress::eNickName);
		SearchAddressBook(adbk, nick_name, CAdbkAddress::eMatchExactly, fields, NULL, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Resolve group nick-name
void CCardDAVVCardClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	// We do not do this for now
}

// Do search
void CCardDAVVCardClient::_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book - addresses actually go into list provided
	mActionAdbk = adbk;

	try
	{
		// Fetch all addresses
		SearchAddressBook(adbk, name, match, fields, &addr_list, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CCardDAVVCardClient::SizeAddressBook_DAV(CAddressBook* adbk)
{
	// Create WebDAV propfind
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth0, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());

		// Look at each propfind result and determine size of address book
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter)->GetResource());
			name.DecodeURL();
		
			// Must match rurl
			if (name.compare_end(rurl))
			{
				if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getcontentlength.FullName()) != 0)
				{
					cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getcontentlength.FullName())).second;
					long size = ::strtoul(result.c_str(), NULL, 10);
					adbk->SetSize(size);
					break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

void CCardDAVVCardClient::SizeAddressBook_HTTP(CAddressBook* adbk)
{
	// Create WebDAV HEAD
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');
	std::auto_ptr<http::webdav::CWebDAVGet> request(new http::webdav::CWebDAVGet(this, rurl, true));
	http::CHTTPOutputDataString dout;
	request->SetData(&dout);

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	// Grab size from response
	adbk->SetSize(request->GetContentLength());
}

void CCardDAVVCardClient::SearchAddressBook(CAddressBook* adbk, const cdstring& name, CAdbkAddress::EAddressMatch match, const CAdbkAddress::CAddressFields& fields,
											CAddressList* addr_list, CGroupList* grp_list)
{
	// Map adbk field to vCard property
	cdstrvect prop_names;
	for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
	{
		switch(*iter)
		{
		case CAdbkAddress::eName:
			prop_names.push_back(vCard::cVCardProperty_FN);
			break;
		case CAdbkAddress::eNickName:
			prop_names.push_back(vCard::cVCardProperty_NICKNAME);
			break;
		case CAdbkAddress::eEmail:
			prop_names.push_back(vCard::cVCardProperty_EMAIL);
			break;
		case CAdbkAddress::eCompany:
			prop_names.push_back(vCard::cVCardProperty_ORG);
			break;
		case CAdbkAddress::eAddress:
			prop_names.push_back(vCard::cVCardProperty_ADR);
			break;
		case CAdbkAddress::ePhoneWork:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::ePhoneHome:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::eFax:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::eURL:
			prop_names.push_back(vCard::cVCardProperty_URL);
			break;
		case CAdbkAddress::eNotes:
			prop_names.push_back(vCard::cVCardProperty_NOTE);
			break;
		default:;
		}
	}

	cdstring match_type;
	switch(match)
	{
	case CAdbkAddress::eMatchExactly:
		match_type = http::carddav::cAttributeValue_matchtype_is.Name();
		break;
	case CAdbkAddress::eMatchAtStart:
		match_type = http::carddav::cAttributeValue_matchtype_starts.Name();
		break;
	case CAdbkAddress::eMatchAtEnd:
		match_type = http::carddav::cAttributeValue_matchtype_ends.Name();
		break;
	case CAdbkAddress::eMatchAnywhere:
		match_type = http::carddav::cAttributeValue_matchtype_contains.Name();
		break;
	}

	// Run the adbk-multiget report
	cdstring rurl = adbk->GetName();
	rurl.EncodeURL('/');

	// Create WebDAV REPORT
	std::auto_ptr<http::carddav::CCardDAVQueryReport> request(new http::carddav::CCardDAVQueryReport(this, rurl, prop_names, name, match_type));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// Check response status
	switch(request->GetStatusCode())
	{
	case http::webdav::eStatus_MultiStatus:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	vCard::CVCardAddressBook temp;
	if (addr_list != NULL)
	{
		http::carddav::CCardDAVReportParser parser(temp, addr_list, true);
		parser.ParseData(dout.GetData());
	}
	else if (adbk != NULL)
	{
		http::carddav::CCardDAVReportParser parser(temp, adbk, true);
		parser.ParseData(dout.GetData());
	}
}

// Return the encoded url for the adbk
cdstring CCardDAVVCardClient::GetRURL(const CAddressBook* adbk, bool abs) const
{
	if (adbk->IsProtocol())
		return abs ? mBaseURL : mBaseRURL;
	else
		return GetRURL(adbk->GetName(), adbk->IsDirectory(), abs);
}

// Return the encoded url for the adbk
cdstring CCardDAVVCardClient::GetRURL(const cdstring& name, bool directory, bool abs) const
{
	// Determine URL
	cdstring rurl = (abs ? mBaseURL : mBaseRURL);
	if (name.c_str()[0] == '/')
		rurl = "";
	cdstring temp(name);
	temp.EncodeURL('/');
	rurl += temp;

	// CardDAV address books are collections with no special extensions and should be treated as directories
	if (rurl[rurl.length() - 1] != '/')
		rurl += "/";
	
	return rurl;
}
