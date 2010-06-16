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
	CHTTPDefinitions.cpp

	Author:			
	Description:	<describe the CHTTPDefinitions class here>
*/

#include "CHTTPDefinitions.h"

namespace http
{

const unsigned short cHTTPServerPort = 80;
const unsigned short cHTTPServerPort_SSL = 443;

// RFC2616 ¤5.1.1 - Request Methods

const char* cRequestOPTIONS = "OPTIONS";
const char* cRequestGET = "GET";
const char* cRequestHEAD = "HEAD";
const char* cRequestPOST = "POST";
const char* cRequestPUT = "PUT";
const char* cRequestDELETE = "DELETE";
const char* cRequestTRACE = "TRACE";
const char* cRequestCONNECT = "CONNECT";

// 

const char* cHeaderDelimiter = ": ";

// RFC2616 ¤4.5 - General Header fields (only the ones we need)

const char* cHeaderConnection = "Connection";
	const char* cHeaderConnectionClose = "close";
const char* cHeaderDate = "Date";
const char* cHeaderTransferEncoding = "Transfer-Encoding";
	const char* cHeaderTransferEncodingChunked = "chunked";

// RFC2616 ¤5.3 - Request Header fields (only the ones we need)

const char* cHeaderAuthorization = "Authorization";
const char* cHeaderHost = "Host";
const char* cHeaderIfMatch = "If-Match";
const char* cHeaderIfNoneMatch = "If-None-Match";

// RFC2616 ¤6.2 - Response Header fields (only the ones we need)

const char* cHeaderETag = "ETag";
const char* cHeaderLocation = "Location";
const char* cHeaderServer = "Server";
const char* cHeaderWWWAuthenticate = "WWW-Authenticate";

// RFC2616 ¤7.1 - Entity Header fields (only the ones we need)

const char* cHeaderAllow = "Allow";
const char* cHeaderContentLength = "Content-Length";
const char* cHeaderContentType = "Content-Type";

namespace webdav 
{

// RFC2518 ¤ - WebDAV Request Methods

const char*	cRequestMKCOL = "MKCOL";
const char*	cRequestMOVE = "MOVE";
const char*	cRequestCOPY = "COPY";
const char*	cRequestPROPFIND = "PROPFIND";
const char*	cRequestPROPPATCH = "PROPPATCH";
const char*	cRequestLOCK = "LOCK";
const char*	cRequestUNLOCK = "UNLOCK";
const char*	cRequestREPORT = "REPORT";		// RFC3253
const char*	cRequestACL = "ACL";			// RFC3744

// RFC2518 ¤9 - Request Header fields (only the ones we need)

const char* cHeaderDAV = "DAV";
	const char* cHeaderDAV1 = "1";
	const char* cHeaderDAV2 = "2";
	const char* cHeaderDAVbis = "bis";
	const char* cHeaderDAVACL = "access-control";		// ACL extension RFC3744
const char* cHeaderDepth = "Depth";
	const char* cHeaderDepth0 = "0";
	const char* cHeaderDepth1 = "1";
	const char* cHeaderDepthInfinity = "infinity";
const char* cHeaderDestination = "Destination";
const char* cHeaderIf = "If";
const char* cHeaderForceAuthentication = "Force-Authentication";
const char* cHeaderLockToken = "Lock-Token";
const char* cHeaderOverwrite = "Overwrite";
	const char* cHeaderOverwriteTrue = "T";
	const char* cHeaderOverwriteFalse = "F";
const char* cHeaderTimeout = "Timeout";
	const char* cHeaderTimeoutSeconds = "Second-";
	const char* cHeaderTimeoutInfinite = "Infinite";
}

namespace caldav 
{

// drft-caldav ¤ - CalDAV Request Methods

const char*	cRequestMKCALENDAR = "MKCALENDAR";

// draft-caldav ¤ - Request Header fields (only the ones we need)

	const char* cHeaderCalendarAccess = "calendar-access";
	const char* cHeaderCalendarSchedule = "calendar-schedule";
	const char* cHeaderCalendarAutoSchedule = "calendar-auto-schedule";
}

namespace carddav 
{

// drft-carddav ¤ - CardDAV Request Methods

// draft-carddav ¤ - Request Header fields (only the ones we need)

	const char* cHeaderAddressBook = "addressbook";
}

}
