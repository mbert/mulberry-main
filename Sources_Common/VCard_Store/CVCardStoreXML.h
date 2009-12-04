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
	CVCardStoreXML.h

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#ifndef CVCardStoreXML_H
#define CVCardStoreXML_H

#include "XMLName.h"

namespace vcardstore {

extern const xmllib::XMLName cXMLElement_adbklist;
extern const char*			 cXMLAttribute_version;
extern const char*			 cXMLAttribute_datestamp;
extern const char*			 cXMLAttribute_has_expanded;

extern const xmllib::XMLName cXMLElement_adbknode;
extern const char*			 cXMLAttribute_adbk;
extern const char*			 cXMLAttribute_directory;
extern const char*			 cXMLAttribute_displayhierachy;

extern const xmllib::XMLName cXMLElement_name;

extern const xmllib::XMLName cXMLElement_displayname;

extern const xmllib::XMLName cXMLElement_lastsync;

}	// namespace calstore

#endif	// CVCardStoreXML_H
