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
 CWebDAVSyncReport.cpp
 
 Author:			
 Description:	<describe the CWebDAVSyncReport class here>
 */

#include "CWebDAVSyncReport.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 

CWebDAVSyncReport::CWebDAVSyncReport(CWebDAVSession* session, const cdstring& ruri, const cdstring& sync_token) :
	CWebDAVReport(session, ruri)
{
	InitRequestData(sync_token);
}


CWebDAVSyncReport::~CWebDAVSyncReport()
{
}


void CWebDAVSyncReport::InitRequestData(const cdstring& sync_token)
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os, sync_token);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVSyncReport::WriteHeaderToStream(std::ostream& os)
{
	// Do default
	CWebDAVRequestResponse::WriteHeaderToStream(os);
	
	// Add depth header
	os << cHeaderDepth << cHeaderDelimiter << "1" << net_endl;
}

void CWebDAVSyncReport::GenerateXML(std::ostream& os, const cdstring& sync_token)
{
	using namespace xmllib;
	
	// Structure of document is:
	//
	// <DAV:sync-collection>
	//   <DAV:sync-token>...</DAV:sync-token>
	//   <DAV:prop>
	//     <DAV:getetag>
	//   </DAV:prop>
	// </DAV:sync-collection>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:sync-collection> element
	xmllib::XMLNode* sync = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(webdav::cNamespace, "D");
	sync->SetName(cElement_sync_collection.Name(), dav_namespc);
	sync->AddNamespace(dav_namespc);
	
	// <DAV:sync-token> element
	new xmllib::XMLNode(&xmldoc, sync, cElement_sync_token.Name(), dav_namespc, sync_token);
	
	// <DAV:prop> element
	xmllib::XMLNode* prop = new xmllib::XMLNode(&xmldoc, sync, cElement_prop.Name(), dav_namespc);
	
	// <DAV:getetag> element
	new xmllib::XMLNode(&xmldoc, prop, cProperty_getetag.Name(), dav_namespc);
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
