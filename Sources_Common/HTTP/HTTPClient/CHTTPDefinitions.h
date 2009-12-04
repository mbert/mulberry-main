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
	CHTTPDefinitions.h

	Author:			
	Description:	<describe the CHTTPDefinitions class here>
*/

#ifndef CHTTPDefinitions_H
#define CHTTPDefinitions_H

namespace http {

extern const unsigned short cHTTPServerPort;
extern const unsigned short cHTTPServerPort_SSL;

// RFC2616 ¤5.1.1 - Request Methods

extern const char* cRequestOPTIONS;
extern const char* cRequestGET;
extern const char* cRequestHEAD;
extern const char* cRequestPOST;
extern const char* cRequestPUT;
extern const char* cRequestDELETE;
extern const char* cRequestTRACE;
extern const char* cRequestCONNECT;

// 

extern const char* cHeaderDelimiter;

// RFC2616 ¤4.5 - General Header fields (only the ones we need)

extern const char* cHeaderConnection;
	extern const char* cHeaderConnectionClose;
extern const char* cHeaderDate;
extern const char* cHeaderTransferEncoding;
	extern const char* cHeaderTransferEncodingChunked;

// RFC2616 ¤5.3 - Request Header fields (only the ones we need)

extern const char* cHeaderAuthorization;
extern const char* cHeaderHost;
extern const char* cHeaderIfMatch;
extern const char* cHeaderIfNoneMatch;

// RFC2616 ¤6.2 - Response Header fields (only the ones we need)

extern const char* cHeaderETag;
extern const char* cHeaderLocation;
extern const char* cHeaderServer;
extern const char* cHeaderWWWAuthenticate;

// RFC2616 ¤7.1 - Entity Header fields (only the ones we need)

extern const char* cHeaderAllow;
extern const char* cHeaderContentLength;
extern const char* cHeaderContentType;

// RFC2616 ¤6.1.1 Status Code and Reason Phrase

enum
{
	eStatus_Unknown = 0,
	eStatus_Continue = 100,
	eStatus_SwitchingProtocols = 101,
	eStatus_OK = 200,
	eStatus_Created = 201,
	eStatus_Accepted = 202,
	eStatus_NonAuthoritativeInformation = 203,
	eStatus_NoContent = 204,
	eStatus_ResetContent = 205,
	eStatus_PartialContent = 206,
	eStatus_MultipleChoices = 300,
	eStatus_MovedPermanently = 301,
	eStatus_Found = 302,
	eStatus_SeeOther = 303,
	eStatus_NotModified = 304,
	eStatus_UseProxy = 305,
	eStatus_TemporaryRedirect = 307,
	eStatus_BadRequest = 400,
	eStatus_Unauthorized = 401,
	eStatus_PaymentRequired = 402,
	eStatus_Forbidden = 403,
	eStatus_NotFound = 404,
	eStatus_MethodNotAllowed = 405,
	eStatus_NotAcceptable = 406,
	eStatus_ProxyAuthenticationRequired = 407,
	eStatus_RequestTimeout = 408,
	eStatus_Conflict = 409,
	eStatus_Gone = 410,
	eStatus_LengthRequired = 411,
	eStatus_PreconditionFailed = 412,
	eStatus_RequestEntityTooLarge = 413,
	eStatus_RequestURITooLarge = 414,
	eStatus_UnsupportedMediaType = 415,
	eStatus_RequestedRangeNotSatisfiable = 416,
	eStatus_ExpectationFailed = 417,
	eStatus_InternalServerError = 500,
	eStatus_NotImplemented = 501,
	eStatus_BadGateway = 502,
	eStatus_ServiceUnavailable = 503,
	eStatus_GatewayTimeout = 504,
	eStatus_HTTPVersionNotSupported = 505
	
};

namespace webdav 
{

// RFC2518 ¤ - WebDAV Request Methods

extern const char*	cRequestMKCOL;
extern const char*	cRequestMOVE;
extern const char*	cRequestCOPY;
extern const char*	cRequestPROPFIND;
extern const char*	cRequestPROPPATCH;
extern const char*	cRequestLOCK;
extern const char*	cRequestUNLOCK;
extern const char*	cRequestREPORT;		// RFC3253
extern const char*	cRequestACL;		// RFC3744

// RFC2518 ¤9 - Request Header fields (only the ones we need)

extern const char* cHeaderDAV;
	extern const char* cHeaderDAV1;
	extern const char* cHeaderDAV2;
	extern const char* cHeaderDAVbis;
	extern const char* cHeaderDAVACL;		// ACL extension RFC3744
extern const char* cHeaderDepth;
	enum EWebDAVDepth
	{
		eDepth0 = 0,
		eDepth1,
		eDepthInfinity
	};
	extern const char* cHeaderDepth0;
	extern const char* cHeaderDepth1;
	extern const char* cHeaderDepthInfinity;
extern const char* cHeaderDestination;
extern const char* cHeaderIf;
extern const char* cHeaderForceAuthentication;
extern const char* cHeaderLockToken;
extern const char* cHeaderOverwrite;
	extern const char* cHeaderOverwriteTrue;
	extern const char* cHeaderOverwriteFalse;
extern const char* cHeaderTimeout;
	extern const char* cHeaderTimeoutSeconds;
	extern const char* cHeaderTimeoutInfinite;

// RFC2518 ¤10 Status Code and Reason Phrase

enum
{
	eStatus_Processing = 102,
	eStatus_MultiStatus = 207,
	eStatus_UnprocessableEntity = 422,
	eStatus_Locked = 423,
	eStatus_FailedDependency = 424,
	eStatus_InsufficientStorage = 507
};

}

namespace caldav 
{

// draft-caldav ¤ - CalDAV Request Methods

extern const char*	cRequestMKCALENDAR;

// draft-caldav ¤ - Request Header fields (only the ones we need)

	extern const char* cHeaderCalendarAccess;
	extern const char* cHeaderCalendarSchedule;
	extern const char* cHeaderCalendarAutoSchedule;

}

namespace carddav 
{

// draft-carddav ¤ - CardDAV Request Methods

extern const char*	cRequestMKADBK;

// draft-carddav ¤ - Request Header fields (only the ones we need)

	extern const char* cHeaderAddressBook;

}

namespace slide 
{
	extern const char* cHeaderCyrusoftInheritable;
}

}	// namespace http

#endif	// CHTTPRequest_H
