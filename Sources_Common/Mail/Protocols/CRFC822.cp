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


// Source for CRFC822 class

#include "CRFC822.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCharSpecials.h"
#include "CCharsetManager.h"
#include "CDSN.h"
#include "CEnvelope.h"
#include "CGeneralException.h"
#include "CINETCommon.h"
#include "CMessage.h"
#include "CMIMEFilters.h"
#include "CMIMESupport.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CRegistration.h"
#include "CSMTPSender.h"
#include "CStringUtils.h"
#include "CURL.h"

#include "cdustring.h"

#if __dest_os == __win32_os
#include <WIN_LMemFileStream.h>
#elif __dest_os == __linux_os
#include "UNX_LMemFileStream.h"
#endif

#include <stdio.h>
#include <string.h>
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"
#include <UEnvironment.h>
#ifdef __MSL__
#include <time.mac.h>
#else
#define _mac_msl_epoch_offset_ (-((365L * 66L) + 17) * 24L * 60L * 60L)		/*- mm 000127 -*/ /*- mm 001023 -*/
#endif
//#include <winsock.h>
#endif

#include <ctype.h>
#include <strstream>

// Local defs

const char cHDR_X[] = "X-";
const char cHDR_MAILER[] = "Mailer: ";

// Word break delimiters
const char *host_delim = " ()<>@,;:\\\"";			// Host delimiters
const char *word_delim = " ()<>@,;:\\\"[]";			// Word delimiters
const char *full_delim = "()<>@,;:\\\"[].";			// Full delimiters
const char *body_delim = " ()<>@,;:\\\"[]./?=";		// Body delimiters

// __________________________________________________________________________________________________
// C L A S S __ C R F C 8 2 2
// __________________________________________________________________________________________________

short CRFC822::sWrapLength = cRFC822Wrap;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________Header Processing

// Remove MIME Headers from a bounced message
const char* CRFC822::RemoveMIMEHeaders(const char* header, bool no_subject)
{
	// Must have header
	if (header == NULL)
		return NULL;

	bool beginLine = true;
	bool record = true;
	std::ostrstream out;

	const char* p = header;
	while(*p)
	{
		switch(*p)
		{
		case '\r':
		case '\n':
			// Check for double-CRLF - should only occur at very end of header
			if (beginLine)
			{
				// Punt over double-CFLFs
				while((*p == '\r') || (*p == '\n'))
					p++;
				continue;
			}

			// Have start of line
			beginLine = true;
			if (record)
				out.put(*p);

			// Punt over LF in CRLF
			if ((*p == '\r') && (*(p+1) == '\n'))
			{
				p++;
				if (record)
					out.put(*p);
			}
			p++;

			// Might be folded, in which case don't change record status
			if ((*p != ' ') && (*p != '\t'))
				record = true;
			break;
		default:
			// Check for MIME header
			if (beginLine &&
			    ((::strncmpnocase(p, "MIME-Version:", 13) == 0) ||
			     (::strncmpnocase(p, "Content", 7) == 0) ||
			     no_subject && (::strncmpnocase(p, "Subject", 7) == 0)))
			{
				record = false;
			}
			else if (record)
			{
				out.put(*p);
			}
			beginLine = false;
			p++;
		}
	}
	
	out << std::ends;
	return out.str();
}

// Remove all but the chosen headers
const char* CRFC822::OnlyHeaders(const char* header, const cdstrvect& hdrs)
{
	bool beginLine = true;
	bool record = true;
	std::ostrstream out;

	const char* p = header;
	while(*p)
	{
		switch(*p)
		{
		case '\r':
		case '\n':
			// Check for double-CRLF - should only occur at very end of header
			if (beginLine)
			{
				// Punt over double-CFLFs
				while((*p == '\r') || (*p == '\n'))
					p++;
				continue;
			}

			// Have start of line
			beginLine = true;
			if (record)
				out.put(*p);

			// Punt over LF in CRLF
			if ((*p == '\r') && (*(p+1) == '\n'))
			{
				p++;
				if (record)
					out.put(*p);
			}
			p++;

			// Might be folded, in which case don't change record status
			if ((*p != ' ') && (*p != '\t'))
				record = true;
			break;
		default:
			// Check for specific header
			bool matched = isspace(*p);
			if (beginLine && !matched)
			{
				for(cdstrvect::const_iterator iter = hdrs.begin(); iter != hdrs.end(); iter++)
				{
					if (!::strncmpnocase(p, *iter, (*iter).length()))
					{
						matched = true;
						break;
					}
				}
			}

			// Check for non-matched header
			if (beginLine && !matched)
			{
				record = false;
			}
			else if (record)
			{
				out.put(*p);
			}
			beginLine = false;
			p++;
		}
	}
	
	out << std::ends;
	return out.str();
}

