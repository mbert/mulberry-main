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
	CWebDAVPropFind.cpp

	Author:			
	Description:	<describe the CWebDAVPropFind class here>
*/

#include "CWebDAVPropFind.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 

CWebDAVPropFind::CWebDAVPropFind(CWebDAVSession* session, const cdstring& ruri, EWebDAVDepth depth, const xmllib::XMLNameList& props) :
	CWebDAVPropFindBase(session, ruri, depth)
{
	mProps = props;
	
	InitRequestData();
}

CWebDAVPropFind::~CWebDAVPropFind()
{
}

void CWebDAVPropFind::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVPropFind::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:propfind>
	//   <DAV:prop>
	//     <<names of each property as elements>>
	//   </DAV:prop>
	// </DAV:propfind>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:propfind> element
	xmllib::XMLNode* propfind = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(http::webdav::cNamespace, "D");
	propfind->SetName(http::webdav::cElement_propfind.Name(), dav_namespc);
	propfind->AddNamespace(dav_namespc);
	
	// <DAV:prop> element
	xmllib::XMLNode* prop = new xmllib::XMLNode(&xmldoc, propfind, http::webdav::cElement_prop.Name(), dav_namespc);
	
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
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
