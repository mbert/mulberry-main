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
	CWebDAVACL.h

	Author:			
	Description:	<describe the CWebDAVACL class here>
*/

#ifndef CWebDAVACL_H
#define CWebDAVACL_H

#include "CWebDAVRequestResponse.h"

#include "CAdbkACL.h"
#include "CCalendarACL.h"

#include <vector>


using namespace http; 

namespace http {

namespace webdav {

class CWebDAVACL: public CWebDAVRequestResponse
{
public:
	CWebDAVACL(CWebDAVSession* session, const cdstring& ruri, const CAdbkACLList* acls);
	CWebDAVACL(CWebDAVSession* session, const cdstring& ruri, const CCalendarACLList* acls);
	virtual ~CWebDAVACL();

protected:
	const CAdbkACLList*		mAdbkACLs;
	const CCalendarACLList*	mCalACLs;

	void	InitRequestData();
	
	void	GenerateXML(std::ostream& os);
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVACL_H