// Create RFC822 mail header from parms
void CRFC822::CreateHeader(CMessage* theMsg,
							ECreateHeaderFlags flags,
							const CIdentity* identity, const CDSN* dsn, const char* bounced, bool original)
{
	// Need default identity
	const CIdentity* default_id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Create stream to handle writing
	std::ostrstream out;

	// Add bounce header if there
	if (bounced)
	{
		// Replace existing MIME headers and subject
		const char* temp_bounce = RemoveMIMEHeaders(bounced, true);

		// Add to start of this message's header
		out << temp_bounce;
		delete temp_bounce;
	}

	// Get Date details
	if (bounced)
		out << cHDR_RESENT;
	out << cHDR_DATE;
	cdstring date = GetRFC822Date();
	out << date << os_endl;

	CEnvelope* theEnv = theMsg->GetEnvelope();
	if (!theEnv)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	// Get From details
	AddAddressList(cHDR_FROM, bounced, theEnv->GetFrom(), out);
	if (bounced && (flags & eAddXMulberry))
		AddAddressList(cHDR_XMULBERRY_MAIL_FROM, false, theEnv->GetFrom(), out);

	// Use original sender if regenerating header
	if (original)
	{
		// Get Sender details
		AddAddressList(cHDR_SENDER, bounced, theEnv->GetSender(), out);
	}

	// Sender only generated under particular circumstances
	else
	{
		// Check whether sender same as first From
		CAddress* from = theEnv->GetFrom()->size() ? theEnv->GetFrom()->front() : NULL;
		CAddress* sender = NULL;
		if (CAdminLock::sAdminLock.mGenerateSender)
		{
			// Generate sender from valid login id
			cdstring sender_txt = CAdminLock::sAdminLock.GetLoginID();
			sender_txt += "@";
			sender_txt += CAdminLock::sAdminLock.GetServerID();
			sender = new CAddress(sender_txt);
		}
		else if (!CAdminLock::sAdminLock.mLockIdentitySender && identity && identity->UseSender())
		{
			// Must not be empty
			if (!identity->GetSender().empty())
				sender = new CAddress(identity->GetSender());
		}
		else if (!CAdminLock::sAdminLock.mLockIdentitySender && default_id->UseSender())
		{
			// Must not be empty
			if (!default_id->GetSender().empty())
				sender = new CAddress(default_id->GetSender());
		}

		// Generate Sender if one is present and there is no
		// From or From is not the same as Sender
		if (sender && (!from || !(*from == *sender)))
		{
			cdstring sender_txt = sender->GetFullAddress(true);

			// Get Sender details
			if (bounced)
				out << cHDR_RESENT;
			out << cHDR_SENDER;
			//TextTo1522(sender_txt);
			out << sender_txt << os_endl;
		}
		delete sender;
	}

	// Get Reply-to details
	AddAddressList(cHDR_REPLY_TO, bounced, theEnv->GetReplyTo(), out);

	// Set To, CC, Bcc details
	// Get To details
	AddAddressList(cHDR_TO, bounced, theEnv->GetTo(), out);
	AddAddressList(cHDR_CC, bounced, theEnv->GetCC(), out);
	if (flags & eAddBcc)
		AddAddressList(cHDR_BCC, bounced, theMsg->GetEnvelope()->GetBcc(), out);

	// Set SMTP envelope details
	if ((bounced || (flags & (eNonBccSend | eBccSend))) && (flags & eAddXMulberry))
	{
		AddAddressList(cHDR_XMULBERRY_RCPT_TO, false, (flags & eBccSend) ? NULL : theEnv->GetTo(), out);
		AddAddressList(cHDR_XMULBERRY_RCPT_CC, false, (flags & eBccSend) ? NULL : theEnv->GetCC(), out);
		AddAddressList(cHDR_XMULBERRY_RCPT_BCC, false, (flags & eNonBccSend) ? NULL : theEnv->GetBcc(), out);
	}
	
	// Must ensure that there is at least one destination address if this is not a draft
	if (!(flags & eAddXIdentity) && !bounced && !theEnv->GetTo()->size() && !theEnv->GetCC()->size())
	{
		if (bounced)
			out << cHDR_RESENT;
		out << cHDR_TO << "unspecified-recipients:;" << os_endl;
	}

	// Get subject details
	cdstring subject = theEnv->GetSubject();
	FoldLines(subject, false, sizeof(cHDR_SUBJECT) - 1);
	out << cHDR_SUBJECT << subject << os_endl;

	// Get Message-ID details
	if (!theEnv->GetMessageID().empty())
	{
		if (bounced)
			out << cHDR_RESENT;
		out << cHDR_MESSAGE_ID;
		out << theEnv->GetMessageID() << os_endl;
	}

	// Only relevant if not bouncing
	if (!bounced)
	{
		// Get In-Reply-to details
		if (!theEnv->GetInReplyTo().empty())
			out << cHDR_IN_REPLY_TO << theEnv->GetInReplyTo() << os_endl;

		// Get References details
		if (theEnv->HasReferences() && !theEnv->GetReferences().empty())
		{
			cdstring refs = theEnv->GetReferences();
			FoldLines(refs, false, sizeof(cHDR_REFERENCES) - 1, '>');
			out << cHDR_REFERENCES << refs << os_endl;
		}
	}

	// Write Originator-Info
	if (CAdminLock::sAdminLock.mOriginator.mGenerate && !original)
	{
		if (bounced)
			out << cHDR_RESENT;
		out << cHDR_ORIGINATOR_INFO;
		if (CAdminLock::sAdminLock.mOriginator.mUseToken)
			out << cHDR_ORIGINATOR_LOGIN_TOKEN << CAdminLock::sAdminLock.GetLoginToken();
		else
		{
			out << cHDR_ORIGINATOR_LOGIN_ID << CAdminLock::sAdminLock.GetLoginID();
			out << "; ";
			out << cHDR_ORIGINATOR_SERVER_ID << CAdminLock::sAdminLock.GetServerID();
		}
		if (CAdminLock::sAdminLock.mOriginator.mUseTokenAuthority)
		{
			cdstring token_auth = CAdminLock::sAdminLock.mOriginator.mTokenAuthority;
			token_auth.quote();
			out << ";" << os_endl << " " << cHDR_ORIGINATOR_TOKEN_ATHORITY << token_auth;
		}
		out << os_endl;
	}

	// Only do MDN for outgoing queue and when at least one From address exists and not an MDN itself
	if (dsn && !(flags & eMDN) && (flags & eAddXMulberry) && dsn->GetMDN() && theEnv->GetFrom() && theEnv->GetFrom()->size())
		out << cHDR_MDN << theEnv->GetFrom()->front()->GetFullAddress(true) << os_endl;

	// Write mailer details
	if (!original)
	{
		// Output header name
		out << cHDR_X;
		if (bounced)
			out << cHDR_RESENT;
		out << cHDR_MAILER;

		// Add details
		out << CPreferences::sPrefs->GetMailerDetails(true) << os_endl;
	}

	// Write identity specific headers
	if (identity && ((flags & eAddXIdentity) || (flags & eAddXMulberry)))
	{
		cdstring id_name = identity->GetIdentity();
		TextTo1522(id_name, false);
		out << cHDR_XMULBERRY_IDENTITY << id_name << os_endl;
	}
	
	// Only do DSN for outgoing queueand not reject
	if (dsn && !(flags & eRejectDSN) && !(flags & eMDN) && (flags & eAddXMulberry) && dsn->GetRequest())
	{
		out << cHDR_XMULBERRY_DSN;
		dsn->WriteHeaderToStream(out);
		out << os_endl;
	}

	// Add Auto-Generated for DSN
	if (flags & eRejectDSN)
	{
		out << cHDR_AUTO_SUBMITTED << cHDR_AUTO_GENERATED_FAILURE << os_endl;

		// Add fake header to force a special MAIL-FROM in SMTP exchange
		if (flags & eAddXMulberry)
			out << cHDR_XMULBERRY_MAIL_FROM << os_endl;
	}

	// Add any user defined header
	if (CAdminLock::sAdminLock.mAllowXHeaders && !original && identity)
	{
		// Insert header if not empty
		cdstring header = identity->GetHeader(true);
		if (!header.empty())
		{
			// Must not have consequetive empty lines
			char* p = header.c_str_mod();
			char* q = p;
			bool got_endl = true;
			while(*p)
			{
				if ((*p == os_endl[0]) && got_endl)
				{
					// Skip extra endls
					p++;
					if (os_endl[1] && (*p == os_endl[1]))
						p++;
				}
				else if ((*p == os_endl[0]) || os_endl[1] && (*p == os_endl[1]))
				{
					got_endl = true;
					*q++ = *p++;
				}
				else
				{
					got_endl = false;
					*q++ = *p++;
				}
			}
			*q = 0;

			TextTo1522(header, true);
			out << header;

			// Add endl if last char was not a endl
			if (!got_endl)
				out << os_endl;
		}
	}

	// These should not happen if bouncing
	if (/*!bounced && */theMsg->GetBody())
	{

		// Add MIME version
		out << cHDR_MIME_VERSION << os_endl;
	}

	// Give buffer to message
	out << std::ends;
	theMsg->SetHeader(out.str());
}

