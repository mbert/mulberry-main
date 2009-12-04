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


// Header for CRFC822 class

#ifndef __CRFC822__MULBERRY__
#define __CRFC822__MULBERRY__

#include "CSMTPCommon.h"
#include "cdstring.h"

//#include "CDSN.h"

#include <ostream>

#include <time.h>

const unsigned long cRFC822Wrap = 76;

#if __line_end == __cr
const char cHDR_END[] = "\r\r";
#elif __line_end == __lf
const char cHDR_END[] = "\n\n";
#elif __line_end == __crlf
const char cHDR_END[] = "\r\n\r\n";
#endif
const char cHDR_RESENT[] = "Resent-";
const char cHDR_DATE[] = "Date: ";
const char cHDR_FROM[] = "From: ";
const char cHDR_SENDER[] = "Sender: ";
const char cHDR_REPLY_TO[] = "Reply-To: ";
const char cHDR_TO[] = "To: ";
const char cHDR_RESENT_TO[] = "Resent-To: ";
const char cHDR_CC[] = "cc: ";
const char cHDR_RESENT_CC[] = "Resent-cc: ";
const char cHDR_BCC[] = "bcc: ";
const char cHDR_RESENT_BCC[] = "Resent-bcc: ";
const char cHDR_SUBJECT[] = "Subject: ";
const char cHDR_IN_REPLY_TO[] = "In-Reply-To: ";
const char cHDR_MESSAGE_ID[] = "Message-ID: ";
const char cHDR_REFERENCES[] = "References: ";
const char cHDR_ORIGINATOR_INFO[] = "Originator-Info: ";
	const char cHDR_ORIGINATOR_LOGIN_ID[] = "login-id=";
	const char cHDR_ORIGINATOR_LOGIN_TOKEN[] = "login-token=";
	const char cHDR_ORIGINATOR_SERVER_ID[] = "server=";
	const char cHDR_ORIGINATOR_TOKEN_ATHORITY[] = "token_authority=";
const char cHDR_AUTO_SUBMITTED[] = "Auto-Submitted: ";
	const char cHDR_AUTO_GENERATED_FAILURE[] = "auto-generated (failure)";
	const char cHDR_XFROM_MAILER_DAEMON[] = "MAILER-DAEMON";
const char cHDR_MDN[] = "Disposition-Notification-To: ";

const char cHDR_MIME_VERSION[] = "MIME-Version: 1.0";
const char cHDR_MIME_TYPE[] = "Content-Type: ";
const char cHDR_MIME_ENCODING[] = "Content-Transfer-Encoding: ";
const char cHDR_MIME_ID[] = "Content-ID: ";
const char cHDR_MIME_DESCRIPTION[] = "Content-Description: ";
const char cHDR_MIME_DISPOSITION[] = "Content-Disposition: ";

const char cHDR_XMULBERRY_IDENTITY[] = "X-Mulberry-Identity: ";
const char cHDR_XMULBERRY_DSN[] = "X-Mulberry-DSN: ";
const char cHDR_XMULBERRY_RESENT_FROM[] = "X-Mulberry-Resent-From: ";
const char cHDR_XMULBERRY_RESENT_TO[] = "X-Mulberry-Resent-To: ";
const char cHDR_XMULBERRY_RESENT_CC[] = "X-Mulberry-Resent-cc: ";
const char cHDR_XMULBERRY_RESENT_BCC[] = "X-Mulberry-Resent-Bcc: ";
const char cHDR_XMULBERRY_MAIL_FROM[] = "X-Mulberry-Mail-From: ";
const char cHDR_XMULBERRY_RCPT_TO[] = "X-Mulberry-Rcpt-To: ";
const char cHDR_XMULBERRY_RCPT_CC[] = "X-Mulberry-Rcpt-cc: ";
const char cHDR_XMULBERRY_RCPT_BCC[] = "X-Mulberry-Rcpt-Bcc: ";

const char cHDR_FILES[] = "Files: ";
const char cHDR_COPYTO[] = "CopyTo: ";

const char cGMT_POS[] = "+";
const char cGMT_NEG[] = "-";

const char cRFC822_MAILBOX_SPECIALS[] = " ()<>@,;:\\\"[]";		// Same as specials but without '.'
//const char cRFC822_QUOTE[] = " ()<>@,;:\\\".[]";
const char cRFC822_QUOTE[] = "()<>@,;:\\\".[]/?=";
const char cRFC822_PARAMQUOTE[] = " ()<>@,;:\\\".[]/?=";
const char cRFC822_ESCAPE[] = "\\\"";

