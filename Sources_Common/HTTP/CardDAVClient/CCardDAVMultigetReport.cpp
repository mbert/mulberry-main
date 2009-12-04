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
	CCardDAVMultigetReport.cpp

	Author:			
	Description:	<describe the CCardDAVMultigetReport class here>
*/

#include "CCardDAVMultigetReport.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 
using namespace carddav; 

CCardDAVMultigetReport::CCardDAVMultigetReport(CWebDAVSession* session, const cdstring& ruri, const cdstrvect& hrefs) :
	CWebDAVReport(session, ruri)
{
	InitRequestData(hrefs);
}


CCardDAVMultigetReport::~CCardDAVMultigetReport()
{
}


void CCardDAVMultigetReport::InitRequestData(const cdstrvect& hrefs)
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os, hrefs);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CCardDAVMultigetReport::GenerateXML(std::ostream& os, const cdstrvect& hrefs)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <CardDAV:addressbook-multiget>
	//   <DAV:prop>
	//     <DAV:getetag>
	//     <CardDAV:address-data/>
	//   </DAV:prop>
	//   <DAV:href>...</DAV:href>
	//   ...
	// </CardDAV:addressbook-multiget>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;

	// <CardDAV:addressbook-multiget> element
	xmllib::XMLNode* multiget = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(webdav::cNamespace, "D");
	xmllib::XMLNamespace carddav_namespc(carddav::cNamespace, "C");
	multiget->SetName(cElement_adbkmultiget.Name(), carddav_namespc);
	multiget->AddNamespace(dav_namespc);
	multiget->AddNamespace(carddav_namespc);
	
	// <DAV:prop> element
	xmllib::XMLNode* prop = new xmllib::XMLNode(&xmldoc, multiget, cElement_prop.Name(), dav_namespc);
	
	// <DAV:getetag> element
	new xmllib::XMLNode(&xmldoc, prop, cProperty_getetag.Name(), dav_namespc);
	
	// <CardDAV:address-data> element
	new xmllib::XMLNode(&xmldoc, prop, cElement_adbkdata.Name(), carddav_namespc);
	
	// Do for each href
	for(cdstrvect::const_iterator iter = hrefs.begin(); iter != hrefs.end(); iter++)
	{
		// <DAV:href> element
		new xmllib::XMLNode(&xmldoc, multiget, cProperty_href.Name(), dav_namespc, *iter);
	}
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