// Process header for sending
void CRFC822::SendHeader(CMessage* theMsg, CDSN& dsn, bool allow_content)
{
	// RESET DSN
	dsn = CDSN();

	// SMTP envelope override address lists
	CAddressList* mail_from = NULL;
	CAddressList* rcpt_to = NULL;
	CAddressList* rcpt_cc = NULL;
	CAddressList* rcpt_bcc = NULL;

	// Create stream to handle writing
	std::ostrstream out;

	const char* p = theMsg->GetHeader();

	// Look for unwanted headers
	bool got_line_end = true;
	bool do_line = true;
	while(*p)
	{

		// Look for unwanted headers (MIME, Mulberry and bcc)
		if (!allow_content && !::strncmpnocase(p, "Content", 7))
			do_line = false;
		else if (!::strncmpnocase(p, "X-Mulberry", 10))
		{
			// Look for specific headers
			if (!::strncmpnocase(p, cHDR_XMULBERRY_DSN, sizeof(cHDR_XMULBERRY_DSN) - 1))
			{
				// Punt to end of line
				const char* line = p + sizeof(cHDR_XMULBERRY_DSN) - 1;
				const char* end = line;
				while(*end && (*end != lendl1)) end++;
				cdstring line_txt(line, end - line);
				dsn.ReadHeader(line_txt.c_str_mod());

				// Mark is as requested
				dsn.SetRequest(true);
			}
			
			// Look for SMTP envelope addressing headers
			// NB We also look for the old Resent headers just in case a user
			// upgrades to the new version while they have disconencted messages
			// to send
			const char* tests[] = 
			{
				cHDR_XMULBERRY_MAIL_FROM,
				cHDR_XMULBERRY_RCPT_TO,
				cHDR_XMULBERRY_RCPT_CC,
				cHDR_XMULBERRY_RCPT_BCC,
				cHDR_XMULBERRY_RESENT_FROM,
				cHDR_XMULBERRY_RESENT_TO,
				cHDR_XMULBERRY_RESENT_CC,
				cHDR_XMULBERRY_RESENT_BCC,
				NULL
			};
			
			unsigned long test_lengths[] = 
			{
				sizeof(cHDR_XMULBERRY_MAIL_FROM) - 1,
				sizeof(cHDR_XMULBERRY_RCPT_TO) - 1,
				sizeof(cHDR_XMULBERRY_RCPT_CC) - 1,
				sizeof(cHDR_XMULBERRY_RCPT_BCC) - 1,
				sizeof(cHDR_XMULBERRY_RESENT_FROM) - 1,
				sizeof(cHDR_XMULBERRY_RESENT_TO) - 1,
				sizeof(cHDR_XMULBERRY_RESENT_CC) - 1,
				sizeof(cHDR_XMULBERRY_RESENT_BCC) - 1,
				0
			};
			
			CAddressList** lists[] =
			{
				&mail_from,
				&rcpt_to,
				&rcpt_cc,
				&rcpt_bcc,
				&mail_from,
				&rcpt_to,
				&rcpt_cc,
				&rcpt_bcc,
				NULL
			};
			
			unsigned long ctr = 0;
			while(tests[ctr])
			{
				if (!::strncmpnocase(p, tests[ctr], test_lengths[ctr]))
				{
					// Get an entire folded line
					cdstring lineout;
					const char* start = p + test_lengths[ctr];
					const char* stop = start;
					while(stop)
					{
						// Look for end of line
						if (*stop == lendl1)
						{
							// Output the current line
							lineout += cdstring(start, stop - start);

							// Step over line end
							stop++;
#if __line_end == __crlf
							if (*stop == lendl2)
								stop++;
#endif

							// Look for fold and terminate loop if not
							if (!isspace(*stop))
								stop = NULL;
							else
								// Restart line append from here
								start = stop;
						}
						else
							stop++;
					}

					// Parse the address list
					*lists[ctr] = ParseAddressList(lineout);
					
					// End loop
					break;
				}
				ctr++;
			}

			do_line = false;
		}
		else if (!::strncmpnocase(p, "bcc", 3))
			do_line = false;

		// Output current line
		while(*p && (*p != lendl1))
		{
			if (do_line)
				out << *p;
			p++;
			got_line_end = false;
		}

		// Punt over line end
		if (*p == lendl1)
		{
			if (do_line)
				out << *p;
			p++;

#if __line_end == __crlf
			if (*p == lendl2)
			{
				if (do_line)
					out << *p;
				p++;
			}
#endif
		}

		// Don't output CRLF pair
		if (*p == lendl1)
		{
			// Punt over line end
			if (*p == lendl1)
			{
				p++;

#if __line_end == __crlf
				if (*p == lendl2)
					p++;
#endif
			}
		}
		// Look for folded line
		else if ((*p != ' ') && (*p != '\t'))
			// Reset line output if not folded
			do_line = true;
			// Otherwise use existing line output
	}

	// Give buffer to message
	out << std::ends;
	theMsg->SetHeader(out.str());
	
	// Replace message envelope address lists with SMTP envelope addresses if present
	if (mail_from)
		theMsg->GetEnvelope()->SetFrom(mail_from);
	if (rcpt_to || rcpt_cc || rcpt_bcc)
	{
		theMsg->GetEnvelope()->SetTo(rcpt_to);
		theMsg->GetEnvelope()->SetCC(rcpt_cc);
		theMsg->GetEnvelope()->SetBcc(rcpt_bcc);
	}

	// Check for 'unspecified-recipients'
	if ((theMsg->GetEnvelope()->GetTo()->size() == 1) &&
		!theMsg->GetEnvelope()->GetCC()->size())
	{
		// Remove all to's if no others
		CAddress* addr = theMsg->GetEnvelope()->GetTo()->front();
		if ((addr->GetMailbox() == "unspecified-recipients") &&
			addr->GetName().empty() && addr->GetHost().empty())
			theMsg->GetEnvelope()->GetTo()->clear();
	}
}

// Search and retreive header text
bool CRFC822::HeaderSearch(const char* hdr, const cdstring& field, cdstring& result)
{
	const char* p = hdr;

	// Make sure field ends with ": "
	cdstring temp_field = field;
	if (!temp_field.compare_end(": "))
		temp_field += ": ";
	unsigned long field_length = temp_field.length();

	// Look for unwanted headers
	bool got_line_end = true;
	bool do_line = true;
	while(*p)
	{
		// Look for the required header at start of line
		if (!::strncmpnocase(p, temp_field, field_length))
		{
			const char* line_start = p + field_length;
			const char* line_end = line_start;

			while(true)
			{
				// Compare entire folder line
				while(*line_end && (*line_end != lendl1))
					line_end++;
				
				const char* snoop = line_end;
				if (*snoop == lendl1)
				{
					snoop++;
#if __line_end == __crlf
					if (*snoop == lendl2)
						snoop++;
#endif
				}
				
				// Look for fold
				if ((*snoop != ' ') && (*snoop != '\t'))
				{
					// Next line is a header so copy up to (but not including) end of line
					result.assign(line_start, line_end - line_start);
					
					// Now unfold it
					UnfoldLines(result.c_str_mod());
					
					// Decode it
					TextFrom1522(result);
					return true;
				}
				
				// Must be folded - adjust end and carry on
				line_end = snoop;
			}
		}

		// Punt to end of line
		while(*p && (*p != lendl1))
			p++;

		// Punt over line end
		if (*p == lendl1)
		{
			p++;

#if __line_end == __crlf
			if (*p == lendl2)
				p++;
#endif
		}
	}

	return false;
}

