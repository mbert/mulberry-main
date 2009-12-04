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

#include "CPOP3Common.h"

const tcp_port cPOP3ServerPort =		110;
const tcp_port cPOP3ServerPort_SSL =	995;

// Responses
const char* cPlusOK		= "+OK";
const char* cMinusERR	= "-ERR";

//const char* cTOP		= "TOP";
//const char* cUSER		= "USER";
const char* cSASL		= "SASL";
const char* cRESPCODES	= "RESP-CODES";
const char* cLOGINDELAY	= "LOGINDELAY";
const char* cPIPELINING	= "PIPELINING";
const char* cEXPIRE		= "EXPIRE";
//const char* cUIDL		= "UIDL";
const char* cIMPLEMENTATION	= "IMPLEMENTATION";

// Extended errors
//const char* cLOGINDELAY	= "LOGINDELAY";
const char* cINUSE		= "IN-USE";

// Commands
const char* cSTAT		= "STAT";
//const char* cLIST		= "LIST";		// Defined in CIMAPCommon.h
const char* cRETR		= "RETR";
const char* cDELE		= "DELE";
const char* cRSET		= "RSET";
const char* cQUIT		= "QUIT";

// Optional
const char* cTOP		= "TOP";
const char* cUIDL		= "UIDL";
const char* cUSER		= "USER";
const char* cPASS		= "PASS";
const char* cAPOP		= "APOP";

// Extensions
const char* cCAPA		= "CAPA";
const char* cSTLS		= "STLS";
const char* cAUTH		= "AUTH";
