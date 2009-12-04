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
	CVCardStoreXML.cpp

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#include "CVCardStoreXML.h"

namespace vcardstore
{

// address book list

// XML DTD:
//
// <addressbooklist>
//   <addressbooknode adbk="true" directory="true">
//		<name>...</name>
//		<addressbooknode>*
//   </addressbooknode>
// </addressbooklist>
//
/*

	<!ELEMENT addressbooklist	(addressbooknode*) >
	<!ATTLIST addressbooklist	version			CDATA	#REQUIRED
							    datestamp       CDATA   "" >

	<!ELEMENT addressbooknode	(name, displayname?, last-sync?, webcal?, addressbooknode*) >
	<!ATTLIST addressbooknode	adbk			(true|false) "false"
								directory		(true|false) "false"
								has_expanded	(true|false)
								display         (true|false) "false" >

	<!ELEMENT name			(#PCDATA) >

	<!ELEMENT last-sync		() >
	<!ATTLIST last-sync		value		CDATA	#REQUIRED >

*/

const xmllib::XMLName cXMLElement_adbklist("addressbooklist");
const char* cXMLAttribute_version = "version";
const char* cXMLAttribute_datestamp = "datestamp";

const xmllib::XMLName cXMLElement_adbknode("addressbooknode");
const char* cXMLAttribute_adbk = "adbk";
const char* cXMLAttribute_directory = "directory";
const char* cXMLAttribute_has_expanded = "has_expanded";
const char* cXMLAttribute_displayhierachy = "display";

const xmllib::XMLName cXMLElement_name("name");

const xmllib::XMLName cXMLElement_displayname("displayname");

const xmllib::XMLName cXMLElement_lastsync("last-sync");

}
