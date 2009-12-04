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


// Header for common INET definitions

#include "CINETCommon.h"

// Strings
const char* cSpace =			" ";
const char* cCR =				"\r";
const char* cCRLF =				"\r\n";
const char* SPACE_CRLF =		" \r\n";
const char cTAG_CHAR = 			'A';
const char* cWILDCARD =			"*";
const char* cWILDCARD_NODIR =	"%";
const char* cWILDCARD_ALL =		"*%";
const char* cNIL =				"NIL";

// Tagged responses
const char* cOK =				"OK";
const char* cNO =				"NO";
const char* cBAD =				"BAD";

// Response Messages
const char* cPREAUTH =			"PREAUTH";
const char* cALERT =			"[ALERT]";
const char* cPARSE =			"[PARSE]";
const char* cREAD_ONLY =		"[READ-ONLY]";
const char* cREAD_WRITE =		"[READ-WRITE]";

// Commands
const char* cNOOP =				"NOOP";
const char* cLOGIN =			"LOGIN";
const char* cLOGOUT =			"LOGOUT";
const char* cSTARTTLS =			"STARTTLS";
const char* cAUTHENTICATE =		"AUTHENTICATE";
const char* cANONYMOUS =		"ANONYMOUS";
const char* cPLAIN =			"PLAIN";
const char* cEXTERNAL =			"EXTERNAL";
const char* cCAPABILITY =		"CAPABILITY";

// Unsolicitied
const char* cSTAR =				"*";
const char* cPLUS =				"+";
const char* cBYE =				"BYE";
