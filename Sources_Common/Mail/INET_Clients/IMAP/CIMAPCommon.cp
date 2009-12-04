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


// Header for common IMAP definitions

#include "CIMAPCommon.h"

const tcp_port cIMAPServerPort =		143;
const tcp_port cIMAPServerPort_SSL =	993;

// Response Messages
const char* cPERMANENTFLAGS =			"[PERMANENTFLAGS ";
const char* cTRYCREATE =				"[TRYCREATE]";
const char* cUIDVALIDITY =				"[UIDVALIDITY ";
const char* cUIDNEXT =					"[UIDNEXT ";
const char* cUNSEEN =					"[UNSEEN ";
const char* cNOSUCHMAILBOX =			"No such destination mailbox";
const int cNOSUCHMAILBOX_LENGTH =		27;
const char* cOPENFAILED =				"Open failed";
const int cOPENFAILED_LENGTH =			11;

// Capability responses
const char* cIMAP4 =					"IMAP4";
const char* cIMAP4REV1 =				"IMAP4REV1";
const char* cIMAP_ACL =					"ACL";
const char* cIMAP_QUOTA =				"QUOTA";
const char* cIMAP_LITERAL_PLUS =		"LITERAL+";
const char* cIMAP_NAMESPACE =			"NAMESPACE";
const char* cIMAP_UIDPLUS =				"UIDPLUS";
const char* cIMAP_UNSELECT =			"UNSELECT";
const char* cIMAP_SORT =				"SORT";
const char* cIMAP_THREAD_SUBJECT =		"THREAD=ORDEREDSUBJECT";
const char* cIMAP_THREAD_REFERENCES =	"THREAD=REFERENCES";
const char* cIMAP_AUTHLOGIN =			"AUTH=LOGIN";
const char* cIMAP_AUTHPLAIN =			"AUTH=PLAIN";
const char* cIMAP_AUTHANON =			"AUTH=ANONYMOUS";

// Commands
const char* cSELECT =					"SELECT";
const char* cEXAMINE =					"EXAMINE";
const char* cBBOARD =					"BBOARD";
const char* cFINDMAILBOXES =			"FIND MAILBOXES *";
const char* cFINDALLMAILBOXES =			"FIND ALL.MAILBOXES";
const char* cFINDBBOARDS =				"FIND BBOARDS";
const char* cLIST =						"LIST";
const char* cLSUB =						"LSUB";
const char* cCHECK =					"CHECK";
const char* cSTATUS =					"STATUS";
const char* cEXPUNGE =					"EXPUNGE";
const char* cCLOSE =					"CLOSE";
const char* cCOPY =						"COPY";
const char* cFETCH =					"FETCH";
const char* cSTORE =					"STORE";
const char* cPARTIAL =					"PARTIAL";
const char* cSEARCH =					"SEARCH";
const char* cCREATE =					"CREATE";
const char* cIMAP_DELETE =				"DELETE";
const char* cRENAME =					"RENAME";
const char* cAPPEND =					"APPEND";
const char* cSUBSCRIBEMBOX =			"SUBSCRIBE MAILBOX";
const char* cUNSUBSCRIBEMBOX =			"UNSUBSCRIBE MAILBOX";
const char* cSUBSCRIBEMBOX4 =			"SUBSCRIBE";
const char* cUNSUBSCRIBEMBOX4 =			"UNSUBSCRIBE";

const char* cUIDCOPY					= "UID COPY";
const char* cUIDFETCH					= "UID FETCH";
const char* cUIDSTORE					= "UID STORE";
const char* cUIDSEARCH					= "UID SEARCH";
const char* cUIDEXPUNGE					= "UID EXPUNGE";

// Unsolicitied
const char* cFLAGS =					"FLAGS";
const char* cMAILBOX =					"MAILBOX";

// Numeric
const char* cMSGEXISTS =				"EXISTS";
const char* cMSGRECENT =				"RECENT";
const char* cMSGEXPUNGE =				"EXPUNGE";
const char* cMSGSTORE =					"STORE";
const char* cMSGFETCH =					"FETCH";
const char* cMSGCOPY =					"COPY";

