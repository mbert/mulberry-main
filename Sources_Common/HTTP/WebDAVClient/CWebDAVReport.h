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
	CWebDAVReport.h

	Author:			
	Description:	<describe the CWebDAVReport class here>
*/

#ifndef CWebDAVReport_H
#define CWebDAVReport_H

#include "CWebDAVRequestResponse.h"

using namespace http; 

namespace http {

namespace webdav {

class CWebDAVReport: public CWebDAVRequestResponse
{
public:
	CWebDAVReport(CWebDAVSession* session, const cdstring& ruri);
	virtual ~CWebDAVReport();

	void SetOutput(CHTTPOutputData* response_data)
	{
		mResponseData = response_data;
	}
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVReport_H
