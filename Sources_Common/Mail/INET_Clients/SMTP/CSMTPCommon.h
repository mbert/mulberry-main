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


// Header for common SMTP definitions

#ifndef __CSMTPCOMMON__MULBERRY__
#define __CSMTPCOMMON__MULBERRY__

#define kSMTPReceiverPort		25
#define kSMTPReceiverPort_SSL	465
#define MAX_CC				25
#define MAX_LINE_LEN		80

// Strings
#define NULL_STR			""
#define SPACE				" "
#define COMMA_SPACE			", "
#define CONTINUATION		'-'
#define EHLO				"EHLO "
#define HELO				"HELO "
#define STARTTLS			"STARTTLS"
#define AUTHPLAIN			"AUTH PLAIN "
#define AUTHLOGIN			"AUTH LOGIN"
#define AUTHEXTERNAL		"AUTH EXTERNAL "
#define RSET				"RSET"
#define MAILFROM			"MAIL FROM:<"
#define RCPTTO				"RCPT TO:<"
#define DATA				"DATA"
#define QUIT				"QUIT"
#define RANGLE_CRLF			">\r\n"
#define CR					"\r"
#define CRLF				"\r\n"
#define DOT_CRLF 			".\r\n"
#define CRLF_DOT_CRLF 		"\r\n.\r\n"

// Extensions
#define RET					"RET="
#define RET_HDRS			"HDRS"
#define RET_FULL			"FULL"
#define NOTIFY				"NOTIFY="
#define NOTIFY_NEVER		"NEVER"
#define NOTIFY_SUCCESS		"SUCCESS"
#define NOTIFY_FAILURE		"FAILURE"
#define NOTIFY_DELAY		"DELAY"
#define ESMTP_SIZE			"SIZE"

// Response code chars
#define OK_RESPONSE			'2'
#define FAIL_RESPONSE		'5'
#define DATA_RESPONSE		'3'

#endif