// Add address list to header
void CRFC822::AddAddressList(const char* field,
								bool bounced,
								const CAddressList* theList,
								std::ostream& out,
								bool encode)
{
	short line_length = ::strlen(field);

	// Address list may be NULL or empty
	if (!theList || (theList->size() == 0))
		return;

	// Write out field header
	if (bounced)
		out << cHDR_RESENT;
	out << field;

	// Add list items
	bool add_comma = false;
	for(CAddressList::iterator iter = ((CAddressList*) theList)->begin(); iter != ((CAddressList*) theList)->end(); iter++)
	{
		// Form first address (possible encode for RFC1522)
		cdstring addr = (*iter)->GetFullAddress(encode);

		// Check that cat'ed str is less than max line len
		line_length += addr.length();
		if (line_length + 3 > MAX_LINE_LEN)
		{

			// More to come so add comma
			if (add_comma)
				out << ',';

			// Terminate line
			out << os_endl;

			// Start new line indented with a space
			out << SPACE;
			add_comma = false;
			line_length = 1;
		}

		// Add comma if required
		if (add_comma)
			out << COMMA_SPACE;

		// Add next address to line and make sure comma gets used if more to come
		out << addr;
		add_comma = true;
	}

	// Terminate line
	out << os_endl;

}

CAddressList* CRFC822::ParseAddressList(const char* line)
{
	// Punt to end of line
	const char* end = line;
	while(true)
	{
		// Punt to end of line
		while(*end && (*end != lendl1)) end++;
#if __line_end == __crlf
		if (*end == lendl2)
			end++;
#endif
		// Sniff for LWSP
		if ((*end != ' ') && (*end != '\t'))
			break;

	}
	cdstring line_txt(line, end - line);
	return new CAddressList(line_txt, line_txt.length());
}

#pragma mark ____________________________Message Processing

// Set number of chars to wrap to
void CRFC822::SetWrapLength(short wrap)
{
	sWrapLength = (wrap == 0) ? 1000 : wrap;
}

// Get number of chars to wrap to
short CRFC822::GetWrapLength()
{
	return sWrapLength;
}

// Fold lines
void CRFC822::FoldLines(cdstring& text, bool addr_phrase, unsigned long offset, char breakat)
{
	// Try encoding with wrap
	if (TextTo1522(text, addr_phrase, true, offset))
		return;

	// Check for exceed of wrap length
	if (text.length() + offset < GetWrapLength())
		return;

	// Create stream to handle writing
	std::ostrstream out;
	const char* p = text.c_str();

	// Loop over all text and wrap
	long remaining = text.length();
	while(remaining > 0)
	{
		long lastSpace = -1;
		long lastBreak = -1;
		long count = 0;
		const char* endLine = p;

		// Loop while waiting for line break or exceed of wrap length
		// NB When breakat is set, if there is no break character below the wrap length,
		// keep going until one is found.
		while ((*endLine != '\r') &&
				(*endLine != '\n') &&
				((count + offset <= GetWrapLength()) || ((breakat != 0) && (lastBreak == -1) && (lastSpace == -1))) &&
				(remaining - count > 0))
		{
			if (*endLine == ' ')
				lastSpace = count;
			if (breakat && (*endLine == breakat))
				lastBreak = count;
			endLine++;
			count++;
		}

		// Check break state
		if (count + offset > GetWrapLength())
		{
			// Exceed wrap - check for spaces
			if (lastSpace > 0)
			{
				// Adjust count
				count = lastSpace;

				// Copy line to buffer without trailing SP
				out.write(p, count);
				
				// We want to ignore the trailing space
				count++;
			}

			// Exceed wrap - check for break character
			else if (lastBreak > 0)
			{
				// Adjust count to include the break character
				count = lastBreak + 1;

				// Copy line to buffer without trailing SP
				out.write(p, count);
			}

			// Exceed wrap - forced break
			else
				// Copy whole line to buffer
				out.write(p, count);

			// Add FWS if more to come
			if (remaining - count > 0)
				out << os_endl << " ";
		}
		
		else
		{
			// Check whether at end of text
			if (remaining - count > 0)
			{
				// Copy line to buffer with trailing CR/LF
				out.write(p, count);
				out << os_endl << " ";
				count++;
			}
			else
				// Copy last line to buffer
				out.write(p, count);
		}

		// Update counter & ptr (include CR/LF)
		remaining -= count;
		p += count;
#if __line_end == __crlf
		// Bump past '\n'
		if (*p == lendl2)
		{
			remaining--;
			p++;
		}
#endif
		// Reset offset after first time through
		offset = 0;
	}

	// Now create the buffer for the header
	out << std::ends;
	text.steal(out.str());
}

// Unfold lines
void CRFC822::UnfoldLines(char* text)
{
	char* p = text;
	char* q = text;

	while(*p)
	{
		// Snoop for possible FWS
		if ((*p == '\r') || (*p == '\n'))
		{
			// Bump over CR & CRLFs
			char* r = p;
			while((*r == '\r') || (*r == '\n'))
				r++;

			// Check for FWS-char
			if (isspace(*r))
			{
				// Bump over CR or CRLF
				p = r;
			}
			// Copy all CR & CRLFs
			else
				while((*p == '\r') || (*p == '\n'))
					*q++ = *p++;
		}
		// Copy in -> out
		else
			*q++ = *p++;
	}

	*q = '\0';
}

#pragma mark ____________________________MIME Processing

// Encode string into RFC1522
void CRFC822::HeaderQuote(cdstring& str, bool addr_phrase, bool encode, bool param)
{
	if (str.empty())
		return;

	// Try to encode to 1522 if requested and required
	if (encode)
		encode = TextTo1522(str, addr_phrase);

	// If not encoding try to quote if required
	if (!encode)
	{
		// Check for specials
		bool do_quote = ::strpbrk(str.c_str(), param ? cRFC822_PARAMQUOTE : cRFC822_QUOTE);

		std::ostrstream out;
		if (do_quote)
			out << '\"';

		// May need further quoting
		const char* p = str;
		do
		{
			switch(*p)
			{
			// Quote these
			case '\\':
			case '\"':
				out << '\\';
			// Fall through
			default:
				// Make characters safe
				if (!param || cUSASCIIChar[*reinterpret_cast<const unsigned char*>(p)])
					out << *p;
				else
					out << '_';
			}
		} while (*++p);
		if (do_quote)
			out << '\"';

		// Copy stream back to string
		out << std::ends;
		str.steal(out.str());
	}
}

