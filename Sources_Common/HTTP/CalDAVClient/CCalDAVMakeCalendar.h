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
	CCalDAVMakeCalendar.h

	Author:			
	Description:	<describe the CCalDAVMakeCalendar class here>
*/

#ifndef CCalDAVMakeCalendar_H
#define CCalDAVMakeCalendar_H

#include "CWebDAVRequestResponse.h"

using namespace http; 
using namespace webdav; 

namespace http {

namespace caldav {

class CCalDAVMakeCalendar: public CWebDAVRequestResponse
{
public:
	CCalDAVMakeCalendar(CWebDAVSession* session, const cdstring& ruri);
	virtual ~CCalDAVMakeCalendar();

};

}	// namespace webdav

}	// namespace http

#endif	// CCalDAVMakeCalendar_H
