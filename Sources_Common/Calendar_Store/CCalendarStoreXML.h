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
	CCalendarStoreXML.h

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#ifndef CCalendarStoreXML_H
#define CCalendarStoreXML_H

#include "XMLName.h"

namespace calstore {

extern const xmllib::XMLName cXMLElement_calendarlist;
extern const char*			 cXMLAttribute_version;
extern const char*			 cXMLAttribute_datestamp;
extern const char*			 cXMLAttribute_has_expanded;

extern const xmllib::XMLName cXMLElement_calendarnode;
extern const char*			 cXMLAttribute_directory;
extern const char*			 cXMLAttribute_inbox;
extern const char*			 cXMLAttribute_outbox;
extern const char*			 cXMLAttribute_displayhierachy;

extern const xmllib::XMLName cXMLElement_name;

extern const xmllib::XMLName cXMLElement_displayname;

extern const xmllib::XMLName cXMLElement_lastsync;

extern const xmllib::XMLName cXMLElement_webcal;

extern const xmllib::XMLName cXMLElement_webcal_url;

extern const xmllib::XMLName cXMLElement_webcal_refresh;

extern const xmllib::XMLName cXMLElement_webcal_interval;

extern const xmllib::XMLName cXMLElement_webcal_autopublish;

extern const xmllib::XMLName cXMLElement_webcal_readonly;

}	// namespace calstore

#endif	// CCalendarStoreXML_H