// Encode utf8 string into RFC1522 with utf8 encoding
bool CRFC822::TextTo1522(cdstring& str, bool addr_phrase, bool wrap, unsigned long offset)
{
	// Check for any to be encoded
	i18n::ECharsetCode charset = i18n::CCharsetManager::sCharsetManager.CharsetForText(str, CPreferences::sPrefs->mAlwaysUnicode.GetValue());

	// Do encoding if required
	if (charset != i18n::eUSASCII)
	{
		std::ostrstream out;
		const char* quote_start = cRFC1522_QUOTE_START_UTF8;
		switch(charset)
		{
		case i18n::eISO_8859_1:
			quote_start = cRFC1522_QUOTE_START_1;
			str = i18n::CCharsetManager::sCharsetManager.Transcode(i18n::eUTF8, charset, str);
			break;
		case i18n::eISO_8859_15:
			quote_start = cRFC1522_QUOTE_START_15;
			str = i18n::CCharsetManager::sCharsetManager.Transcode(i18n::eUTF8, charset, str);
			break;
		default:;
		}
		size_t size_quote_start = ::strlen(quote_start);

		// Start with standard RFC1522 info
		out << quote_start;
		unsigned long line_length = offset + size_quote_start + sizeof(cRFC1522_QUOTE_END) - 1 - 3;

		// Add each char and encode
		const char* p = str.c_str();
		while(*p)
		{
			// Now check for an ordinary unencoded character
			if ((addr_phrase ? cNoQuoteChar1522Addr : cNoQuoteChar1522)[(unsigned char) *p])
			{
				char d = *p++;

				// Use underscore for mapped space character
				if (d == ' ')
					d = '_';

				out.put(d);
				line_length++;
			}
			
			// Must be an encoded character
			else
			{
				out << '=';

				// Map to charset
				unsigned char d = *(unsigned char*)p++;

				// Do high nibble
				out.put(cHexChar[(d >> 4)]);

				// Do low nibble
				out.put(cHexChar[(d & 0x0F)]);
				
				line_length += 3;
			}
			
			// Check for exceed of line length (account for possible encoding of next char if it exists)
			if (*p && wrap && (line_length >= GetWrapLength()))
			{
				// Must do wrap
				out << cRFC1522_QUOTE_END << os_endl << " " << quote_start;
				
				line_length = 1 + size_quote_start + sizeof(cRFC1522_QUOTE_END) - 1 - 3;
			}
		}

		// Add trailer
		out << cRFC1522_QUOTE_END;

		// Copy stream back to string
		out << std::ends;
		str.steal(out.str());
	}

	return charset != i18n::eUSASCII;
}

// Decode string from RFC1522 to utf8
bool CRFC822::TextFrom1522(cdstring& str)
{
	const char* p = str;
	bool decode = false;
	bool did_decode = false;
	bool non_ascii = false;
	std::ostrstream* out = NULL;

	try
	{
		// Check for any encoded
		while(*p)
		{
			// Look for start of 1522 text '=?'
			if ((*p == '=') && (*(p+1) == '?'))
			{
				// See if this is the start of decoding
				if (!decode)
				{
					// Create an output stream for decoded data
					out = new std::ostrstream;

					// Copy previous set of unencoded chars into output buffer
					size_t length = p - str.c_str();
					if (length)
						out->write(str.c_str(), p - str.c_str());
				}

				// Advance over '=?' and point at charset descriptor
				p += 2;
				const char* charset_start = p;

				// Step to end of charset descriptor '?'
				while(*p && (*p != '?')) p++;

				// Parse error if no '?'
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Get end of charset string and advance over trailing '?'
				const char* charset_end = p++;

				// Determine charset
				cdstring charset_str(charset_start, charset_end - charset_start);
				
				// Handle RFC2231 language tags
				if (::strchr(charset_str.c_str(), '*'))
					*::strchr(charset_str.c_str_mod(), '*') = 0;

				// Map charset string to enum
				i18n::ECharsetCode charset = i18n::CCharsetManager::sCharsetManager.GetCodeFromName(charset_str);

				// Can we handle this charset?
				if (charset == i18n::eUnknown)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Parse error - must have valid encoding
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Get encoding character and advance
				char mode = *p++;

				// Parse error - must have '?' after encoding
				if (!*p || (*p != '?'))
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Punt over trailing '?' after encoding
				p++;

				// Parse error - must have data after encoding
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Point at text to decode
				const char* txt_start = p;

				// Step to end of encoding and terminate, advance
				while(*p && ((*p != '?') || (*(p+1) != '='))) p++;

				// Parse error
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				// Get end of text and punt over trailing '?='
				const char* txt_stop = p++;
				p++;

				// Filter text according to encoding
				if (txt_stop > txt_start)
				{
					CFilter* filter = NULL;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					LHandleStream data;
#elif __dest_os == __win32_os || __dest_os == __linux_os
					LMemFileStream data;
#else
#error __dest_os
#endif

					try
					{
						switch(mode)
						{
						case 'Q':
						case 'q':
							filter = new CQPFilter(i18n::eUSASCII, true, false, false, &data);
							
							// Do '_' -> ' ' conversion here before the qp decode, as the qp data
							// may contain legitimate encoded '_' characters
							{
								char* q = const_cast<char*>(txt_start);
								while(q < txt_stop)
								{
									if (*q == '_') *q = ' ';
									q++;
								}
							}
							break;
						case 'B':
						case 'b':
							filter = new CBase64Filter(i18n::eUSASCII, true, false, false, &data);
							break;
						default:
							// Parse error
							//CLOG_LOGTHROW(CGeneralException, -1L);
							throw CGeneralException(-1L);
						}

						// Filter into buffer
						long txt_len = txt_stop - txt_start;
						filter->PutBytes(txt_start, txt_len);
						delete filter;
						filter = NULL;

						// Copy buffer to stream
						{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
							StHandleLocker lock(data.GetDataHandle());
#endif

							// Convert from '_' to ' '
							unsigned long actual_len = data.GetLength();
							unsigned long len = actual_len;
							if (len != 0)
							{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
								char* p = *data.GetDataHandle();
#elif __dest_os == __win32_os || __dest_os == __linux_os
								char* p = data.DetachData();
								cdstring tmp;
								tmp.steal(p);
#else
#error __dest_os
#endif
								// Convert to utf8 if not already
								i18n::CCharsetManager::sCharsetManager.ToUTF8(charset, p, actual_len, *out);
							}
						}
					}
					catch (...)
					{
						//CLOG_LOGCATCH(...);

						// Clean up and throw up
						delete filter;
						//CLOG_LOGRETHROW;
						throw;
					}
				}

				// Set decode flag
				decode = true;
				
				// Set flag for last decode
				did_decode = true;
			}
			
			// Only do the copy after we've started any decoding
			else if (decode)
			{
				// Sniff to see if encoding run
				bool got_run = false;
				if (did_decode)
				{
					const char* q = p;
					while(*q && !got_run && did_decode)
					{
						switch(*q)
						{
						case ' ':
						case '\t':
							q++;
							break;
						case '=':
							if (*(q+1) == '?')
							{
								// Punt over the spaces
								p = q;
								got_run = true;
							}
							// Fall through
						default:
							// Don't have run of encoded chars => output as normal
							did_decode = false;
							break;
						}
					}
				}

				// Just copy existing char
				if (!got_run)
					out->put(*p++);
			}
			else
			{
				if (*(unsigned char*)p > 0x7F)
					non_ascii = true;
				p++;
			}
		}

		// Copy stream back to string if decode done
		if (decode)
		{
			*out << std::ends;
			str.steal(out->str());
			delete out;
			out = NULL;
		}
		else if (non_ascii)
		{
			// Attempt sensible 8-bit header interpretation
			if (!str.IsUTF8())
			{
				// Transcode from iso-8859-15
				str = i18n::CCharsetManager::sCharsetManager.Transcode(i18n::eISO_8859_15, i18n::eUTF8, str);
			}
		}
	}
	catch (...)
	{
		//CLOG_LOGCATCH(...);

		// Leave string undecoded if parse error
		decode = false;
		delete out;
	}


	return decode;
}

bool CRFC822::Needs2231(const cdstring& str)
{
	// Write out encoded text
	const char* p = str.c_str();
	while(*p)
	{
		// Check for non-ascii character
		if (*(unsigned char*)p++ > 0x7F)
			return true;
	}

	return false;
}

