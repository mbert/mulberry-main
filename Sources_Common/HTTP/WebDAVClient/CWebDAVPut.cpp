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
	CWebDAVPut.cpp

	Author:			
	Description:	<describe the CWebDAVPut class here>
*/

#include "CWebDAVPut.h"

using namespace webdav; 

CWebDAVPut::CWebDAVPut(CWebDAVSession* session, const cdstring& ruri) :
	CWebDAVRequestResponse(session, eRequest_PUT, ruri)
{
}

CWebDAVPut::CWebDAVPut(CWebDAVSession* session, const cdstring& ruri, const cdstring& lock) :
	CWebDAVRequestResponse(session, eRequest_PUT, ruri, lock)
{
}

CWebDAVPut::~CWebDAVPut()
{
}

void CWebDAVPut::SetData(CHTTPInputData* request_data, CHTTPOutputData* response_data, bool new_item)
{
	mRequestData = request_data;
	mResponseData = response_data;

	// ETag should be '*' and we add If-None-Match header to ensure we do not overwrite something already there
	if (new_item)
	{
		mETag = "*";
		mETagMatch = false;
	}
}

void CWebDAVPut::SetData(CHTTPInputData* request_data, CHTTPOutputData* response_data, const cdstring& etag)
{
	mRequestData = request_data;
	mResponseData = response_data;

	// Must have matching ETag
	if (!etag.empty())
	{
		mETag = etag;
		mETagMatch = true;
	}
}

const cdstring* CWebDAVPut::GetNewETag() const
{
	// Get the ETag header from response headers
	if (GetResponseHeaders().count(cHeaderETag) != 0)
	{
		return &(*GetResponseHeaders().find(cHeaderETag)).second;
	}
	else
		return NULL;
}
