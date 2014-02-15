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


// Header for common SIEVE definitions

const tcp_port cSIEVEServerPort =		2000;
const tcp_port cSIEVEServerPort_SSL =	2001;

// Commands
const char* cHAVESPACE =				"GAVESPACE";
const char* cPUTSCRIPT =				"PUTSCRIPT";
const char* cLISTSCRIPTS =				"LISTSCRIPTS";
const char* cSETACTIVE =				"SETACTIVE";
const char* cGETSCRIPT =				"GETSCRIPT";
const char* cDELETESCRIPT =				"DELETESCRIPT";

// SIEVE extensions
const char* cREJECT =					"REJECT";
const char* cFILEINTO =					"FILEINTO";
const char* cVACATION =					"VACATION";
const char* cRELATIONAL =				"RELATIONAL";
const char* cIMAP4FLAGS =				"IMAP4FLAGS";

// Capabilities
const char* cSIEVE_IMPLEMENTATION =		"IMPLEMENTATION";
const char* cSIEVE_SASL =				"SASL";
const char* cSIEVE =					"SIEVE";

// Responses
const char* cACTIVE =					"ACTIVE";

// Response Codes
const char* cSIEVE_RESPONSECODE_AUTHTOOWEAK =				"AUTH-TOO-WEAK";
const char* cSIEVE_RESPONSECODE_ENCRYPTNEEDED =				"ENCRYPT-NEEDED";
const char* cSIEVE_RESPONSECODE_QUOTA =						"QUOTA";
const char* cSIEVE_RESPONSECODE_SASL =						"SASL";
const char* cSIEVE_RESPONSECODE_REFERRAL =					"REFERRAL";
const char* cSIEVE_RESPONSECODE_TRANSITIONNEEDED =			"TRANSITION-NEEDED";
const char* cSIEVE_RESPONSECODE_TRYLATER =					"TRYLATER";