bool CRFC822::TextTo2231(cdstring& str)
{
	// Assume it needs encoding
	std::ostrstream out;

	// Determine charset to use
	const char* p = str.c_str();
	bool utf8 = false;
	while(*p)
	{
		if (*(unsigned char*)p++ > 0x7F)
		{
			utf8 = true;
			break;
		}
	}
	
	// Write out charset
	if (utf8)
		out << "utf-8''";
	else
		out << "us-ascii''";
	
	// Write out encoded text
	p = str.c_str();
	while(*p)
	{
		// Check for encoded character
		if (cNoQuoteChar2231[(unsigned char)*p] == 1)
		{
			out.put(*p++);
		}
		else
		{
			out.put('%');
			out.put(cHexChar[(((unsigned char) *p) >> 4)]);
			out.put(cHexChar[(((unsigned char) *p) & 0x0F)]);
			p++;
		}
	}
	
	out << std::ends;
	str.steal(out.str());

	return true;	
}

// Decode string from RFC2231 to utf8
bool CRFC822::TextFrom2231(cdstring& str)
{
	bool decode = true;
	const char* p = str;

	try
	{
		// Grab charset descriptor
		const char* charset_start = p;
		
		// Step to end of charset descriptor
		while(*p && (*p != '\'')) p++;
		
		// Parse error if no '\''
		if (!*p)
		{
			//CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get end of charset string and advance over trailing '\''
		const char* charset_end = p++;

		// Determine charset
		cdstring charset_str(charset_start, charset_end - charset_start);

		// Map charset string to enum
		i18n::ECharsetCode charset = i18n::CCharsetManager::sCharsetManager.GetCodeFromName(charset_str);

		// Can we handle this charset?
		if (charset == i18n::eUnknown)
		{
			//CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Parse error - must have valid language tag
		if (!*p)
		{
			//CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Grab language descriptor
		const char* lang_start = p;
		
		// Step to end of lang descriptor
		while(*p && (*p != '\'')) p++;
		
		// Parse error if no '\''
		if (!*p)
		{
			//CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get end of language string and advance over trailing '\''
		const char* lang_end = p++;

		// Determine charset
		cdstring lang_str(lang_start, lang_end - lang_start);

		// Remainder of string is encoded data - write to stream doing decode
		std::ostrstream decodedout;
		while(*p)
		{
			switch(*p)
			{
			case '%':
			{
				// Decode two hex chars
				p++;
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
				char c1 = *p++;
				if (!*p)
				{
					//CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
				char c2 = *p++;
				
				// Make actual char
				char c = (cFromHex[(unsigned char) c1] << 4);
				c |= cFromHex[(unsigned char)c2];
				decodedout.put(c);
				break;
			}
			default:
				decodedout.put(*p++);
			}
		}

		// Grab decoded string
		decodedout << std::ends;
		str.steal(decodedout.str());

		// Convert to utf8 if not already
		std::ostrstream out;
		i18n::CCharsetManager::sCharsetManager.ToUTF8(charset, str, str.length(), out);
		out << std::ends;
		str.steal(out.str());
	}
	catch (...)
	{
		//CLOG_LOGCATCH(...);

		// Leave string undecoded if parse error
		decode = false;
	}


	return decode;
}

#pragma mark ____________________________Date Handling

// Get RFC822 style date & time
cdstring CRFC822::GetRFC822Date(time_t use_date, long use_zone)
{
	// Day and month are manually inserted into the string
	return GetDate("%%s, %d %%s %Y %H:%M:%S", use_date, use_zone);
}

// Get UNIX mailbox style date & time
cdstring CRFC822::GetUNIXDate(time_t use_date, long use_zone)
{
	// Day and month are manually inserted into the string
	return GetDate("%%s %%s %d %H:%M:%S %Y", use_date, use_zone);
}

// Get UNIX mailbox style date & time
cdstring CRFC822::GetIMAPDate(time_t use_date, long use_zone)
{
	// Only the month is manually inserted into the string
	return GetDate("%d-%%s-%Y %H:%M:%S", use_date, use_zone, false);
}

static const char *day_name[]
	= {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static const char *month_name[]
	= {"Jan", "Feb", "Mar","Apr","May","Jun",
	   "Jul", "Aug",  "Sep", "Oct", "Nov","Dec"};

// Get formatted date & time
cdstring CRFC822::GetDate(const char* format, time_t use_date, long use_zone, bool dayname, bool monthname)
{
	// Get local time
	time_t local = (use_date ? use_date : ::time(NULL));
	struct tm* now = ::localtime(&local);

	// Turn into string
	char tst[256];
	::strftime(tst, 256, format, now);

	// Add non-locale specific day and month names
	char ts[256];
	if (dayname && monthname)
		::snprintf(ts, 256, tst, day_name[now->tm_wday], month_name[now->tm_mon]);
	else if (dayname)
		::snprintf(ts, 256, tst, day_name[now->tm_wday]);
	else if (monthname)
		::snprintf(ts, 256, tst, month_name[now->tm_mon]);
	else
		::strcpy(ts, tst);

 	long diff = (use_date ? ((use_zone / 100) * 60 + (use_zone % 100)) * 60 : 0);

 	if (!use_date)
 		diff = GetLocalZone();

	// No longer test timezone range - use whatever the OS provides
	char fmt[32];
	long hrs = diff/3600;
	long mins = diff/60 - hrs*60;
	if (diff >= 0)
		::snprintf(fmt, 32, " +%02ld%02ld", hrs, mins);
	else
		::snprintf(fmt, 32, " -%02ld%02ld", -hrs, -mins);
	::strcat(ts, fmt);

 	return cdstring(ts);
}

// Set date from text
void CRFC822::SetDate(char* theDate, time_t& date, long& zone)
{
	// Make sure string exists
	if (!theDate)
		return;

	try
	{
		struct tm tms;
#ifdef __GNUC__
		::bzero(&tms, sizeof(tms));
#else
		tms.tm_mday = 0;
		tms.tm_mon = 0;
		tms.tm_year = 0;
		tms.tm_sec= 0;
		tms.tm_min = 0;
		tms.tm_hour = 0;
		tms.tm_wday = 0;
		tms.tm_yday = 0;
		tms.tm_isdst = 0;
#endif

		char* p = theDate;

		// Nil existing
		date = 0;
		zone = 0;

		// Strip space
		while(*p == ' ') p++;

		// Compare with NULL IMAP string
		if (::strncmp(p, cNIL, 3) == 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Check for days
		unsigned long dayOfWeek = GetDayOfWeek(p);

		// Get day in month from next field or previous if no match
		if (dayOfWeek)
		{
			p = ::strtok(p, WHITE_SPACE ",");
			if (!p)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			p = ::strtok(NULL, DATE_DELIM);
		}
		else
			p = ::strtok(p, DATE_DELIM);
		if (!p)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Look for month or day number
		if (isdigit(*p))
		{
			// Get day number
			tms.tm_mday = ::atoi(p);
			if ((tms.tm_mday < 1) || (tms.tm_mday > 31))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Get month
			p = ::strtok(NULL, DATE_DELIM);
			if (!p)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			tms.tm_mon = GetMonth(p) - 1;
		}
		else
		{
			// Get month
			tms.tm_mon = GetMonth(p) - 1;
			p = ::strtok(NULL, DATE_DELIM);
			if (!p)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Get day number
			tms.tm_mday = ::atoi(p);
			if ((tms.tm_mday < 1) || (tms.tm_mday > 31))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

		// Get year
		p = ::strtok(NULL, DATE_DELIM);
		if (!p)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		tms.tm_year = ::atoi(p);
		if (tms.tm_year < 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// If year less than '70 => next century .'. add 100
		if (tms.tm_year < 70)
			tms.tm_year += 100;
		// If year greater than 1900 => century present .'. remove 1900
		if (tms.tm_year >= 1900)
			tms.tm_year -= 1900;

		// Next field should be time, separated by a space
		p = ::strtok(NULL, " ");
		if (!p)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get hour
		tms.tm_hour = ::atoi(p);
		if ((tms.tm_hour < 0) || (tms.tm_hour > 23))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		while(*p && (*p != ':')) p++;

		// Get minutes
		if (*p == ':')
		{
			p++;
			tms.tm_min = ::atoi(p);
			if ((tms.tm_min < 0) || (tms.tm_min > 59))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
		while(*p && (*p != ':') && (*p != ' ')) p++;

		// Get seconds (may not be present)
		if (*p == ':')
		{
			p++;
			tms.tm_sec = ::atoi(p);
			if ((tms.tm_sec < 0) || (tms.tm_sec > 61))	// Remember leap seconds!!
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
		else
			tms.tm_sec = 0;

		// Next field should be
		p = ::strtok(NULL, " ");

		// Try to get time zone
		if (p && *p)
			zone = GetZone(p);

#if __dest_os == __mac_os_x
		date = ::timegm(&tms);
#else
#if __dest_os == __linux_os
		// DST correction for mktime
		tms.tm_isdst = -1;
#endif
		date = ::mktime(&tms);
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Make sure string is at least empty
		date = 0;
		zone = 0;
	}

} // CEnvelope::SetDate

// Set date from text
void CRFC822::SetUNIXDate(char* theDate, time_t& date, long& zone)
{
	try
	{
		struct tm tms;
#ifdef __GNUC__
		::bzero(&tms, sizeof(tms));
#else
		tms.tm_mday = 0;
		tms.tm_mon = 0;
		tms.tm_year = 0;
		tms.tm_sec= 0;
		tms.tm_min = 0;
		tms.tm_hour = 0;
		tms.tm_wday = 0;
		tms.tm_yday = 0;
		tms.tm_isdst = 0;
#endif

		char* p = theDate;

		// Nil existing
		date = 0;
		zone = 0;

		// Make sure string exists
		if (!theDate)
			return;

		// Strip space
		while(*p == ' ') p++;

		// Check for days
		unsigned long dayOfWeek = GetDayOfWeek(p);

		// Punt white space
		p += 3;
		while(*p == ' ') p++;

		// Get month
		tms.tm_mon = GetMonth(p) - 1;

		// Punt whitespace
		p += 3;
		while(*p == ' ') p++;

		// Get day in month
		tms.tm_mday = ::strtol(p, &p, 10);
		if ((tms.tm_mday < 1) || (tms.tm_mday > 31))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Punt whitespace
		while(*p == ' ') p++;

		// Get hour
		tms.tm_hour = ::strtol(p, &p, 10);
		if ((tms.tm_hour < 0) || (tms.tm_hour > 23))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get minutes
		p++;
		tms.tm_min = ::strtol(p, &p, 10);
		if ((tms.tm_min < 0) || (tms.tm_min > 59))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get seconds (may not be present)
		if (*p == ':')
		{
			p++;
			tms.tm_sec = ::strtol(p, &p, 10);
			if ((tms.tm_sec < 0) || (tms.tm_sec > 61))	// Remember leap seconds!!
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
		else
			tms.tm_sec = 0;

		// Punt whitespace
		while(*p == ' ') p++;

		// Look for year or zone here
		if (isdigit(*p))
		{
			// Get year
			tms.tm_year = ::strtol(p, &p, 10) - 1900;
			if (tms.tm_year < 0)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Punt whitespace
			while(*p == ' ') p++;

			// Grab null-terminated zone
			char* q = p;
			while(*p && (*p != ' ')) p++;
			if (*p)
				*p++ = 0;

			// Try to convert time zone
			if (*q)
				zone = GetZone(q);
		}
		else
		{
			// Grab null-terminated zone
			char* q = p;
			while(*p && (*p != ' ')) p++;
			if (*p)
				*p++ = 0;

			// Try to convert time zone
			if (*q)
				zone = GetZone(q);

			// Punt whitespace
			while(*p == ' ') p++;

			// Get year
			tms.tm_year = ::strtol(p, &p, 10) - 1900;
			if (tms.tm_year < 0)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

#if __dest_os == __mac_os_x
		date = ::timegm(&tms);
#else
#if __dest_os == __linux_os
		// DST correction for mktime
		tms.tm_isdst = -1;
#endif
		date = ::mktime(&tms);
#endif
		
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Make sure string is at least empty
		date = 0;
		zone = 0;
	}

} // CEnvelope::SetUNIXDate

// Copy date text
cdstring CRFC822::GetTextDate(time_t date, long zone, bool for_display, bool long_date)
{
	cdstring result;


	// Create date string if not already specified
	if (date)
	{
		// Adjust for local zone if requested
		if (for_display && CPreferences::sPrefs->mUseLocalTimezone.GetValue())
		{
			long zone_hours = zone / 100;
			long zone_mins = zone % 100;
			
			// Convert to UTC
			date -= 60 * (zone_mins + 60 * (zone_hours));
			
			// Now convert to local
			date += GetLocalZone();
		}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Convert from posix (1970) to Mac (1904) time
		date -= _mac_msl_epoch_offset_;

		CFLocaleRef locale = ::CFLocaleCopyCurrent();
		if (locale != NULL)
		{
			CFAbsoluteTime abs_time;
			::UCConvertSecondsToCFAbsoluteTime(date, &abs_time);
			CFDateFormatterRef formatter = ::CFDateFormatterCreate(kCFAllocatorDefault, locale,
																	long_date ? kCFDateFormatterLongStyle : kCFDateFormatterShortStyle,
																	long_date ? kCFDateFormatterLongStyle : kCFDateFormatterShortStyle);
			if (formatter != NULL)
			{
				MyCFString format(::CFDateFormatterGetFormat(formatter));
				cdstring temp = format.GetString();
				
				// May need to alter long date format
				if (long_date)
				{
					bool change_format = false;
					
					// See if day name is present, if not add it
					if (temp.find('E') != cdstring::npos)
					{
						temp = "EEEE, ";
						temp += format.GetString();
						change_format = true;
					}
					
					// See if zone name is present, if so remove it
					if (temp.find('Z', 0, true) != cdstring::npos)
					{
						temp.erase(temp.find('Z', 0, true), 1);
						temp.trimspace();
						change_format = true;
					}
					
					// Set new format if change required
					if (change_format)
					{
						MyCFString newformat(temp, kCFStringEncodingUTF8);
						::CFDateFormatterSetFormat(formatter, newformat);
					}
				}
				
				// Get formatted date/time string
				MyCFString cfstr(::CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, formatter, abs_time), false);
				result = cfstr.GetString();

				::CFRelease(formatter);
			}
			::CFRelease(locale);
		}
#elif __dest_os == __win32_os
		struct tm* now_tm = ::localtime(&date);

		SYSTEMTIME sys;
		sys.wYear = now_tm->tm_year + 1900;
		sys.wMonth = now_tm->tm_mon + 1;		// Adjust for month offset
		sys.wDayOfWeek = now_tm->tm_wday;
		sys.wDay = now_tm->tm_mday;
		sys.wHour = now_tm->tm_hour;
		sys.wMinute = now_tm->tm_min;
		sys.wSecond = now_tm->tm_sec;
		sys.wMilliseconds = 0;

		cdustring udate;
		udate.reserve(256);
		::GetDateFormatW(LOCALE_USER_DEFAULT, long_date ? DATE_LONGDATE : DATE_SHORTDATE, &sys, NULL, udate.c_str_mod(), 256);
		
		cdustring utime;
		utime.reserve(256);
		::GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &sys, NULL, utime.c_str_mod(), 256);
		
		udate += L" ";
		udate += utime;
		result = udate.ToUTF8();
#else
		bool will_add_tz = (!for_display || !CPreferences::sPrefs->mUseLocalTimezone.GetValue());
		struct tm* now_tm = ::localtime(&date);
		result.reserve(256);
		::strftime(result.c_str_mod(), 256, long_date ? "%A, %B %d, %Y %X" : (will_add_tz ? "%x %X" : "%c"), now_tm);

		// unix locale may give us back non-utf8 string so test it and try conversion if not
		if (!result.IsUTF8())
		  result.FromISOToUTF8();
#endif

		// Add timezone if required
		if (!for_display || !CPreferences::sPrefs->mUseLocalTimezone.GetValue())
		{
			cdstring zone_str;
			zone_str.reserve(256);
			::snprintf(zone_str.c_str_mod(), 256, "%+05ld", zone);

			result += " ";
			result += zone_str;
		}
	}

	return result;


} // CRFC822::GetTextDate


// Parse search item date
unsigned long CRFC822::ParseDate(char* txt)
{
	// This maybe in dd-mmm-yyyy format or a single number
	if (txt && *txt && ::strchr(txt, '-'))
	{
		long day = 0;
		long month = 0;
		long year = 0;

		char* p = ::strtok(txt, "-");
		day = ::atol(p);

		p = ::strtok(NULL, "-");
		month = GetMonth(p);

		p = ::strtok(NULL, "-");
		year = ::atol(p);

		struct tm tms;
#ifdef __GNUC__
		::bzero(&tms, sizeof(tms));
#endif
		tms.tm_mday = day;
		tms.tm_mon = month - 1;
		tms.tm_year = year - 1900;
#ifndef __GNUC__
		tms.tm_sec= 0;
		tms.tm_min = 0;
		tms.tm_hour = 0;
		tms.tm_wday = 0;
		tms.tm_yday = 0;
		tms.tm_isdst = 0;
#endif

#if __dest_os == __mac_os_x
		return ::timegm(&tms);
#else
#if __dest_os == __linux_os
		// DST correction for mktime
		tms.tm_isdst = -1;
#endif
		return ::mktime(&tms);
#endif
	}
	else
		return 0;
}

// Get day of week (Sun = 1)
unsigned long CRFC822::GetDayOfWeek(char* day)
{
	::strupper(day);
#ifdef sparc
	long temp = (day[0] << 24) | (day[1] << 16) | (day[2] << 8) | day[3];
#else
	long temp = *((long*) day);
#endif
	long test = ntohl(temp) & 0xFFFFFF00;
	switch(test)
	{
	case 'SUN\0':
		return 1;
	case 'MON\0':
		return 2;
	case 'TUE\0':
		return 3;
	case 'WED\0':
		return 4;
	case 'THU\0':
		return 5;
	case 'FRI\0':
		return 6;
	case 'SAT\0':
		return 7;
	}
	
	// Don't throw just return 0 - day may legitimately not be present
	return 0;
}

// Get month (Jan = 1)
unsigned long CRFC822::GetMonth(char* month)
{
	::strupper(month);
#ifdef sparc
	long temp = (month[0] << 24) | (month[1] << 16) | (month[2] << 8) | month[3];
#else
	long temp = *((long*) month);
#endif
	long test = ntohl(temp) & 0xFFFFFF00;
	switch(test)
	{
	case 'JAN\0':
		return 1;
	case 'FEB\0':
		return 2;
	case 'MAR\0':
		return 3;
	case 'APR\0':
		return 4;
	case 'MAY\0':
		return 5;
	case 'JUN\0':
		return 6;
	case 'JUL\0':
		return 7;
	case 'AUG\0':
		return 8;
	case 'SEP\0':
		return 9;
	case 'OCT\0':
		return 10;
	case 'NOV\0':
		return 11;
	case 'DEC\0':
		return 12;
	}
	
	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	
	return 0;
}

// Get zone
long CRFC822::GetZone(char* zone)
{
	// Look for +/- or digit
	if ((*zone == '+') || (*zone == '-') || isdigit(*zone))
	{
		// Convert numeric value
		return ::atol(zone);
	}
	
	// Check for obsolete zone codes
	::strupper(zone);
	if (!::strcmp(zone, "UT") || !::strcmp(zone, "GMT"))
		return 0;
	else if (!::strcmp(zone, "EDT"))
		return -400;
	else if (!::strcmp(zone, "EST") || !::strcmp(zone, "CDT"))
		return -500;
	else if (!::strcmp(zone, "CST") || !::strcmp(zone, "MDT"))
		return -600;
	else if (!::strcmp(zone, "MST") || !::strcmp(zone, "PDT"))
		return -700;
	else if (!::strcmp(zone, "PST"))
		return -800;
	else
		return 0;
}

// Get local timezone offset in secs
long CRFC822::GetLocalZone()
{
	long diff = 0;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Must correct for daylight savings as MSL does not
	MachineLocation loc;
	ReadLocation(&loc);

	diff = loc.u.gmtDelta & 0x00FFFFFF;
	if (diff & 0x00800000)
		diff |= 0xFF000000;
#elif __dest_os == __win32_os
	// Replacement code reads time zone info direct from OS (code copied from MSL)
	// Note this is still wrong as Win32s does not support this API call
	TIME_ZONE_INFORMATION tzi;
	DWORD result = ::GetTimeZoneInformation(&tzi);
	switch(result)
	{
	case TIME_ZONE_ID_UNKNOWN:
		diff = -(tzi.Bias * 60);
		break;
	case TIME_ZONE_ID_STANDARD:
		diff = -((tzi.Bias + tzi.StandardBias) * 60);
		break;
	case TIME_ZONE_ID_DAYLIGHT:
		diff = -((tzi.Bias + tzi.DaylightBias) * 60);
		break;
	default:;
	}
#else
	// Get struct tm's for local and UTC
	time_t local = ::time(NULL);
	struct tm utc_now = *::gmtime(&local);
	struct tm local_now = *::localtime(&local);

	// Stupid correction because mktime needs this!
	utc_now.tm_isdst = -1;

	// Convert struct tm's to time_t's
	time_t utcnow = ::mktime(&utc_now);  
	time_t localnow = ::mktime(&local_now);

	// Get offset in time_t's
	diff = (long) ::difftime(localnow, utcnow);
#endif

	return diff;
}
