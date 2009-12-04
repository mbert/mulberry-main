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
	CCalendarStoreXML.cpp

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#include "CCalendarStoreXML.h"

namespace calstore
{

// calendar list

// XML DTD:
//
// <calendarlist>
//   <calendarnode type='directory'>
//		<name>...</name>
//		<displayname>...</displayname>
//		<calendarnode>*
//   </calendarnode>
// </calendarlist>
//
/*

	<!ELEMENT calendarlist	(calendarnode*) >
	<!ATTLIST calendarlist	version			CDATA	#REQUIRED
							datestamp       CDATA   "">

	<!ELEMENT calendarnode	(name, displayname?, last-sync?, webcal?, calendarnode*) >
	<!ATTLIST calendarnode	directory		(true|false) "false"
							has_expanded	(true|false) 
							inbox           (true|false) "false"
							outbox          (true|false) "false"
							display         (true|false) "false">

	<!ELEMENT name			(#PCDATA) >
 
	<!ELEMENT displayname	(#PCDATA) >
 
	<!ELEMENT last-sync		() >
	<!ATTLIST last-sync		value		CDATA	#REQUIRED >

	<!ELEMENT webcal		(url, refresh-interval, auto-publish, read-only) >

	<!ELEMENT url				(#PCDATA) >

	<!ELEMENT refresh-interval	() >
	<!ATTLIST refresh-interval	value		CDATA	#REQUIRED >

	<!ELEMENT auto-publish		() >
	<!ATTLIST auto-publish		value		(true|false)	#REQUIRED >

	<!ELEMENT read-only			() >
	<!ATTLIST read-only			value		(true|false)	#REQUIRED >

*/

const xmllib::XMLName cXMLElement_calendarlist("calendarlist");
const char* cXMLAttribute_version = "version";
const char* cXMLAttribute_datestamp = "datestamp";

const xmllib::XMLName cXMLElement_calendarnode("calendarnode");
const char* cXMLAttribute_directory = "directory";
const char* cXMLAttribute_has_expanded = "has_expanded";
const char*	cXMLAttribute_inbox = "inbox";
const char*	cXMLAttribute_outbox = "outbox";
const char* cXMLAttribute_displayhierachy = "display";

const xmllib::XMLName cXMLElement_name("name");

const xmllib::XMLName cXMLElement_displayname("displayname");

const xmllib::XMLName cXMLElement_lastsync("last-sync");

const xmllib::XMLName cXMLElement_webcal("webcal");

const xmllib::XMLName cXMLElement_webcal_url("url");

const xmllib::XMLName cXMLElement_webcal_refresh("refresh");

const xmllib::XMLName cXMLElement_webcal_interval("refresh-interval");

const xmllib::XMLName cXMLElement_webcal_autopublish("auto-publish");

const xmllib::XMLName cXMLElement_webcal_readonly("read-only");

}
