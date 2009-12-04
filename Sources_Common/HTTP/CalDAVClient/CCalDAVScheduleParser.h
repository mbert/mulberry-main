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
	CCalDAVScheduleParser.h

	Author:			
	Description:	<describe the CCalDAVScheduleParser class here>
*/

#ifndef CCalDAVScheduleParser_H
#define CCalDAVScheduleParser_H

#include "CWebDAVXMLResponseParser.h"

#include "CITIPScheduleResults.h"
#include "cdstring.h"

using namespace http; 
using namespace webdav; 

namespace iCal
{
	class CICalendar;
}

namespace http {

namespace caldav {

class CCalDAVScheduleParser: public CWebDAVXMLResponseParser
{
public:
	CCalDAVScheduleParser(iCal::CITIPScheduleResultsList& results);
	virtual ~CCalDAVScheduleParser();

protected:
	iCal::CITIPScheduleResultsList&	mResults;

	// Parse the response element down to the properties

	virtual void Parse(const xmllib::XMLNode* root_node);
	virtual void ParseScheduleResponse(const xmllib::XMLNode* response);
	
	// Calendar data
	virtual void ParseCalendarData(const xmllib::XMLNode* response,
								   iCal::CICalendar& cal);

};

}	// namespace caldav

}	// namespace http

#endif	// CCalDAVScheduleParser_H
