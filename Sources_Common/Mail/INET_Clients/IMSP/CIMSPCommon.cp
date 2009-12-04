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


// Header for common IMSP definitions

#include "CIMSPCommon.h"

const tcp_port cIMSPServerPort =		406;
const tcp_port cIMSPServerPort_SSL =	906;

// Commands
const char* cGET =						"GET";
const char* cSET =						"SET";
const char* cUNSET =					"UNSET";
const char* cLOCKOPTION =				"LOCK OPTION";
const char* cUNLOCKOPTION =				"UNLOCK OPTION";

const char* cADDRESSBOOK =				"ADDRESSBOOK";
const char* cCREATEADDRESSBOOK =		"CREATEADDRESSBOOK";
const char* cDELETEADDRESSBOOK =		"DELETEADDRESSBOOK";
const char* cRENAMEADDRESSBOOK =		"RENAMEADDRESSBOOK";
const char* cLOCKADDRESSBOOK =			"LOCK ADDRESSBOOK";
const char* cUNLOCKADDRESSBOOK =		"UNLOCK ADDRESSBOOK";

const char* cFETCHADDRESS =				"FETCHADDRESS";
const char* cDELETEADDRESS =			"DELETEADDRESS";
const char* cSEARCHADDRESS =			"SEARCHADDRESS";
const char* cSTOREADDRESS =				"STOREADDRESS";

const char* cSETACLMAILBOX =			"SETACL MAILBOX";
const char* cDELETEACLMAILBOX =			"DELETEACL MAILBOX";
const char* cGETACLMAILBOX =			"GETACL MAILBOX";
const char* cMYRIGHTSMAILBOX =			"MYRIGHTS MAILBOX";

const char* cSETACLADDRESSBOOK =		"SETACL ADDRESSBOOK";
const char* cDELETEACLADDRESSBOOK =		"DELETEACL ADDRESSBOOK";
const char* cGETACLADDRESSBOOK =		"GETACL ADDRESSBOOK";
const char* cMYRIGHTSADDRESSBOOK =		"MYRIGHTS ADDRESSBOOK";

// Responses
const char* cOPTION =					"OPTION";
const char* cACLMAILBOX =				"ACL MAILBOX";
const char* cACLADDRESSBOOK =			"ACL ADDRESSBOOK";

// Address Tags
const char* cADDRESS_NAME =				"name";
const char* cADDRESS_ALIAS =			"alias";
const char* cADDRESS_EMAIL =			"email";
const char* cADDRESS_MEMBER =			"member";			// Simeon compat.
const char* cADDRESS_MEMBERS =			"members";			// Simeon compat.
const char* cADDRESS_CALENDAR =			"calendar-address";
const char* cADDRESS_COMPANY =			"company";
const char* cADDRESS_ADDRESS =			"address";
const char* cADDRESS_CITY =				"city";				// Simeon compat.
const char* cADDRESS_PHONE =			"phone";			// Simeon compat.
const char* cADDRESS_PHONE_WORK =		"phone-work";
const char* cADDRESS_PHONE_HOME =		"phone-home";
const char* cADDRESS_FAX =				"fax";
const char* cADDRESS_HOME =				"home_page";		// Simeon compat.
const char* cADDRESS_URLS =				"urls";
const char* cADDRESS_NOTES =			"notes";
const char* cADDRESS_GROUP =			"group";			// Existence indicates a group
