/*
 Copyright (c) 2010 Cyrus Daboo. All rights reserved.
 
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
 CWebDAVSyncReportParser.cpp
 
 Author:			
 Description:	<describe the CWebDAVSyncReportParser class here>
 */

#include "CWebDAVSyncReportParser.h"

#include "CHTTPUtils.h"
#include "CURL.h"
#include "CWebDAVDefinitions.h"
#include "XMLNode.h"

#include <ostream>
#include <strstream>

using namespace http; 
using namespace webdav; 
using namespace xmllib; 

CWebDAVSyncReportParser::CWebDAVSyncReportParser(cdstrmap& changed, cdstrset& removed, cdstring& sync_token) :
	mChanged(changed), mRemoved(removed), mSyncToken(sync_token)
{
}

CWebDAVSyncReportParser::~CWebDAVSyncReportParser()
{
}

void CWebDAVSyncReportParser::ParseResponse(const xmllib::XMLNode* response)
{
	// Verify that the node is the correct element <DAV:response>
	if (response->CompareFullName(cElement_response))
	{
	
		// Node is the right type, so iterate over all child response nodes and process each one
		cdstring href;
		for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
		{
			// Is it the href
			if ((*iter)->CompareFullName(cElement_href))
			{
				href = (*iter)->Data();
				CURL name(href, true);
				href = name.ToString(CURL::eLastPath);
			}
			
			// Is it status
			else if ((*iter)->CompareFullName(cElement_status))
			{
				ParseStatus(*iter, href);
			}
			
			// Is it propstat
			else if ((*iter)->CompareFullName(cElement_propstat))
			{
				ParsePropStat(*iter, href);
			}
		}
	}

	// Is it sync-token
	else if (response->CompareFullName(cElement_sync_token))
	{
		mSyncToken = response->Data();
	}
}

void CWebDAVSyncReportParser::ParseStatus(const xmllib::XMLNode* response, const cdstring& href)
{
	// Now parse the response and dispatch accordingly
	cdstring status = response->Data();
	if (!status.empty())
	{
		uint32_t status_result = CHTTPUtils::ParseStatusLine(status);
		
		// Any 404 code is OK
		if (status_result == 404)
		{
			mRemoved.insert(href);
		}
	}
}

void CWebDAVSyncReportParser::ParsePropStat(const xmllib::XMLNode* response, const cdstring& href)
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
					ParseProp(*iter, href);
				}
			}
			return;
		}
	}
}

void CWebDAVSyncReportParser::ParseProp(const xmllib::XMLNode* response, const cdstring& href)
{
	// Scan the prop node - each child is processed
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		ParsePropElement(*iter, href);
	}
}

void CWebDAVSyncReportParser::ParsePropElement(const xmllib::XMLNode* response, const cdstring& href)
{
	// Here we need to detect the type of element and dispatch accordingly
	
	// Ideally this will be overridden for specific types of properties being requested
	
	// For now detect the common ones in webdav-bis document
	
	// Get the full name of the element
	cdstring fullname = response->GetFullName();
	
	// Look for etag
	if (fullname == "DAV:getetag")
	{
		mChanged[href] = response->Data();
	}
}