// Paramaters
const char* cINBOX =					"INBOX";
const char* cENVELOPE =					"ENVELOPE";
const char* cBODY =						"BODY";
const char* cBODYALL_OUT =				"(BODY.PEEK[])";
const char* cBODYSTRUCTURE =			"BODYSTRUCTURE";
const char* cBODYSECTION_OUT =			"(BODY[%s])";
const char* cBODYSECTIONPEEK_OUT =		"(BODY.PEEK[%s])";
const char* cBODYSECTION_IN =			"BODY[";
const char* cBODYSECTIONPEEK_IN =		"BODY.PEEK[";
const char* cBODYTEXT =					"BODY[TEXT]";
const char* cBODYPEEKTEXT =				"BODY.PEEK[TEXT]";
const char* cBODYTEXT_OUT =				"(BODY[TEXT])";
const char* cINTERNALDATE =				"INTERNALDATE";
const char* cUID =						"UID";
const char* cRFC822 =					"RFC822";
const char* cRFC822HEADER =				"RFC822.HEADER";
const char* cRFC822HEADER_OUT =			"(RFC822.HEADER)";
const char* cRFC822SIZE =				"RFC822.SIZE";
const char* cRFC822TEXT =				"RFC822.TEXT";
const char* cRFC822TEXTPEEK =			"RFC822.TEXT.PEEK";
const char* cRFC822TEXT_OUT =			"(RFC822.TEXT)";
const char* cBODYHEADER =				"HEADER";
const char* cBODYHEADERFIELDS =			"HEADER.FIELDS";
const char* cBODYHEADERFIELDSNOT =		"HEADER.FIELDS.NOT";
const char* cBODYHEADERMIME =			"MIME";
const char* cBODYHEADERTEXT =			"TEXT";
const char* cSUMMARY =					"(FLAGS RFC822.SIZE INTERNALDATE ENVELOPE BODY)";
const char* cSUMMARY4 =					"(FLAGS RFC822.SIZE UID INTERNALDATE ENVELOPE BODYSTRUCTURE)";
const char* cRFC822MSG =				"(RFC822.HEADER RFC822.TEXT)";
const char* cRFC822_OUT =				"(RFC822)";
const char* cRFC822PEEK_OUT =			"(RFC822.PEEK)";

// Flags
const char* cFLAGRECENT =				"\\Recent";
const char* cFLAGANSWERED =				"\\Answered";
const char* cFLAGFLAGGED =				"\\Flagged";
const char* cFLAGDELETED =				"\\Deleted";
const char* cFLAGSEEN =					"\\Seen";
const char* cFLAGDRAFT =				"\\Draft";
const char* cFLAGMDNSENT =				"$MDNSent";
const char* cFLAGLABELS[] =				{"Cyrusoft.Mulberry.01",
										 "Cyrusoft.Mulberry.02",
										 "Cyrusoft.Mulberry.03",
										 "Cyrusoft.Mulberry.04",
										 "Cyrusoft.Mulberry.05",
										 "Cyrusoft.Mulberry.06",
										 "Cyrusoft.Mulberry.07",
										 "Cyrusoft.Mulberry.08"};
const char* cFLAGKEYWORDS =				"\\*";
const char* cSET_FLAG =					"+FLAGS (";
const char* cUNSET_FLAG =				"-FLAGS (";
const char* cFLAG_END =					")";

const char* cMBOXFLAGMARKED =					"\\Marked";
const char* cMBOXFLAGNOINFERIORS =				"\\Noinferiors";
const char* cMBOXFLAGNOSELECT =					"\\Noselect";
const char* cMBOXFLAGUNMARKED =					"\\Unmarked";
const char* cMBOXFLAGUNMARKEDHASCHILDREN =		"\\HasChildren";
const char* cMBOXFLAGUNMARKEDHASNOCHILDREN =	"\\HasNoChildren";

// STATUS bits
const char* cSTATUS_MESSAGES =			"MESSAGES";
const char* cSTATUS_RECENT =			"RECENT";
const char* cSTATUS_UIDNEXT =			"UIDNEXT";
const char* cSTATUS_UIDVALIDITY =		"UIDVALIDITY";
const char* cSTATUS_UNSEEN =			"UNSEEN";
const char* cSTATUS_CHECK =				"(MESSAGES RECENT UNSEEN UIDVALIDITY UIDNEXT)";

