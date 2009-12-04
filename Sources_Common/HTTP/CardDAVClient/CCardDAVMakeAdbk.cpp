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
	CCardDAVMakeAdbk.cpp

	Author:			
	Description:	<describe the CCardDAVMakeAdbk class here>
*/

#include "CCardDAVMakeAdbk.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 
using namespace carddav; 

CCardDAVMakeAdbk::CCardDAVMakeAdbk(CWebDAVSession* session, const cdstring& ruri) :
	CWebDAVMakeCollection(session, ruri)
{
	InitRequestData();
}


CCardDAVMakeAdbk::~CCardDAVMakeAdbk()
{
}

void CCardDAVMakeAdbk::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;

	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CCardDAVMakeAdbk::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:mkcol>
	//   <DAV:set>
	//     <DAV:prop>
	//       <DAV:resource-type>
	//         <DAV:collection/>
	//         <CARDDAV:addressbook/>
	//       </DAV:resource-type>
	//     </DAV:prop>
	//   </DAV:set>
	// </DAV:mkcol>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:mkcol> element
	xmllib::XMLNode* mkcol = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(http::webdav::cNamespace, "D");
	xmllib::XMLNamespace carddav_namespc(carddav::cNamespace, "C");
	mkcol->SetName(http::webdav::cElement_mkcol.Name(), dav_namespc);
	mkcol->AddNamespace(dav_namespc);
	
	// <DAV:set> element
	xmllib::XMLNode* set = new xmllib::XMLNode(&xmldoc, mkcol, cElement_set.Name(), dav_namespc);
	
	// <DAV:prop> element
	xmllib::XMLNode* prop = new xmllib::XMLNode(&xmldoc, set, cElement_prop.Name(), dav_namespc);
	
	// <DAV:resourcetype> element
	xmllib::XMLNode* resourcetype = new xmllib::XMLNode(&xmldoc, prop, cProperty_resourcetype.Name(), dav_namespc);
	
	// <DAV:collection> element
	new xmllib::XMLNode(&xmldoc, resourcetype, cProperty_collection.Name(), dav_namespc);
	
	// <DAV:resourcetype> element
	new xmllib::XMLNode(&xmldoc, resourcetype, cProperty_carddavadbk.Name(), carddav_namespc);
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
