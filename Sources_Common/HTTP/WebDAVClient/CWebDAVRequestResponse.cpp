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
	CWebDAVRequestResponse.cpp

	Author:			
	Description:	<describe the CWebDAVRequestResponse class here>
*/

#include "CWebDAVRequestResponse.h"

#include "CHTTPDefinitions.h"
#include "CWebDAVSession.h"

using namespace http; 
using namespace webdav; 

CWebDAVRequestResponse::CWebDAVRequestResponse(const CWebDAVSession* session, ERequestMethod method, const cdstring& ruri) :
	CHTTPRequestResponse(session, method, ruri)
{
}

CWebDAVRequestResponse::CWebDAVRequestResponse(const CWebDAVSession* session, ERequestMethod method, const cdstring& ruri, const cdstring& lock) :
	CHTTPRequestResponse(session, method, ruri)
{
	mLock = lock;
}

CWebDAVRequestResponse::CWebDAVRequestResponse(const CWebDAVSession* session, ERequestMethod method, const cdstring& ruri, const cdstring& etag, bool etag_match) :
	CHTTPRequestResponse(session, method, ruri, etag, etag_match)
{
}

CWebDAVRequestResponse::~CWebDAVRequestResponse()
{
}

void CWebDAVRequestResponse::WriteHeaderToStream(std::ostream& os)
{
	// Do inherited
	CHTTPRequestResponse::WriteHeaderToStream(os);

	// Do Lock matching
	if (!mLock.empty())
	{
		// This is an untagged token - i.e. it applies to the resource being addressed
		os << cHeaderIf << cHeaderDelimiter << "(<" << mLock << ">)" << net_endl;
	}
}
