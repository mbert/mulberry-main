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
	CWebDAVGet.h

	Author:			
	Description:	<describe the CWebDAVGet class here>
*/

#ifndef CWebDAVGet_H
#define CWebDAVGet_H

#include "CWebDAVRequestResponse.h"

using namespace http; 

namespace http {

namespace webdav {

class CWebDAVGet: public CWebDAVRequestResponse
{
public:
	CWebDAVGet(CWebDAVSession* session, const cdstring& ruri, bool head = false);
	CWebDAVGet(CWebDAVSession* session, const cdstring& ruri, const cdstring& lock, bool head = false);
	virtual ~CWebDAVGet();

	void SetData(CHTTPOutputData* response_data);
	void SetData(CHTTPOutputData* response_data, const cdstring& etag);

	const cdstring* GetNewETag() const;

	virtual uint32_t GetContentLength() const
	{
		return mHead ? 0 : mContentLength;		// Always zero to prevent attempt to read response
	}
	virtual bool GetChunked() const
	{
		return mHead ? false : mChunked;		// Always false to prevent attempt to read response
	}

protected:
	bool mHead;
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVGet_H
