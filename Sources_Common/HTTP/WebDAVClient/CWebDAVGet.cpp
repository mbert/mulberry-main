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
	CWebDAVGet.cpp

	Author:			
	Description:	<describe the CWebDAVGet class here>
*/

#include "CWebDAVGet.h"

using namespace webdav; 

CWebDAVGet::CWebDAVGet(CWebDAVSession* session, const cdstring& ruri, bool head) :
	CWebDAVRequestResponse(session, head ? eRequest_HEAD : eRequest_GET, ruri), mHead(head)
{
}

CWebDAVGet::CWebDAVGet(CWebDAVSession* session, const cdstring& ruri, const cdstring& lock, bool head) :
	CWebDAVRequestResponse(session, head ? eRequest_HEAD : eRequest_GET, ruri, lock), mHead(head)
{
}

CWebDAVGet::~CWebDAVGet()
{
}

void CWebDAVGet::SetData(CHTTPOutputData* response_data)
{
	mRequestData = NULL;
	mResponseData = response_data;
}

void CWebDAVGet::SetData(CHTTPOutputData* response_data, const cdstring& etag)
{
	mRequestData = NULL;
	mResponseData = response_data;

	// Must have matching ETag
	if (!etag.empty())
	{
		mETag = etag;
		mETagMatch = false;
	}
}

const cdstring* CWebDAVGet::GetNewETag() const
{
	// Get the ETag header from response headers
	if (GetResponseHeaders().count(cHeaderETag) != 0)
	{
		return &(*GetResponseHeaders().find(cHeaderETag)).second;
	}
	else
		return NULL;
}
