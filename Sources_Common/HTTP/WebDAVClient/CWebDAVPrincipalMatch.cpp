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
	CWebDAVPrincipalMatch.cpp

	Author:			
	Description:	<describe the CWebDAVPrincipalMatch class here>
*/

#include "CWebDAVPrincipalMatch.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 

CWebDAVPrincipalMatch::CWebDAVPrincipalMatch(CWebDAVSession* session, const cdstring& ruri, const xmllib::XMLNameList& props) :
	CWebDAVPropFindBase(session, ruri, eDepth0)
{
	mProps = props;
	mMethod = eRequest_REPORT;
	
	InitRequestData();
}

CWebDAVPrincipalMatch::~CWebDAVPrincipalMatch()
{
}

void CWebDAVPrincipalMatch::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVPrincipalMatch::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:principal-match>
	//   <DAV:self/>
	//   <DAV:prop>
	//     <<names of each property as elements>>
	//   </DAV:prop>
	// </DAV:principal-match>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:principal-match> element
	xmllib::XMLNode* principalmatch = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(http::webdav::cNamespace, "D");
	principalmatch->SetName(http::webdav::cElement_principal_match.Name(), dav_namespc);
	principalmatch->AddNamespace(dav_namespc);
	
	// <DAV:self> element
	xmllib::XMLNode* selfnode = new xmllib::XMLNode(&xmldoc, principalmatch, http::webdav::cProperty_self.Name(), dav_namespc);
	
	if (!mProps.empty())
	{
		// <DAV:prop> element
		xmllib::XMLNode* prop = new xmllib::XMLNode(&xmldoc, principalmatch, http::webdav::cElement_prop.Name(), dav_namespc);
		
		// Now add each property
		for(xmllib::XMLNameList::const_iterator iter = mProps.begin(); iter != mProps.end(); iter++)
		{
			// Add property element taking namespace into account

			// Look for DAV namespace and reuse that one
			if (((*iter).Namespace() != NULL) && (::strcmp((*iter).Namespace(), http::webdav::cNamespace) == 0))
			{
				new xmllib::XMLNode(&xmldoc, prop, (*iter).Name(), dav_namespc);
			}
			else
				new xmllib::XMLNode(&xmldoc, prop, *iter);
		}
	}
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
