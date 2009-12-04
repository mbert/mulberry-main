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
	CWebDAVACL.cpp

	Author:			
	Description:	<describe the CWebDAVACL class here>
*/

#include "CWebDAVACL.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "CWebDAVSession.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 

CWebDAVACL::CWebDAVACL(CWebDAVSession* session, const cdstring& ruri, const CAdbkACLList* acls) :
	CWebDAVRequestResponse(session, eRequest_ACL, ruri)
{
	mAdbkACLs = acls;
	mCalACLs = NULL;

	InitRequestData();
}

CWebDAVACL::CWebDAVACL(CWebDAVSession* session, const cdstring& ruri, const CCalendarACLList* acls) :
	CWebDAVRequestResponse(session, eRequest_ACL, ruri)
{
	mAdbkACLs = NULL;
	mCalACLs = acls;

	InitRequestData();
}

CWebDAVACL::~CWebDAVACL()
{
}

void CWebDAVACL::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVACL::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:acl>
	//   <DAV:ace>
	//     <S:inheritable xmlns:S="http://jakarta.apache.org/slide/"> / <S:non-inheritable xmlns:S="http://jakarta.apache.org/slide/">
	//     <DAV:principal>...</DAV:principal>
	//	   <DAV:grant>...</DAV:grant>
	//   </DAV:ace>
	//   ...
	// </DAV:acl>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:acl> element
	xmllib::XMLNode* acl = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(cNamespace, "D");
	acl->SetName("acl", dav_namespc);
	acl->AddNamespace(dav_namespc);
	xmllib::XMLNamespace slide_namespc(slide::cNamespace, "S");
	if (static_cast<const CWebDAVSession*>(GetSession())->HasDAVVersion(CWebDAVSession::eCyrusoftInheritable))
	{
		xmldoc.AddNamespace(slide_namespc);
		acl->AddNamespace(slide_namespc);
	}
	
	// Do for each ACL
	if (mAdbkACLs != NULL)
	{
		for(CAdbkACLList::const_iterator iter = mAdbkACLs->begin(); iter != mAdbkACLs->end(); iter++)
		{
			// Cannot do if change not allowed
			if (!(*iter).CanChange())
				continue;
			
			// <DAV:ace> element
			(*iter).GenerateACE(&xmldoc, acl, static_cast<const CWebDAVSession*>(GetSession())->HasDAVVersion(CWebDAVSession::eCyrusoftInheritable));
		}
	}
	if (mCalACLs != NULL)
	{
		for(CCalendarACLList::const_iterator iter = mCalACLs->begin(); iter != mCalACLs->end(); iter++)
		{
			// Cannot do if change not allowed
			if (!(*iter).CanChange())
				continue;
			
			// <DAV:ace> element
			(*iter).GenerateACE(&xmldoc, acl, static_cast<const CWebDAVSession*>(GetSession())->HasDAVVersion(CWebDAVSession::eCyrusoftInheritable));
		}
	}
	
	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}
