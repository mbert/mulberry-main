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
	CWebDAVDelete.cpp

	Author:			
	Description:	<describe the CWebDAVDelete class here>
*/

#include "CWebDAVDelete.h"

using namespace webdav; 

CWebDAVDelete::CWebDAVDelete(CWebDAVSession* session, const cdstring& ruri) :
	CWebDAVRequestResponse(session, eRequest_DELETE, ruri)
{
}

CWebDAVDelete::~CWebDAVDelete()
{
}

void CWebDAVDelete::SetData()
{
	mRequestData = NULL;
	mResponseData = NULL;
}

void CWebDAVDelete::SetData(const cdstring& etag)
{
	mRequestData = NULL;
	mResponseData = NULL;

	// Must have matching ETag
	if (!etag.empty())
	{
		mETag = etag;
		mETagMatch = true;
	}
}

