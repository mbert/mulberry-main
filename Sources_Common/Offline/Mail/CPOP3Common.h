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


// Header for common POP3 definitions

#ifndef __CPOP3COMMON__MULBERRY__
#define __CPOP3COMMON__MULBERRY__

#include "CTCPSocket.h"

extern const tcp_port cPOP3ServerPort;
extern const tcp_port cPOP3ServerPort_SSL;

// Responses
extern const char* cPlusOK;
extern const char* cMinusERR;

//extern const char* cTOP;
//extern const char* cUSER;
extern const char* cSASL;
extern const char* cRESPCODES;
extern const char* cLOGINDELAY;
extern const char* cPIPELINING;
extern const char* cEXPIRE;
//extern const char* cUIDL;
extern const char* cIMPLEMENTATION;

// Extended errors
//extern const char* cLOGINDELAY;
extern const char* cINUSE;

// Commands
extern const char* cSTAT;
extern const char* cLIST;
extern const char* cRETR;
extern const char* cDELE;
extern const char* cRSET;
extern const char* cQUIT;

// Optional
extern const char* cTOP;
extern const char* cUIDL;
extern const char* cUSER;
extern const char* cPASS;
extern const char* cAPOP;

// Extensions
extern const char* cCAPA;
extern const char* cSTLS;
extern const char* cAUTH;

#endif
