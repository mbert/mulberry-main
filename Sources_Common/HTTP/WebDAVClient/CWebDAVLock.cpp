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
	CWebDAVLock.cpp

	Author:			
	Description:	<describe the CWebDAVLock class here>
*/

#include "CWebDAVLock.h"

#include "CHTTPDataString.h"
#include "CWebDAVDefinitions.h"
#include "XMLDocument.h"
#include "XMLNode.h"

#include <strstream>

using namespace webdav; 

CWebDAVLock::CWebDAVLock(CWebDAVSession* session, const cdstring& ruri, EWebDAVDepth depth, EWebDAVLockScope scope, const cdstring& owner, unsigned long timeout, EResourceExists exists) :
	CWebDAVRequestResponse(session, eRequest_LOCK, ruri)
{
	mDepth = depth;
	mScope = scope;
	mOwner = owner;
	mTimeout = timeout;
	
	// Do appropriate etag based on exists
	switch(exists)
	{
	case eResourceMustExist:
		mETag = "*";
		mETagMatch = true;
		break;
	case eResourceMustNotExist:
		mETag = "*";
		mETagMatch = false;
		break;
	case eResourceMayExist:
		break;
	}
	
	InitRequestData();
}

CWebDAVLock::~CWebDAVLock()
{
	// We own the request data and will delete it here
	if (mRequestData != NULL)
	{
		delete mRequestData;
		mRequestData = NULL;
	}
}

static const char* cDepthMap[] = {cHeaderDepth0, cHeaderDepth1, cHeaderDepthInfinity};

void CWebDAVLock::WriteHeaderToStream(std::ostream& os)
{
	// Do default
	CWebDAVRequestResponse::WriteHeaderToStream(os);

	// Add depth header
	os << cHeaderDepth << cHeaderDelimiter << cDepthMap[mDepth] << net_endl;

	// Add timeout header
	if (mTimeout == 0xFFFFFFFF)
		os << cHeaderTimeout << cHeaderDelimiter << cHeaderTimeoutInfinite << net_endl;
	else if (mTimeout > 0)
		os << cHeaderTimeout << cHeaderDelimiter << cHeaderTimeoutSeconds << cdstring(mTimeout) << net_endl;
}

void CWebDAVLock::InitRequestData()
{
	// Write XML info to a string
	std::ostrstream os;
	GenerateXML(os);
	os << std::ends;
	
	mRequestData = new CHTTPInputDataString(os.str(), "text/xml; charset=utf-8");
}

void CWebDAVLock::GenerateXML(std::ostream& os)
{
	using namespace xmllib;

	// Structure of document is:
	//
	// <DAV:lockinfo>
	//   <DAV:lockscope>
	//     <DAV:exclusive/> | <DAV:shared/>
	//   </DAV:lockscope>
	//   <DAV:locktype>
	//     <DAV:write/>
	//   </DAV:locktype>
	//   <DAV:owner>
	//     <<owner>>
	//   </DAV:owner>
	// </DAV:lockinfo>
	
	// Create document and get the root
	xmllib::XMLDocument xmldoc;
	
	// <DAV:lockinfo> element
	XMLNode* lockinfo = xmldoc.GetRoot();
	xmllib::XMLNamespace dav_namespc(cNamespace, "D");
	lockinfo->SetName("lockinfo", dav_namespc);
	lockinfo->AddNamespace(dav_namespc);
	
	// <DAV:lockscope> element
	xmllib::XMLNode* lockscope = new xmllib::XMLNode(&xmldoc, lockinfo, "lockscope", dav_namespc);
	
	// <DAV:exclusive/> | <DAV:shared/> element
	new XMLNode(&xmldoc, lockscope, (mScope == eExclusive) ? "exclusive" : "shared", dav_namespc);
	
	// <DAV:locktype> element
	xmllib::XMLNode* locktype = new xmllib::XMLNode(&xmldoc, lockinfo, "locktype", dav_namespc);
	
	// <DAV:write/> element
	new xmllib::XMLNode(&xmldoc, locktype, "write", dav_namespc);
	
	// <DAV:owner> element is optional
	if (!mOwner.empty())
	{
		// <DAV:owner> element
		xmllib::XMLNode* owner = new xmllib::XMLNode(&xmldoc, lockinfo, "owner", dav_namespc, mOwner);
	}

	// Now we have the complete document, so write it out (no indentation)
	xmldoc.Generate(os, false);
}

// Parse header
cdstring CWebDAVLock::GetLockToken() const
{
	cdstring result;

	// Get the Lock-Token header from response headers
	if (GetResponseHeaders().count(cHeaderLockToken) != 0)
	{
		// Get Coded-URL
		cdstring codedurl = (*GetResponseHeaders().find(cHeaderLockToken)).second;
		
		// Strip leading/trailing <>
		codedurl.trimspace();
		if (codedurl.compare_start("<") && codedurl.compare_end(">"))
		{
			result.assign(codedurl, 1, codedurl.length() - 2);
		}
		else
			result = codedurl;
	}

	return result;
}