const char cRFC1522_QUOTE_START_1[] = "=?ISO-8859-1?Q?";
const char cRFC1522_QUOTE_START_15[] = "=?ISO-8859-15?Q?";
const char cRFC1522_QUOTE_START_UTF8[] = "=?UTF-8?Q?";
const char cRFC1522_QUOTE_END[] = "?=";

// Classes
class CMessage;
class CAddress;
class CAddressList;
class CIdentity;
class CDSN;

class CRFC822 {

private:
	static short			sWrapLength;					// Length to wrap to

					CRFC822() {};
					~CRFC822() {};

public:

	enum ECreateHeaderFlags
	{
		eNoFlags		=	0L,
		eAddBcc			=	1L << 0,
		eAddXIdentity	=	1L << 1,
		eAddXMulberry	=	1L << 2,
		eNonBccSend		=	1L << 3,
		eBccSend		=	1L << 4,
		eRejectDSN		=	1L << 5,
		eMDN			=	1L << 6
	};

	static void		CreateHeader(CMessage* theMsg,
									ECreateHeaderFlags flags = eNoFlags,
									const CIdentity* identity = NULL,
									const CDSN* dsn = NULL,
									const char* bounced = NULL,
									bool original = false);		// Create RFC822 mail header from envelope
	static void		SendHeader(CMessage* theMsg,				// Strip unwanted headers
								CDSN& dsn,
								bool allow_content);

	static bool		HeaderSearch(const char* hdr,				// Search and retreive header text
									const cdstring& field,
									cdstring& result);

	static const char* 	RemoveMIMEHeaders(const char *header,
											bool no_subject = false);
	static const char* 	OnlyHeaders(const char* header, const cdstrvect& hdrs);

	static void		AddAddressList(const char* field,
									bool bounced,
									const CAddressList* theList,
									std::ostream& theStream,
									bool encode = true);		// Add address list to header
	static CAddressList* ParseAddressList(const char* line);

	static void		SetWrapLength(short wrap);					// Set number of chars to wrap to
	static short	GetWrapLength();							// Get number of chars to wrap to

	static void		FoldLines(cdstring& text,					// Fold lines
								bool addr_phrase,
								unsigned long offset = 0,
								char breakat = 0);
	static void		UnfoldLines(char* text);					// Unfold lines
	static void		HeaderQuote(cdstring& str,					// Encode header item into RFC1522 or quote
								bool addr_phrase,
								bool encode,
								bool param = false);
	static bool		TextTo1522(cdstring& str,					// Encode string into RFC1522
									bool addr_phrase,
									bool wrap = false,
									unsigned long offset = 0);
	static bool		TextFrom1522(cdstring& str);				// Decode string from RFC1522
	static bool		Needs2231(const cdstring& str);				// See whether RFC2231 encoding is needed
	static bool		TextTo2231(cdstring& str);					// Encode string into RFC2231
	static bool		TextFrom2231(cdstring& str);				// Decode string from RFC2231

	// Date handling
	static cdstring	GetRFC822Date(time_t use_date = 0, long use_zone = 0);			// Create RFC822 style date & time
	static cdstring	GetUNIXDate(time_t use_date = 0, long use_zone = 0);			// Create UNIX mailbox style date & time
	static cdstring	GetIMAPDate(time_t use_date = 0, long use_zone = 0);			// Create IMAP date_time syntax
	static cdstring	GetDate(const char* format, time_t use_date = 0, long use_zone = 0,
								bool dayname = true, bool monthname = true);// Create formatted date & time
	static void	SetDate(char* theDate, time_t& date, long& zone);					// Set numeric date from text
	static void	SetUNIXDate(char* theDate, time_t& date, long& zone);				// Set numeric date from UNIX format text
	static cdstring GetTextDate(time_t date, long zone, bool for_display, bool long_date = false);	// Get date as text
	static unsigned long ParseDate(char* txt);										// Parse search item date
	
	static unsigned long GetDayOfWeek(char* day);									// Get day of week (Sun = 1)
	static unsigned long GetMonth(char* month);										// Get month (Jan = 1)
	
	static long GetZone(char* zone);												// Get zone
	static long GetLocalZone();														// Get local timezone offset in secs
};

#endif
