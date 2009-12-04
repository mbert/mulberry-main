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
	CWebDAVPropFindParser.cpp

	Author:			
	Description:	<describe the CWebDAVPropFindParser class here>
*/

#include "CWebDAVPropFindParser.h"

#include "CHTTPUtils.h"
#include "CWebDAVDefinitions.h"
#include "XMLNode.h"

#include <memory>
#include <ostream>

using namespace http; 
using namespace webdav; 
using namespace xmllib; 

std::set<cdstring> CWebDAVPropFindParser::sSimpleProperties;

CWebDAVPropFindParser::CWebDAVPropFindParser()
{
	// Do the property set caches
	if (sSimpleProperties.empty())
	{
		sSimpleProperties.insert("DAV:creationdate");
		sSimpleProperties.insert("DAV:displayname");
		sSimpleProperties.insert("DAV:getcontentlanguage");
		sSimpleProperties.insert("DAV:getcontentlength");
		sSimpleProperties.insert("DAV:getcontenttype");
		sSimpleProperties.insert("DAV:getetag");
		sSimpleProperties.insert("DAV:getlastmodified");
		sSimpleProperties.insert("http://calendarserver.org/ns/getctag");
	}
}

CWebDAVPropFindParser::~CWebDAVPropFindParser()
{
	// Delete all results
	for(CPropFindResults::iterator iter = mResults.begin(); iter != mResults.end(); iter++)
	{
		delete *iter;
	}
	mResults.clear();
}

void CWebDAVPropFindParser::ParseResponse(const xmllib::XMLNode* response)
{
	// Verify that the node is the correct element <DAV:response>
	if (!response->CompareFullName(cElement_response))
		return;
	
	// Node is the right type, so iterate over all child response nodes and process each one
	std::auto_ptr<CPropFindResult> result(new CPropFindResult);
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		// Is it the href
		if ((*iter)->CompareFullName(cElement_href))
		{
			result->SetResource((*iter)->Data());
		}
		
		// Is it propstat
		else if ((*iter)->CompareFullName(cElement_propstat))
		{
			ParsePropStat(*iter, result.get());
		}
	}
	
	// Add the resource only if we got one
	if (!result->GetResource().empty())
	{
		mResults.push_back(result.release());
	}
}

void CWebDAVPropFindParser::ParsePropStat(const xmllib::XMLNode* response, CPropFindResult* result)
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
				if ((*iter)->CompareFullName(cElement_prop))
				{
					ParseProp(*iter, result);
				}
			}
			return;
		}
	}
}

void CWebDAVPropFindParser::ParseProp(const xmllib::XMLNode* response, CPropFindResult* result)
{
	// Scan the prop node - each child is processed
	for(XMLNodeList::const_iterator iter = response->Children().begin(); iter != response->Children().end(); iter++)
	{
		ParsePropElement(*iter, result);
	}
}

void CWebDAVPropFindParser::ParsePropElement(const xmllib::XMLNode* response, CPropFindResult* result)
{
	// Here we need to detect the type of element and dispatch accordingly
	
	// Ideally this will be overridden for specific types of properties being requested
	
	// For now detect the common ones in webdav-bis document
	
	// Get the full name of the element
	cdstring fullname = response->GetFullName();
	
	// See if its a simple property
	if (sSimpleProperties.count(fullname))
	{
		ParsePropElementSimple(response, result);
	}
	else
		ParsePropElementUnknown(response, result);
}

void CWebDAVPropFindParser::ParsePropElementSimple(const xmllib::XMLNode* response, CPropFindResult* result)
{
	// The grab the element data
	result->AddTextProperty(response->GetFullName(), response->Data());
	result->AddNodeProperty(response->GetFullName(), response);
}

void CWebDAVPropFindParser::ParsePropElementUnknown(const xmllib::XMLNode* response, CPropFindResult* result)
{
	// Just add the node
	result->AddNodeProperty(response->GetFullName(), response);
}

void CWebDAVPropFindParser::DebugPrint(std::ostream& os) const
{
	// Print out each result
	for(CPropFindResults::const_iterator iter1 = mResults.begin(); iter1 != mResults.end(); iter1++)
	{
		os << "--- Resource: " << (*iter1)->GetResource() << " ---" << std::endl;
		
		// Do each known property
		for(CPropFindResult::CTextPropertyMap::const_iterator iter2 = (*iter1)->GetTextProperties().begin(); iter2 != (*iter1)->GetTextProperties().end(); iter2++)
		{
			os << "Property: " << (*iter2).first << std::endl <<  "  Value: " << (*iter2).second << std::endl;
		}

		os << "--- Resource ---" << std::endl;
	}
}
