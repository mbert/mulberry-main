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
	CCalDAVSchedule.cpp

	Author:			
	Description:	<describe the CCalDAVSchedule class here>
*/

#include "CCalDAVSchedule.h"

#include "CWebDAVDefinitions.h"

using namespace caldav; 

CCalDAVSchedule::CCalDAVSchedule(CWebDAVSession* session, const cdstring& ruri, const cdstring& originator, const cdstrvect& recipients) :
	CWebDAVRequestResponse(session, eRequest_POST, ruri)
{
	mOriginator = originator;
	mRecipients = recipients;
}


CCalDAVSchedule::~CCalDAVSchedule()
{
}

void CCalDAVSchedule::WriteHeaderToStream(std::ostream& os)
{
	// Do default
	CWebDAVRequestResponse::WriteHeaderToStream(os);
	
	// Add Originator header
	os << cHeaderOriginator << cHeaderDelimiter << mOriginator << net_endl;
	
	// Add Recipient header
	os << cHeaderRecipient << cHeaderDelimiter;
	for(cdstrvect::const_iterator iter = mRecipients.begin(); iter != mRecipients.end(); iter++)
	{
		if (iter != mRecipients.begin())
			os << ", ";
		os << *iter;
	}
	os << net_endl;
}
