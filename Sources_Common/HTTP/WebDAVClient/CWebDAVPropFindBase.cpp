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
	CWebDAVPropFindBase.cpp

	Author:			
	Description:	<describe the CWebDAVPropFindBase class here>
*/

#include "CWebDAVPropFindBase.h"

using namespace webdav; 

CWebDAVPropFindBase::CWebDAVPropFindBase(CWebDAVSession* session, const cdstring& ruri, EWebDAVDepth depth) :
	CWebDAVRequestResponse(session, eRequest_PROPFIND, ruri)
{
	mDepth = depth;
}

CWebDAVPropFindBase::~CWebDAVPropFindBase()
{
	// We own the request data and will delete it here
	if (mRequestData != NULL)
	{
		delete mRequestData;
		mRequestData = NULL;
	}
}

static const char* cDepthMap[] = {cHeaderDepth0, cHeaderDepth1, cHeaderDepthInfinity};

void CWebDAVPropFindBase::WriteHeaderToStream(std::ostream& os)
{
	// Do default
	CWebDAVRequestResponse::WriteHeaderToStream(os);
	
	// Add depth header
	os << cHeaderDepth << cHeaderDelimiter << cDepthMap[mDepth] << net_endl;
}