// SEARCH criteria
const char* cSEARCH_CHARSET =			"CHARSET";
const char* cSEARCH_ALL =				"ALL";
const char* cSEARCH_ANSWERED =			"ANSWERED";
const char* cSEARCH_BCC =				"BCC";
const char* cSEARCH_BEFORE =			"BEFORE";
const char* cSEARCH_BODY =				"BODY";
const char* cSEARCH_CC =				"CC";
const char* cSEARCH_DELETED =			"DELETED";
const char* cSEARCH_DRAFT =				"DRAFT";
const char* cSEARCH_FLAGGED =			"FLAGGED";
const char* cSEARCH_FROM =				"FROM";
const char* cSEARCH_HEADER =			"HEADER";
const char* cSEARCH_KEYWORD =			"KEYWORD";
const char* cSEARCH_LARGER =			"LARGER";
const char* cSEARCH_NEW =				"NEW";
const char* cSEARCH_NOT =				"NOT";
const char* cSEARCH_OLD =				"OLD";
const char* cSEARCH_ON =				"ON";
const char* cSEARCH_OR =				"OR";
const char* cSEARCH_RECENT =			"RECENT";
const char* cSEARCH_SEEN =				"SEEN";
const char* cSEARCH_SENTBEFORE =		"SENTBEFORE";
const char* cSEARCH_SENTON =			"SENTON";
const char* cSEARCH_SENTSINCE =			"SENTSINCE";
const char* cSEARCH_SINCE =				"SINCE";
const char* cSEARCH_SMALLER =			"SMALLER";
const char* cSEARCH_SUBJECT =			"SUBJECT";
const char* cSEARCH_TEXT =				"TEXT";
const char* cSEARCH_TO =				"TO";
const char* cSEARCH_UID =				"UID";
const char* cSEARCH_UNANSWERED =		"UNANSWERED";
const char* cSEARCH_UNDELETED =			"UNDELETED";
const char* cSEARCH_UNDRAFT =			"UNDRAFT";
const char* cSEARCH_UNFLAGGED =			"UNFLAGGED";
const char* cSEARCH_UNKEYWORD =			"UNKEYWORD";
const char* cSEARCH_UNSEEN =			"UNSEEN";

// E X T E N S I O N S

// SORT/THREAD

// Commands
const char* cSORT =						"SORT";
const char* cUIDSORT =					"UID SORT";

const char* cTHREAD =					"THREAD";
const char* cUIDTHREAD =				"UID THREAD";

// Keys
const char* cSORT_REVERSE =				"REVERSE";
const char* cSORT_ARRIVAL =				"ARRIVAL";
const char* cSORT_CC =					"CC";
const char* cSORT_DATE =				"DATE";
const char* cSORT_FROM =				"FROM";
const char* cSORT_SIZE =				"SIZE";
const char* cSORT_SUBJECT =				"SUBJECT";
const char* cSORT_TO =					"TO";

const char* cTHREAD_SUBJECT =			"ORDEREDSUBJECT";
const char* cTHREAD_REFERENCES =		"REFERENCES";

// ACL

// Commands
const char* cSETACL =					"SETACL";
const char* cDELETEACL =				"DELETEACL";
const char* cGETACL =					"GETACL";
const char* cLISTRIGHTS =				"LISTRIGHTS";
const char* cMYRIGHTS =					"MYRIGHTS";

// Responses
const char* cACL =						"ACL";


// QUOTA

// Commands
const char* cSETQUOTA =				"SETQUOTA";
const char* cGETQUOTA =				"GETQUOTA";
const char* cGETQUOTAROOT =			"GETQUOTAROOT";

// Responses
const char* cQUOTA =				"QUOTA";
const char* cQUOTAROOT =			"QUOTAROOT";

// NAMESPACE
const char* cNAMESPACE =			"NAMESPACE";

// UIDPLUS

// Response codes
const char* cCOPYUID =				"COPYUID";
const char* cAPPENDUID =			"APPENDUID";

// UNSELECT

// Commands
const char* cUNSELECT =				"UNSELECT";
