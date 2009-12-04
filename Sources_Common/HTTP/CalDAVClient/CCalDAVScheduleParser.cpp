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
	CCalDAVScheduleParser.cpp

	Author:			
	Description:	<describe the CCalDAVScheduleParser class here>
*/

#include "CCalDAVScheduleParser.h"

#include "CHTTPUtils.h"
#include "CWebDAVDefinitions.h"
#include "XMLNode.h"

#include "CICalendar.h"

#include <ostream>
#include <strstream>

using namespace http; 
using namespace webdav; 
using namespace caldav; 
using namespace xmllib; 

CCalDAVScheduleParser::CCalDAVScheduleParser(iCal::CITIPScheduleResultsList& results) :
	mResults(results)
{
}

CCalDAVScheduleParser::~CCalDAVScheduleParser()
{
}

void CCalDAVScheduleParser::Parse(const XMLNode* root_node)
{
	// Must have a node
	if (root_node == NULL)
		return;
	
	// Verify that the node is the correct element <CALDAV:schedule-response>
	if (!root_node->CompareFullName(cElement_scheduleresponse))
		return;
	
	// Node is the right type, so iterator over all child response nodes and process each one
	for(XMLNodeList::const_iterator iter = root_node->Children().begin(); iter != root_node->Children().end(); iter++)
		ParseScheduleResponse(*iter);
}

void CCalDAVScheduleParser::ParseScheduleResponse(const xmllib::XMLNode* response)
{
	// Verify that the node is the correct element <CALDAV:response>
	if (!response->CompareFullName(cElement_caldavresponse))
		return;
	
	
	// Node is the right type, so iterate over all child response nodes and process each one
	cdstring recipient;
	cdstring status;
	iCal::CICalendar cal;
	bool calendar = false;
	for(XMLNodeList::const_iterator iter1 = response->Children().begin(); iter1 != response->Children().end(); iter1++)
	{
		// Is it the recipient
		if ((*iter1)->CompareFullName(cElement_recipient))
		{
			for(XMLNodeList::const_iterator iter2 = (*iter1)->Children().begin(); iter2 != (*iter1)->Children().end(); iter2++)
			{
				if ((*iter2)->CompareFullName(cElement_href))
				{
					recipient = (*iter2)->Data();
					break;
				}
			}
		}
		
		// Is it request-status
		else if ((*iter1)->CompareFullName(cElement_requeststatus))
		{
			status = (*iter1)->Data();
		}
		
		// Is it calendar-data
		else if ((*iter1)->CompareFullName(cElement_calendardata))
		{
			ParseCalendarData(*iter1, cal);
			calendar = true;
		}
	}
	
	if (calendar)
		mResults.push_back(iCal::CITIPScheduleResults(recipient, status, cal));
	else
		mResults.push_back(iCal::CITIPScheduleResults(recipient, status));
}

void CCalDAVScheduleParser::ParseCalendarData(const xmllib::XMLNode* response, iCal::CICalendar& cal)
{
	// Read calendar component(s) from file
	std::istrstream is(response->Data().c_str());
	cal.Parse(is);
}
