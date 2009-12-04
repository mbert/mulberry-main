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
	CCardDAVReportParser.cpp

	Author:			
	Description:	<describe the CCardDAVReportParser class here>
*/

#include "CCardDAVReportParser.h"

#include "CHTTPUtils.h"
#include "CWebDAVDefinitions.h"
#include "XMLNode.h"

#include "CVCardMapper.h"

#include "CVCardAddressBook.h"

#include <ostream>
#include <strstream>

using namespace http; 
using namespace webdav; 
using namespace carddav; 
using namespace xmllib; 

CCardDAVReportParser::CCardDAVReportParser(vCard::CVCardAddressBook& vadbk)
{
	mVAdbk = &vadbk;
	mAdbk = NULL;
	mAddrs = NULL;
	mAddItems = false;
}

CCardDAVReportParser::CCardDAVReportParser(vCard::CVCardAddressBook& vadbk, CAddressBook* adbk, bool add)
{
	mVAdbk = &vadbk;
	mAdbk = adbk;
	mAddrs = NULL;
	mAddItems = add;
}

CCardDAVReportParser::CCardDAVReportParser(vCard::CVCardAddressBook& vadbk, CAddressList* addrs, bool add)
{
	mVAdbk = &vadbk;
	mAdbk = NULL;
	mAddrs = addrs;
	mAddItems = add;
}

CCardDAVReportParser::~CCardDAVReportParser()
{
}

void CCardDAVReportParser::ParseResponse(const xmllib::XMLNode* response)
{
	// Verify that the node is the correct element <DAV:response>
	if (!response->CompareFullName(cElement_response))
		return;
	
	// Node is the right type, so iterate over all child response nodes and process each one
	cdstring href;
	cdstring etag;
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		// Is it the href
		if ((*iter)->CompareFullName(cElement_href))
		{
			href = (*iter)->Data();
		}
		
		// Is it propstat
		else if ((*iter)->CompareFullName(cElement_propstat))
		{
			ParsePropStat(*iter, href, etag);
		}
	}
}

void CCardDAVReportParser::ParsePropStat(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag)
{
	// Scan the propstat node the status - we only process OK status

	// Now look for a <DAV:status> element in its children
	cdstring status;
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		if ((*iter)->CompareFullName(cElement_status))
		{
			status = (*iter)->Data();
			break;
		}
	}
	
	// Now parse the response and dispatch accordingly
	if (!status.empty())
	{
		uint32_t status_result = CHTTPUtils::ParseStatusLine(status);
		
		// Any 2xx code is OK
		if (status_result / 100 == 2)
		{
			// Now look for a <DAV:prop> element in its children
			for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
			{
				if ((*iter)->CompareFullName(webdav::cElement_prop))
				{
					ParseProp(*iter, href, etag);
				}
			}
			return;
		}
	}
}

void CCardDAVReportParser::ParseProp(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag)
{
	// Scan the prop node - each child is processed
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		ParsePropElement(*iter, href, etag);
	}
}

void CCardDAVReportParser::ParsePropElement(const xmllib::XMLNode* response, const cdstring& href, cdstring& etag)
{
	// Here we need to detect the type of element and dispatch accordingly
	
	// Ideally this will be overridden for specific types of properties being requested
	
	// For now detect the common ones in webdav-bis document
	
	// Get the full name of the element
	cdstring fullname = response->GetFullName();
	
	// Look for etag
	if (fullname == "DAV:getetag")
	{
		etag = response->Data();
	}
		
	// Is it adbk-data
	else if (response->CompareFullName(cElement_adbkdata))
	{
		ParseAdbkData(response, href, etag);
	}
}

void CCardDAVReportParser::ParseAdbkData(const xmllib::XMLNode* response, const cdstring& href, const cdstring& etag)
{
	// Get last segment of RURL path
	cdstring last_path(href);
	if (href.find('/') != cdstring::npos)
		last_path.assign(href, href.rfind('/') + 1, cdstring::npos);
	
	// Read adbk component(s) from file
	std::istrstream is(response->Data().c_str());
	vCard::CVCardVCard* vcard = mVAdbk->ParseComponent(is, last_path, etag);
	if ((vcard != NULL) && mAddItems)
	{
		if (mAdbk != NULL)
			vcardstore::MapFromVCard(mAdbk, *vcard);
		else if (mAddrs != NULL)
			vcardstore::MapFromVCard(mAddrs, *vcard);
	}
}
