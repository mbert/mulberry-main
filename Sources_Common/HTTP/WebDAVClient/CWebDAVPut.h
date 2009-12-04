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
	CWebDAVPut.h

	Author:			
	Description:	<describe the CWebDAVPut class here>
*/

#ifndef CWebDAVPut_H
#define CWebDAVPut_H

#include "CWebDAVRequestResponse.h"

using namespace http; 

namespace http {

namespace webdav {

class CWebDAVPut: public CWebDAVRequestResponse
{
public:
	CWebDAVPut(CWebDAVSession* session, const cdstring& ruri);
	CWebDAVPut(CWebDAVSession* session, const cdstring& ruri, const cdstring& lock);
	virtual ~CWebDAVPut();

	void SetData(CHTTPInputData* request_data, CHTTPOutputData* response_data, bool new_item = true);
	void SetData(CHTTPInputData* request_data, CHTTPOutputData* response_data, const cdstring& etag);

	const cdstring* GetNewETag() const;
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVPut_H
