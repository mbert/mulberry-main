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

#ifndef __CIMSPCOMMON__MULBERRY__
#define __CIMSPCOMMON__MULBERRY__

#include "CTCPSocket.h"

extern const tcp_port cIMSPServerPort;
extern const tcp_port cIMSPServerPort_SSL;

// Commands
extern const char* cGET;
extern const char* cSET;
extern const char* cUNSET;
extern const char* cLOCKOPTION;
extern const char* cUNLOCKOPTION;

extern const char* cADDRESSBOOK;
extern const char* cCREATEADDRESSBOOK;
extern const char* cDELETEADDRESSBOOK;
extern const char* cRENAMEADDRESSBOOK;
extern const char* cLOCKADDRESSBOOK;
extern const char* cUNLOCKADDRESSBOOK;

extern const char* cFETCHADDRESS;
extern const char* cDELETEADDRESS;
extern const char* cSEARCHADDRESS;
extern const char* cSTOREADDRESS;

extern const char* cSETACLMAILBOX;
extern const char* cDELETEACLMAILBOX;
extern const char* cGETACLMAILBOX;
extern const char* cMYRIGHTSMAILBOX;

extern const char* cSETACLADDRESSBOOK;
extern const char* cDELETEACLADDRESSBOOK;
extern const char* cGETACLADDRESSBOOK;
extern const char* cMYRIGHTSADDRESSBOOK;

// Responses
extern const char* cOPTION;
extern const char* cACLMAILBOX;
extern const char* cACLADDRESSBOOK;

// Address Tags
extern const char* cADDRESS_NAME;
extern const char* cADDRESS_ALIAS;
extern const char* cADDRESS_EMAIL;
extern const char* cADDRESS_MEMBER;				// Simeon compat.
extern const char* cADDRESS_MEMBERS;			// Simeon compat.
extern const char* cADDRESS_CALENDAR;
extern const char* cADDRESS_COMPANY;
extern const char* cADDRESS_ADDRESS;
extern const char* cADDRESS_CITY;				// Simeon compat.
extern const char* cADDRESS_PHONE;				// Simeon compat.
extern const char* cADDRESS_PHONE_WORK;
extern const char* cADDRESS_PHONE_HOME;
extern const char* cADDRESS_FAX;
extern const char* cADDRESS_URLS;
extern const char* cADDRESS_HOME;				// Simeon compat.
extern const char* cADDRESS_NOTES;
extern const char* cADDRESS_GROUP;

#endif
