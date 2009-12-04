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
	CWebDAVMove.cpp

	Author:			
	Description:	<describe the CWebDAVMove class here>
*/

#include "CWebDAVMove.h"

using namespace webdav; 

CWebDAVMove::CWebDAVMove(CWebDAVSession* session, const cdstring& ruri_old, const cdstring& absuri_new, bool overwrite) :
	CWebDAVRequestResponse(session, eRequest_MOVE, ruri_old), mMoveToAbsURI(absuri_new), mOverWrite(overwrite)
{
}

CWebDAVMove::~CWebDAVMove()
{
}

void CWebDAVMove::SetData()
{
	mRequestData = NULL;
	mResponseData = NULL;
}

void CWebDAVMove::SetData(const cdstring& etag)
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

void CWebDAVMove::WriteHeaderToStream(std::ostream& os)
{
	// Do default
	CWebDAVRequestResponse::WriteHeaderToStream(os);
	
	// Add Destination header
	os << cHeaderDestination << cHeaderDelimiter << mMoveToAbsURI << net_endl;
	
	// Add Overwrite header
	os << cHeaderOverwrite << cHeaderDelimiter << (mOverWrite ? cHeaderOverwriteTrue : cHeaderOverwriteFalse) << net_endl;
}
