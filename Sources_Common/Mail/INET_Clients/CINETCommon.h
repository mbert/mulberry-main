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

#ifndef __CINETCOMMON__MULBERRY__
#define __CINETCOMMON__MULBERRY__

// Strings
extern const char* cSpace;
extern const char* cCR;
extern const char* cCRLF;
extern const char* SPACE_CRLF;
extern const char cTAG_CHAR;
extern const char* cWILDCARD;
extern const char* cWILDCARD_NODIR;
extern const char* cWILDCARD_ALL;
extern const char* cNIL;

// Tagged responses
extern const char* cOK;
extern const char* cNO;
extern const char* cBAD;

// Response Messages
extern const char* cPREAUTH;
extern const char* cALERT;
extern const char* cPARSE;
extern const char* cREAD_ONLY;
extern const char* cREAD_WRITE;

// Commands
extern const char* cNOOP;
extern const char* cLOGIN;
extern const char* cLOGOUT;
extern const char* cSTARTTLS;
extern const char* cAUTHENTICATE;
extern const char* cANONYMOUS;
extern const char* cPLAIN;
extern const char* cEXTERNAL;
extern const char* cCAPABILITY;

// Unsolicitied
extern const char* cSTAR;
extern const char* cPLUS;
extern const char* cBYE;

const int cINETBufferLen = 8192;

#endif
