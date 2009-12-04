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
	CWebDAVPropNames.cpp

	Author:			
	Description:	<describe the CWebDAVPropNames class here>
*/

#include "CWebDAVPropNames.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav;

CWebDAVPropNames::CWebDAVPropNames(CWebDAVSession* session, const cdstring& ruri, EWebDAVDepth depth) :
	CWebDAVPropFindBase(session, ruri, depth)
{
	InitRequestData();
}


CWebDAVPropNames::~CWebDAVPropNames()
{
}

void CWebDAVPropNames::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;

	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVPropNames::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:propfind>
	//   <DAV:propname/>
	// </DAV:propfind>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:propfind> element
	xmllib::XMLNode* propfind = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(http::webdav::cNamespace, "D");
	propfind->SetName(http::webdav::cElement_propfind.Name(), dav_namespc);
	propfind->AddNamespace(dav_namespc);
	
	// <DAV:prop> element
	xmllib::XMLNode* propname = new xmllib::XMLNode(&xmldoc, propfind, cElement_propname.Name(), dav_namespc);
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
