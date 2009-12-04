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


// Header for common ACAP definitions

const tcp_port cACAPServerPort =		674;
const tcp_port cACAPServerPort_SSL =	674;

// Options spec
const char cOPTIONS_VENDORAPP[] =		"Cyrusoft/Mulberry";

// Commands
const char cCMD_NOOP[] =				"NOOP";
const char cCMD_LANG[] =				"LANG";
const char cCMD_SEARCH[] =				"SEARCH";
const char cCMD_FREECONTEXT[] =			"FREECONTEXT";
const char cCMD_UPDATECONTEXT[] =		"UPDATECONTEXT";
const char cCMD_STORE[] =				"STORE";
const char cCMD_DELETEDSINCE[] =		"DELETEDSINCE";
const char cCMD_SETACL[] =				"SETACL";
const char cCMD_DELETEACL[] =			"DELETEACL";
const char cCMD_MYRIGHTS[] =			"MYRIGHTS";
const char cCMD_LISTRIGHTS[] =			"LISTRIGHTS";
const char cCMD_GETQUOTA[] =			"GETQUOTA";

// Search modifiers
const char cASEARCH_DEPTH[] =			"DEPTH";
const char cASEARCH_HARDLIMIT[] =		"HARDLIMIT";
const char cASEARCH_LIMIT[] =			"LIMIT";
const char cASEARCH_MAKECONTEXT[] =		"MAKECONTEXT";
const char cASEARCH_ENUMERTAE[] =		"ENUMERTAE";
const char cASEARCH_NOTIFY[] =			"NOTIFY";
const char cASEARCH_NOINHERIT[] =		"NOINHERIT";
const char cASEARCH_RETURN[] =			"RETURN";
const char cASEARCH_SORT[] =			"SORT";

// Search criteria
const char cASEARCH_ALL[] =				"ALL";
const char cASEARCH_AND[] =				"AND";
const char cASEARCH_COMPARE[] =			"COMPARE";
const char cASEARCH_COMPARESTRICT[] =	"COMPARESTRICT";
const char cASEARCH_EQUAL[] =			"EQUAL";
const char cASEARCH_NOT[] =				"NOT";
const char cASEARCH_OR[] =				"OR";
const char cASEARCH_PREFIX[] =			"PREFIX";
const char cASEARCH_RANGE[] =			"RANGE";
const char cASEARCH_SUBSTRING[] =		"SUBSTRING";

// Responses
const char cRESP_ACAP[] =				"ACAP";
const char cRESP_ADDTO[] =				"ADDTO";
const char cRESP_ALERT[] =				"ALERT";
const char cRESP_CHANGE[] =				"CHANGE";
const char cRESP_DELETED[] =			"DELETED";
const char cRESP_ENTRY[] =				"ENTRY";
const char cRESP_LANG[] =				"LANG";
const char cRESP_LISTRIGHTS[] =			"LISTRIGHTS";
const char cRESP_MODTIME[] =			"MODTIME";
const char cRESP_MYRIGHTS[] =			"MYRIGHTS";
const char cRESP_QUOTA[] =				"QUOTA";
const char cRESP_REFER[] =				"REFER";
const char cRESP_REMOVEFROM[] =			"REMOVEFROM";

// Predefined attributes
const char cATTR_ENTRY[] = 				"entry";
const char cATTR_MODTIME[] = 			"modtime";
const char cATTR_SUBDATASET[] = 		"subdataset";

// Attributes metadata
const char cATTRMETA_ACL[] = 			"acl";
const char cATTRMETA_ATTRIBUTE[] = 		"attribute";
const char cATTRMETA_MYRIGHTS[] = 		"myrights";
const char cATTRMETA_SIZE[] = 			"size";
const char cATTRMETA_VALUE[] = 			"value";

// Commonly used strings
const char cUSER[] = 					"/user";
const char cSUBDATASET_ADD[] =			"\"subdataset\" (\"value\" (\".\"))";
const char cDATASET_OPTIONS[] =			"/option/~/vendor.";
const char cASEARCH_ENTRY[] =			"EQUAL \"entry\" \"i;octet\"";
const char cDATASET_ADBK[] =			"/addressbook/";
const char cDATASET_ADBKUSER[] =		"/addressbook/user/";
const char cASEARCH_ADBK[] =			"DEPTH 0 RETURN (\"addressbook.CommonName\") NOT EQUAL \"subdataset\" \"i;octet\" NIL";
const char cASEARCH_ADDRS[] = 			"RETURN (\"addressbook.*\") EQUAL \"subdataset\" \"i;octet\" NIL";
const char cASEARCH_ALIAS[] = 			"RETURN (\"addressbook.*\")";
const char cASEARCH_NOTADBK[] = 		"EQUAL \"subdataset\" \"i;octet\" NIL";
const char cASEARCH_IOCTET[] = 			"\"i;octet\"";
const char cASEARCH_INOCASE[] = 		"\"i;ascii-casemap\"";
const char cASEARCH_INUMERIC[] = 		"\"i;ascii-numeric\"";

// Datasets

// Options dataset
const char cOPTIONSDATASET_ATTR[] =		"option.";

// Addressbook dataset
const char cADBKDATASET_ATTR[] =		"addressbook.";
const char cADBK_NAME[] = 				"CommonName";
const char cADBK_NICKNAME[] = 			"Alias";
const char cADBK_EMAIL[] = 				"Email";
const char cADBK_ADDRESS[] = 			"Postal";
const char cADBK_COMPANY[] = 			"Organization";
const char cADBK_PHONE[] = 				"Telephone";
const char cADBK_PHONEOTHER[] = 		"TelephoneOther";
const char cADBK_URLS[] = 				"HomePage";
const char cADBK_NOTES[] = 				"Comment";

const char cADBK_PHONE_HOME[] = 		"home";
const char cADBK_PHONE_WORK[] = 		"work";
const char cADBK_PHONE_FAX[] = 			"fax";

// Response codes
const char cACAP_RESPONSE_AUTHTOOWEAK[] =		"AUTH-TOO-WEAK";
const char cACAP_RESPONSE_ENCRYPT_NEEDED[] =	"ENCRYPT-NEEDED";
const char cACAP_RESPONSE_INVALID[] =			"INVALID";
const char cACAP_RESPONSE_MODIFIED[] =			"MODIFIED";
const char cACAP_RESPONSE_NOEXIST[] =			"NOEXIST";
const char cACAP_RESPONSE_PERMISSION[] =		"PERMISSION";
const char cACAP_RESPONSE_QUOTA[] =				"QUOTA";
const char cACAP_RESPONSE_REFER[] =				"REFER";
const char cACAP_RESPONSE_IMPLEMENTATION[] =	"IMPLEMENTATION";
const char cACAP_RESPONSE_SASL[] =				"SASL";
const char cACAP_RESPONSE_STARTTLS[] =			"STARTTLS";
const char cACAP_RESPONSE_TOOMANY[] =			"TOOMANY";
const char cACAP_RESPONSE_TOOOLD[] =			"TOOOLD";
const char cACAP_RESPONSE_TRANSITIONNEEDED[] =	"TRANSITION-NEEDED";
const char cACAP_RESPONSE_TRYFREECONTEXT[] =	"TRYFREECONTEXT";
const char cACAP_RESPONSE_TRYLATER[] =			"TRYLATER";
const char cACAP_RESPONSE_WAYTOOMANY[] =		"WAYTOOMANY";
