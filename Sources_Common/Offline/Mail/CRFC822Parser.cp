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


// Source for an RFC822 Message parser

#include "CRFC822Parser.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CLocalAttachment.h"
#include "CEnvelope.h"
#include "CGeneralException.h"
#include "CMbox.h"
#include "CLocalMessage.h"
#include "CMailControl.h"
#include "CMessageList.h"
#include "CMIMESupport.h"
#include "CProgress.h"
#include "CRFC822.h"
#include "CStatusWindow.h"
#include "CStreamAttachment.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CXStringResources.h"

#include "cdstring.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <UProfiler.h>
#endif


const char* cRFC822_From = "From";
const char* cRFC822_Bcc = "Bcc";
const char* cRFC822_CC = "CC";
const char* cRFC822_Content = "Content-";
size_t cRFC822_Content_Length = 8;
const char* cRFC822_Date = "Date";
const char* cRFC822_InReplyTo = "In-Reply-To";
const char* cRFC822_MessageID = "Message-ID";
const char* cRFC822_MIMEVersion = "MIME-Version";
const char* cRFC822_ReplyTo = "Reply-To";
const char* cRFC822_Subject = "Subject";
const char* cRFC822_Sender = "Sender";
const char* cRFC822_To = "To";
const char* cRFC822_Content_ID = "ID";
const char* cRFC822_Content_Description = "Description";
const char* cRFC822_Content_Disposition = "Disposition";
const char* cRFC822_Content_Type = "Type";
const char* cRFC822_Content_TransferEncoding = "Transfer-Encoding";
const char* cRFC822_X_Mulberry_UID = "X-Mulberry-UID";

				/* parse-word specials */
const char *wspecials = " ()<>@,;:\\\"[]";
				/* parse-token specials for parsing */
const char *ptspecials = " ()<>@,;:\\\"[]/?=";

CRFC822Parser::CRFC822Parser(bool stream, CMessage* owner)
{
	mMsgList = NULL;
	mMsg = NULL;
	mOwner = owner;
	mAddrList = NULL;
	mProgress = NULL;
	mProgressStart = 0;
	mProgressTotal = 0;
	mStreamAttachment = stream;
	mValidMIMEVersion = false;
	mLastEndl2 = false;
}

CRFC822Parser::~CRFC822Parser()
{
	mMsgList = NULL;
	mMsg = NULL;
	mAddrList = NULL;
	mProgress = NULL;
}

#pragma push
#pragma global_optimizer on
#pragma optimization_level 4

CMessageList* CRFC822Parser::ListFromStream(std::istream& in, CProgress* progress)
{
	// This assumes an mbx format mailbox.
	// Therefore need to break down the mailbox into individual messages based
	// on the From xxxx line at the start of each message

	//StProfileSection profile("\pCRFC822Parser Profile", 200, 20);
	try
	{
		CBusyContext busy;
		cdstring desc = rsrc::GetString("Status::IMAP::Selecting");
		StMailBusy busy_lock(&busy, &desc);

		// Initialise progress indicator
		InitProgress(in, progress);

		// Create message list
		mMsgList = new CMessageList;

		// Look for From at start of line
		std::istream::pos_type msg_start = MessageStart(in);
		if (msg_start >= 0)
			in.seekg(msg_start);

		while(!in.fail())
		{
			// Look for manual abort
			if (CMailControl::ProcessBusy(&busy))
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Parse all messages from stream into list
			mMsgList->push_back(MessageFromStream(in));

			// Update progress
			UpdateProgress(in);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete mMsgList;
		mMsgList = NULL;
	}

	return mMsgList;
}

CLocalMessage* CRFC822Parser::MessageFromStream(std::istream& in, CProgress* progress, CMessage* owner)
{
	// Preserve old owner
	CLocalMessage* old = mMsg;

	try
	{
		// Init progress
		InitProgress(in, progress);

		// Create new message
		mMsg = owner ? new CLocalMessage(owner) : new CLocalMessage(static_cast<CMbox*>(NULL));
		if (!mMsg->IsFullyCached() && !mMsg->CacheMessage())
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		CEnvelope* env = new CEnvelope;
		mMsg->SetEnvelope(env);
		CLocalAttachment* attach = (mStreamAttachment ? new CStreamAttachment : new CLocalAttachment);
		mMsg->SetBody(attach);

		// Clear boundary stack if top level message
		if (!owner)
			mBoundaries.clear();

		// Cache starting position
		bool has_eof = false;
		long tellg = SafeTellg(in, has_eof);
		if (has_eof)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		mMsg->SetIndexStart(mStreamAttachment ? cStreamAttachmentIndexStart : tellg);
		attach->SetIndexStart(0);

		// Check to see if valid header in first line of top level message
		if (!owner)
		{
			cdstring temp;
			GetLine(in, temp);
			const char* p = temp;
			while(p && *p)
			{
				switch(*p++)
				{
				case ' ':
					// Attempt to parse internal date
					p = temp;

					// Step over From
					while(*p && (*p != ' ')) p++;
					while(*p == ' ') p++;

					// Step over user
					while(*p && (*p != ' ')) p++;
					while(*p == ' ') p++;

					// Now pointing at date part
					mMsg->SetInternalDate(const_cast<char*>(p), true);

					// Now terminate remainder of scan
					p = NULL;
					break;
				case ':':
					in.seekg(mMsg->GetIndexStart());
					p = NULL;
					break;
				default:;
				}
			}
		}

		// Marke the start of the header
		tellg = SafeTellg(in, has_eof);
		if (has_eof)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		mMsg->SetIndexHeaderStart(tellg - (long) mMsg->GetIndexStart());
		attach->SetIndexHeaderStart(mMsg->GetIndexHeaderStart());

		// Parse envelope from stream
		EnvelopeFromStream(in, *mMsg);

		// Check for valid MIME
		if (!mValidMIMEVersion)
		{
			// Force top-level to text/plain
			attach->SetContent(eContentText, eContentSubPlain);
		}

		// Cache header length, body start
		tellg = SafeTellg(in, has_eof);
		long ilen = tellg - (long) mMsg->GetIndexStart() - (long) mMsg->GetIndexHeaderStart();
		if (ilen < 0)
			ilen = 0;

		mMsg->SetIndexHeaderLength(ilen);
		mMsg->SetIndexBodyStart(tellg - (long) mMsg->GetIndexStart());

		attach->SetIndexHeaderLength(mMsg->GetIndexHeaderLength());
		attach->SetIndexBodyStart(mMsg->GetIndexBodyStart());

		// Set attachment indices
		// Parse body from stream
		ParseContent(attach, in);

		// Check for possible EOF
		tellg = SafeTellg(in, has_eof);
		if (has_eof)
		{
			// Cleared EOF error and seeked to end
		}

		// Adjust back to previous CRLF if some content exists
		else if (tellg > (long) mMsg->GetIndexStart() + (long) mMsg->GetIndexBodyStart())
			tellg -= (mLastEndl2 ? 2 : 1);

		// Cache remaining lengths
		ilen = tellg - mMsg->GetIndexStart() - mMsg->GetIndexBodyStart();
		if (ilen < 0)
			ilen = 0;

		mMsg->SetIndexBodyLength(ilen);
		mMsg->SetIndexLength(tellg - mMsg->GetIndexStart());

		attach->SetIndexBodyLength(mMsg->GetIndexBodyLength());
		attach->SetIndexLength(mMsg->GetIndexLength());

		// Set certain internal parameters
		mMsg->SetSize(mMsg->GetIndexLength());
		attach->GetContent().SetContentSize(attach->GetIndexLength());

		// Clear boundary stack if top level message
		if (!owner)
			mBoundaries.clear();

		// Force fail state back on
		if (has_eof)
			in.setstate(std::ios::failbit);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete mMsg;
		mMsg = old;

		// Clear boundary stack
		mBoundaries.clear();

		CLOG_LOGRETHROW;
		throw;
	}

	// Recover the old one and return the new one
	CLocalMessage* temp = mMsg;
	mMsg = old;
	return temp;
}

void CRFC822Parser::EnvelopeFromStream(std::istream& in, CLocalMessage& msg)
{
	try
	{
		// Read in entire header
		cdstring hdr;
		GetHeader(in, hdr);

		// Parse header into message
		ParseHeader(&msg, hdr);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		msg.SetEnvelope(NULL);
		CLOG_LOGRETHROW;
		throw;
	}
}

CAttachment* CRFC822Parser::AttachmentFromStream(std::istream& in, CLocalAttachment* parent)
{
	CLocalAttachment* attach = NULL;
	unsigned long msg_index_start = mMsg ? mMsg->GetIndexStart() : 0;

	try
	{
		// Now pointing at MIME headers of next part
		attach = (mStreamAttachment ? new CStreamAttachment : new CLocalAttachment);
		if (parent)
			parent->AddPart(attach);

		// Cache starting position
		bool has_eof = false;
		long tellg = SafeTellg(in, has_eof);
		if (has_eof)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		attach->SetIndexStart(tellg - (long) msg_index_start);
		attach->SetIndexHeaderStart(tellg - (long) msg_index_start);

		// Read in entire header
		cdstring hdr;
		GetHeader(in, hdr);

		// Parse MIME header into attachment
		ParseHeader(attach, hdr);

		// Cache header length, body start
		tellg = SafeTellg(in, has_eof);
		long ilen = tellg - (long) msg_index_start - (long) attach->GetIndexHeaderStart();
		if (ilen < 0)
			ilen = 0;

		attach->SetIndexHeaderLength(ilen);
		attach->SetIndexBodyStart(tellg - (long) msg_index_start);

		// Now parse MIME body of this part
		ParseContent(attach, in);

		// Check for possible EOF
		tellg = SafeTellg(in, has_eof);
		if (has_eof)
		{
			// Cleared EOF error and seeked to end
		}
		
		// Cannot have a negative size
		else if (tellg < (long) attach->GetIndexBodyStart())
			tellg = attach->GetIndexBodyStart();

		// Cache remaining lengths
		ilen = tellg - msg_index_start - attach->GetIndexBodyStart();
		if (ilen < 0)
			ilen = 0;

		attach->SetIndexBodyLength(ilen);
		attach->SetIndexLength(tellg - msg_index_start - attach->GetIndexStart());

		attach->GetContent().SetContentSize(attach->GetIndexLength());
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove and delete
		if (parent)
			parent->RemovePart(attach);
		CLOG_LOGRETHROW;
		throw;
	}
	
	return attach;
}

void CRFC822Parser::DefaultSubtype(CLocalAttachment* body)
{
	// Get its content
	CMIMEContent& content = body->GetContent();

	// Check for valid content type first
	if (content.GetContentType() == eNoContentType)
	{
		// Default to text/plain
		content.SetContent(eContentText, eContentSubPlain);
		
		// Look for parent
		CAttachment* parent = body->GetParent();
		
		// Special case of multipart/digest parent
		if (parent &&
			(parent->GetContent().GetContentType() == eContentMultipart) &&
			(parent->GetContent().GetContentSubtype() == eContentSubDigest))

			// Default child to message/rfc822
			content.SetContent(eContentMessage, eContentSubRFC822);
	}

	// Look for missing subtype and default
	if (content.GetContentSubtype() == eNoContentSubType)
	{
		switch(content.GetContentType())
		{
		case eNoContentType:		/* default is TEXT/PLAIN */
			content.SetContent(eContentText, eContentSubPlain);
			break;
		case eContentText:		/* default is TEXT/PLAIN */
			content.SetContentSubtype(eContentSubPlain);
			break;
		case eContentMultipart:		/* default is MULTIPART/MIXED */
			content.SetContentSubtype(eContentSubMixed);
			break;
		case eContentMessage:		/* default is MESSAGE/RFC822 */
			content.SetContentSubtype(eContentSubRFC822);
			break;
		case eContentApplication:		/* default is APPLICATION/OCTET-STREAM */
			content.SetContentSubtype(eContentSubOctetStream);
			break;
		case eContentAudio:		/* default is AUDIO/BASIC */
			content.SetContentSubtype(eContentSubBasic);
			break;
		default:			/* others have no default subtype */
			content.SetContentSubtype("unknown");
			break;
		}
	}
}

// Look for properly formatted 'From user date' line
std::istream::pos_type CRFC822Parser::MessageStart(std::istream& in, bool do_progress)
{
	bool got_LF = true;		// Start of processing equiv. to end of line last time

	// Speedier version with direct streambuf access
	std::streambuf* sb = in.rdbuf();
	std::streamoff posn = in.tellg();
	std::streamoff startn = posn;
	std::streamoff progress = 0;

	// Look for From at start of line
	bool done = false;
	while(!done)
	{
		int c = sb->sbumpc();
		posn++;
		progress++;
		if ((c == '\r') || (c == '\n'))
		{
			if ((c == '\r') && (sb->sgetc() == '\n'))
			{
				sb->sbumpc();
				posn++;
				progress++;
				mLastEndl2 = true;
			}
			else
				mLastEndl2 = false;
			got_LF = true;
			startn = posn;
		}
		else if (c == 'F')
		{
			if (got_LF)
			{
				bool matched = false;
				c = sb->sbumpc();
				if (c == 'r')
				{
					posn++;
					progress++;

					c = sb->sbumpc();
					if (c == 'o')
					{
						posn++;
						progress++;

						c = sb->sbumpc();
						if (c == 'm')
						{
							posn++;
							progress++;

							c = sb->sbumpc();
							if (c == ' ')
							{
								posn++;
								progress++;
								matched = true;
							}
						}
					}
				}

				if (matched)
				{
					// Get the entire current line
					cdstring line;
					GetLine(in, line);

					// See if its in a valid format
					if (ValidMessageStart(line))
						return startn;

					// After GetLine this must be true!
					got_LF = true;
					startn = posn = in.tellg();
				}
				else if (c == EOF)
				{
					done = true;
					in.setstate(std::ios::failbit);
				}
				else
				{
					sb->sungetc();
					got_LF = false;
				}
			}
			else
				got_LF = false;
		}
		else if (c == EOF)
		{
			done = true;
			in.setstate(std::ios::failbit);
		}
		else
			got_LF = false;

		if (progress > 8192)
		{
			if (do_progress)
				UpdateProgress(posn);
			progress = 0;
		}
	}

	return -1L;
}

bool CRFC822Parser::ValidMessageStart(const cdstring& start)
{
	const char* s = start.c_str() - 5;		// Points to where 'From ' would be but its not actually in this string
	const char* x;
	int ti = 0;
	int zn;

	// Punt x to end of line
	for (x = s + 5; *x; x++) {}

	if (x - s >= 41)
	{
		for (zn = -1; x[zn] != ' '; zn--) {}
		if ((x[zn-1] == 'm') && (x[zn-2] == 'o') && (x[zn-3] == 'r') &&
			(x[zn-4] == 'f') && (x[zn-5] == ' ') && (x[zn-6] == 'e') &&
			(x[zn-7] == 't') && (x[zn-8] == 'o') && (x[zn-9] == 'm') &&
			(x[zn-10] == 'e') && (x[zn-11] == 'r') && (x[zn-12] == ' '))
			x += zn - 12;
	}
	if (x - s >= 27)
	{
		if (x[-5] == ' ')
		{
			if (x[-8] == ':')
				zn = 0,ti = -5;
			else if (x[-9] == ' ')
				ti = zn = -9;
			else if ((x[-11] == ' ') && ((x[-10]=='+') || (x[-10]=='-')))
				ti = zn = -11;
		}
		else if (x[-4] == ' ')
		{
			if (x[-9] == ' ') zn = -4,ti = -9;
		}
		else if (x[-6] == ' ')
		{
			if ((x[-11] == ' ') && ((x[-5] == '+') || (x[-5] == '-')))
			zn = -6,ti = -11;
		}
		if (ti && !((x[ti - 3] == ':') &&
			(x[ti -= ((x[ti - 6] == ':') ? 9 : 6)] == ' ') &&
			(x[ti - 3] == ' ') && (x[ti - 7] == ' ') &&
			(x[ti - 11] == ' ')))
			ti = 0;
	}

	return ti;
}

void CRFC822Parser::GetLine(std::istream& in, cdstring& line)
{
	// Get current stream pos
	std::istream::pos_type line_start = in.tellg();

	// Speedier version with direct streambuf access
	std::streambuf* sb = in.rdbuf();

	// Find header end
	bool got_eol = false;
	bool got_lf = false;
	bool got_eof = false;
	while(!got_eol && !got_eof)
	{
		int s = sb->sbumpc();
		if ((s == '\r') || (s == '\n'))
		{
			if ((s == '\r') && (sb->sgetc() == '\n'))
			{
				s = sb->sbumpc();
				got_lf = true;
				mLastEndl2 = true;
			}
			else
				mLastEndl2 = false;
			got_eol = true;
		}
		else if (s == EOF)
		{
			got_eof = true;
			in.setstate(std::ios::failbit);
		}
	}

	// Check for possible EOF
	if (in.fail())
	{
		// Clear and go to last byte
		in.clear();
		in.seekg(0, std::ios_base::end);
	}

	// Get final stream position
	std::istream::pos_type line_end = in.tellg() - std::streamoff((got_eof ? 0L : (got_lf ? 2L : 1L)));

	// Copy from stream into line (don't include EOL)
	line.reserve(line_end - line_start + (std::istream::pos_type)1);
	in.seekg(line_start);
	in.read(line.c_str_mod(), line_end - line_start);
	if (!got_eof)
		in.ignore(got_lf ? 2 : 1);
}

void CRFC822Parser::GetHeader(std::istream& in, cdstring& hdr)
{
	// Get current stream pos
	std::istream::pos_type hdr_start = in.tellg();

	// Speedier version with direct streambuf access
	std::streambuf* sb = in.rdbuf();

	// Find header end
	bool got_eol1 = true;	// Must assume that previous char was line end
	bool got_eol2 = false;
	while(!got_eol2)
	{
		int s = sb->sbumpc();
		if ((s == '\r') || (s == '\n'))
		{
			if ((s == '\r') && (sb->sgetc() == '\n'))
			{
				s = sb->sbumpc();
				mLastEndl2 = true;
			}
			else
				mLastEndl2 = false;

			if (!got_eol1)
				got_eol1 = true;
			else
				got_eol2 = true;
		}
		else if (s == EOF)
		{
			got_eol2 = true;
			in.setstate(std::ios::failbit);
		}
		else
		{
			got_eol1 = false;
		}
	}

	// Get final stream position
	std::istream::pos_type hdr_end = in.tellg();
	if (hdr_end == std::istream::pos_type(-1L))
	{
		in.clear();
		in.seekg(0, std::ios_base::end);
		hdr_end = in.tellg();
	}

	// Copy from stream into header
	hdr.reserve(hdr_end - hdr_start + (std::istream::pos_type)1);
	in.seekg(hdr_start);
	in.read(hdr.c_str_mod(), hdr_end - hdr_start);
}

void CRFC822Parser::ParseHeader(CLocalMessage* msg, char* str)
{
	char* p = str;

	CEnvelope* env = msg->GetEnvelope();

	// Loop over entire header
	while(*p)
	{
		// Get pointer to start of line
		char* line = p;
		char* q = p;

		// Create c-string from line
		while(*p && *q)
		{
			if ((*p == '\r') || (*p == '\n'))
			{
				// Look for LWSP
				char c = *p;
				p++;
				if ((c == '\r') && (*p == '\n'))
				{
					p++;
					mLastEndl2 = true;
				}
				else
					mLastEndl2 = false;

				if ((*p != ' ') && (*p != '\t'))
					*q = 0;
				else
				{
					// Remove linear white space
					*q++ = ' ';
					while((*p == ' ') || (*p == '\t'))
						p++;
				}
			}
			else if (*p == '\t')
			{
				// Make it a space
				*q++ = ' ';
				p++;
			}
			else
				// Just copy it
				*q++ = *p++;
		}

		// Find header field
		if ((q = ::strchr(line, ':')) != NULL)
		{
			// Tie off field and flush spaces at end of field and start of field data
			char* r = q++;
			*r = 0;
			while(*q == ' ') q++;
			while(*--r == ' ') *r = 0;

			switch(toupper(*line))
			{
			case '>':
				// '> From:'
				if ((*(line+1) == ' ') && !::strcmpnocase(line+2, cRFC822_From))
					env->SetFrom(ParseAddressList(q));
				break;
			case 'B':
				// 'Bcc:'
				if (!::strcmpnocase(line, cRFC822_Bcc))
					env->SetBcc(ParseAddressList(q));
				break;
			case 'C':
				// 'CC:'
				if (!::strcmpnocase(line, cRFC822_CC))
					env->SetCC(ParseAddressList(q));

				// 'Content-xxx:'
				else if (!::strncmpnocase(line, cRFC822_Content, cRFC822_Content_Length))
					ParseContentHeader(dynamic_cast<CLocalAttachment*>(msg->GetBody()), line + 8, q);
				break;
			case 'D':
				// 'Date:'
				if (!::strcmpnocase(line, cRFC822_Date))
					env->SetDate(q);
				break;
			case 'F':
				// 'From:'
				if (!::strcmpnocase(line, cRFC822_From))
					env->SetFrom(ParseAddressList(q));
				break;
			case 'I':
				// 'In-Reply-To:'
				if (!::strcmpnocase(line, cRFC822_InReplyTo))
					env->GetInReplyTo() = q;
				break;
			case 'M':
				// 'Message-ID:'
				if (!::strcmpnocase(line, cRFC822_MessageID))
					env->GetMessageID() = q;

				// 'MIME-Version:'
				else if (!::strcmpnocase(line, cRFC822_MIMEVersion))
				{
					// Should check MIME version here
					mValidMIMEVersion = ParseMIMEVersion(q);
				}
				break;
			case 'R':
				// 'Reply-To:'
				if (!::strcmpnocase(line, cRFC822_ReplyTo))
					env->SetReplyTo(ParseAddressList(q));

				// 'References:'
				//else if (!::strcmpnocase(line, cRFC822_References))
					// env->SetReplyTo(ParseAddressList(q));
				break;
			case 'S':
				// 'Subject:'
				if (!::strcmpnocase(line, cRFC822_Subject))
				{
					env->GetSubject() = q;
					CRFC822::TextFrom1522(env->GetSubject());
				}

				// 'Sender:'
				else if (!::strcmpnocase(line, cRFC822_Sender))
					env->SetSender(ParseAddressList(q));
				break;
			case 'T':
				// 'To:'
				if (!::strcmpnocase(line, cRFC822_To))
					env->SetTo(ParseAddressList(q));
				break;
			case 'X':
				// 'X-Mulberry-UID:'
				if (!::strcmpnocase(line, cRFC822_X_Mulberry_UID))
				{
					unsigned long uid = ::strtoul(q, NULL, 10);
					//msg->SetUID(uid);
				}
				break;
			default:;
			}
		}
	}
}

void CRFC822Parser::ParseHeader(CLocalAttachment* attach, char* str)
{
	char* p = str;

	// Loop over entire header
	while(*p)
	{
		// Get pointer to start of line
		char* line = p;
		char* q = p;

		// Create c-string from line
		while(*p && *q)
		{
			if ((*p == '\r') || (*p == '\n'))
			{
				// Look for LWSP
				char c = *p;
				p++;
				if ((c == '\r') && (*p == '\n'))
				{
					p++;
					mLastEndl2 = true;
				}
				else
					mLastEndl2 = false;

				if ((*p != ' ') && (*p != '\t'))
					*q = 0;
				else
				{
					// Remove linear white space
					*q++ = ' ';
					while((*p == ' ') || (*p == '\t'))
						p++;
				}
			}
			else if (*p == '\t')
			{
				// Make it a space
				*q++ = ' ';
				p++;
			}
			else
				// Just copy it
				*q++ = *p++;
		}

		// Find header field
		if ((q = ::strchr(line, ':')) != NULL)
		{
			// Tie off field and flush spaces at end of field and start of field data
			char* r = q++;
			*r = 0;
			while(*q == ' ') q++;
			while(*--r == ' ') *r = 0;

			// Only interested in Content-xxx headers
			switch(toupper(*line))
			{
			case 'C':
				// 'Content-xxx:'
				if (!::strncmpnocase(line, cRFC822_Content, cRFC822_Content_Length))
					ParseContentHeader(attach, line + 8, q);
				break;
			default:;
			}
		}
	}
}

bool CRFC822Parser::ParseMIMEVersion(char* text)
{
	// From now on the presence of the MIME-Version header is enought to indicate MIME
	// We won't bother look at the actual content of the header. This is required because
	// some MUAs are broken and send back an invalid format.
	return true;

	// Here's the old code to test for the correct version format
#if 0
	const char* p = text;
	if ((*p++ != '1') || (*p++ != '.') || (*p++ != '0'))
		return false;
	if (!*p || isspace(*p))
		return true;
	return false;
#endif
}

void CRFC822Parser::ParseContent(CLocalAttachment* body, std::istream& in)
{
	// Assume attachment already has content, but default subtype just in case
	DefaultSubtype(body);

	// Look at each type in turn
	if (body->IsMultipart())
		ParseMultipart(body, in);
	else if (body->IsMessage())
		ParseMessagePart(body, in);
	else
		ParseSinglePart(body, in);
}

void CRFC822Parser::ParseMultipart(CLocalAttachment* body, std::istream& in)
{
	// Get current pos in case of error
	std::istream::pos_type start_multi = in.tellg();

	// Get current boundary
	cdstring boundary = "--";
	boundary += body->GetContent().GetMultipartBoundary();
	mBoundaries.push_back(boundary);

	// Speedier version with direct streambuf access
	std::streambuf* sb = in.rdbuf();

	// Now find boundary at start of line, punting over preamble
	bool done = false;
	bool boundary_at_from = false;
	int c = 0;
	while(!done)
	{
		// Parse to boundary
		boundary_at_from = ParseToBoundary(in, boundary, false);
		
		// Check error
		if (boundary_at_from || in.fail())
		{
			c = EOF;
			break;
		}

		// Check if end
		c = sb->sbumpc();
		if (c == '-')
		{
			c = sb->sbumpc();
			if (c == '-')
			{
				// Pop boundary
				mBoundaries.pop_back();

				// Check for postamble
				if (mBoundaries.size())
				{
					// Look for matching boundary
					boundary_at_from = ParseToBoundary(in, mBoundaries.back(), true);
				}
				else
				{
					// Must punt past any postamble to start of next message
					std::istream::pos_type next_msg = MessageStart(in);
					if (next_msg >= 0)
						// Adjust back
						in.seekg(next_msg);
				}

				// Mark as complete
				done = true;
			}
		}
		else if ((c == '\r') || (c == '\n'))
		{
			if ((c == '\r') && (sb->sgetc() == '\n'))
			{
				c = sb->sbumpc();
				mLastEndl2 = true;
			}
			else
				mLastEndl2 = false;

			// Now pointing at MIME headers of next part
			AttachmentFromStream(in, body);
		}
		else if (c == EOF)
			// This is extremely nasty - terminate this loop with predjudice
			done = true;
	}

	if (c == EOF)
	{
		// Ughh! This is a disaster - we have a multipart which is not terminated

		// Try rewind and convert this part to text/plain
		in.clear();
		in.seekg(start_multi);

		// Pop boundary
		mBoundaries.pop_back();

		// Convert this part to text/plain
		body->ClearParts();
		body->GetContent().SetContent(eContentText, eContentSubPlain);

		// This will parse to the end of the previous pending boundary or the next message
		ParseSinglePart(body, in);
	}
}

void CRFC822Parser::ParseMessagePart(CLocalAttachment* body, std::istream& in)
{
	// Parse it from the stream and add it to its owner
	body->SetMessage(MessageFromStream(in, NULL, mMsg ? mMsg : mOwner));
}

void CRFC822Parser::ParseSinglePart(CLocalAttachment* body, std::istream& in)
{
	// Check for current boundary
	const cdstring& boundary = mBoundaries.size() ? mBoundaries.back() : cdstring::null_str;
	if (boundary.empty())
	{
		// Find valid start of next message (with progress indicator)
		std::istream::pos_type next_msg = MessageStart(in, true);

		// if start exists (i.e. NOT last message in file) then seek back to it
		if (next_msg >= 0)
			in.seekg(next_msg);
	}
	else
		// Parse upto the next boundary
		ParseToBoundary(in, boundary, true);
}

bool CRFC822Parser::ParseToBoundary(std::istream& in, const cdstring& boundary, bool rewind)
{
	bool boundary_at_from = false;

	// Speedier version with direct streambuf access
	std::streambuf* sb = in.rdbuf();
	std::streamoff posn = in.tellg();
	std::streamoff progress = 0;

	// Reserve a buffer that is just bigger than the boundary
	// We will copy the stream data in here as it is parsed so that we can later
	// do a test for a valid From line
	cdstring from_buffer;
	from_buffer.reserve(boundary.length() + 64);

	// Parse while looking for boundary or valid From
	bool done = false;
	int c = '\n';	// Always start as if previous char was end of line
	while(!done)
	{
		if ((c == '\r') || (c == '\n'))
		{
			// Look for boundary start (include the preceeding CRLF)
			std::streamoff bdry = posn - 1;

			if ((c == '\r') && (sb->sgetc() == '\n'))
			{
				sb->sbumpc();
				posn++;
				progress++;
				mLastEndl2 = true;
			}
			else
				mLastEndl2 = false;

			// Look for boundary start
			const char* p = boundary;
			char* q = from_buffer.c_str_mod();
			char d = *p++;
			while(d)
			{
				*q++ = c = sb->sbumpc();
				posn++;
				progress++;
				if (d == c)
					d = *p++;
				else
					break;
			}

			// If full match we're done
			if (!d)
			{
				done = true;

				// Rewind to start of boundary (which includes preceeding CRLF)
				if (rewind)
					sb->pubseekpos(bdry, std::ios_base::in);
			}
			else
			{
				// Properly terminate from buffer c-string
				*q = 0;

				// Check for possible from
				bool have_from = true;
				const char* r = from_buffer.c_str();
				const char* from = "From ";
				const char* s = from;
				while(have_from && *s)
				{
					// Test for character in from buffer
					if (!*r)
					{
						// Read next character into from buffer
						*q++ = sb->sbumpc();
						posn++;
						progress++;
						*q = 0;
					}
					have_from = (*r++ == *s++);
				}

				if (have_from)
				{
					// Line to test does not include 'From ' at start
					cdstring line(from_buffer.c_str() + 5);

					// Check for entire line in from buffer
					if ((c != '\r') && (c != '\n'))
					{
						// Get the remainder of the line and add it to existing bit
						cdstring remaining_line;
						GetLine(in, remaining_line);
						line += remaining_line;
					}
					else
						// Remove the trailing endl
						*--q = 0;
					
					// Now check for valid message start
					if (ValidMessageStart(line))
					{
						boundary_at_from = true;
						done = true;

						// Rewind to start of delimiter (which includes preceeding CRLF)
						if (rewind)
							sb->pubseekpos(bdry, std::ios_base::in);
					}
				}
			}
		}
		else if (c == EOF)
		{
			done = true;
			in.setstate(std::ios::failbit);
		}
		else
		{
			c = sb->sbumpc();
			posn++;
			progress++;
		}

		if (progress > 8192)
		{
			UpdateProgress(posn);
			progress = 0;
		}
	}
	
	return boundary_at_from;
}

void CRFC822Parser::ParseContentHeader(CLocalAttachment* body, char* name, char* s)
{
	char c,*t;

	/* flush whitespace */
	if ((t = ::strchr (name,' ')) != NULL) *t = '\0';

	switch (toupper(*name))		/* see what kind of content */
	{
	case 'I':
		// Content-ID
		if (!::strcmpnocase(name, cRFC822_Content_ID))
			body->GetContent().SetContentId(s);
		break;

	case 'D':
		// Content-Description
		if (!::strcmpnocase(name, cRFC822_Content_Description))
			body->GetContent().SetContentDescription(s);

		// Content-Disposition
		else if (!::strcmpnocase(name, cRFC822_Content_Disposition))
		{
			/* get type word */
			if (!(name = ParseWord(s, ptspecials)))
				break;
			c = *name;		/* remember delimiter */
			*name = '\0';		/* tie off type */
			body->GetContent().SetContentDisposition(s);
			*name = c;		/* restore delimiter */
			Skipws(&name);	/* skip whitespace */

			// Parse disposition parameters
			ParseParameter(body, name, true);
		}
		break;

	case 'T':			/* possible Content-Type/Transfer-Encoding */
		// Content-Type
		if (!::strcmpnocase(name, cRFC822_Content_Type))
		{
			/* get type word */
			if (!(name = ParseWord(s,ptspecials))) break;
			c = *name;		/* remember delimiter */
			*name = '\0';		/* tie off type */

			// Set the type (WARNING: this may change the transfer encoding already parsed!)
			EContentTransferEncoding save_encoding = body->GetContent().GetTransferEncoding();
			body->GetContent().SetContentType(s);
			body->GetContent().SetTransferEncoding(save_encoding);

			*name = c;		/* restore delimiter */
			Skipws(&name);	/* skip whitespace */
			if ((*name == '/') &&	/* subtype? */
				((name = ParseWord((s = ++name), ptspecials)) != NULL))
			{
				c = *name;			/* save delimiter */
				*name = '\0';		/* tie off subtype */
				Skipws(&s);			/* copy subtype */
				//  (WARNING: this may change the transfer encoding already parsed!)
				EContentTransferEncoding save_encoding = body->GetContent().GetTransferEncoding();
				body->GetContent().SetContentSubtype(s);
				body->GetContent().SetTransferEncoding(save_encoding);
				*name = c;				/* restore delimiter */
				Skipws(&name);		/* skip whitespace */
			}
			else					/* no subtype */
			{
				if (!name)			/* did the fool have a subtype delimiter? */
				{
					name = s;		/* barf, restore pointer */
					Skipws(&name);	/* skip leading whitespace */
				}
			}

			// Parse type parameters
			ParseParameter(body, name, false);
		}

		// Content-Tansfer-Encoding
		else if (!::strcmpnocase(name, cRFC822_Content_TransferEncoding))
		{
			/* flush out any confusing whitespace */
			if ((t = ::strchr (s,' ')) != NULL) *t = '\0';

			// Set encoding
			body->GetContent().SetTransferEncoding(s);
		}
	break;

	default:			/* otherwise unknown */
		break;
	}
}

void CRFC822Parser::ParseParameter(CLocalAttachment* body, char* text, bool disposition)
{
	char c,*s;

	/* parameter list? */
	while (text && (*text == ';') && ((text = ParseWord((s = ++text), ptspecials)) != NULL))
	{
		cdstring param1;
		cdstring param2;

		c = *text;				/* remember delimiter */
		*text = '\0';			/* tie off attribute name */
		Skipws(&s);				/* skip leading attribute whitespace */
		if (!*s)
			*text = c;		/* must have an attribute name */
		else				/* instantiate a new parameter */
		{
			param1 = s;

			*text = c;		/* restore delimiter */
			Skipws(&text);	/* skip whitespace before equal sign */
			if ((*text != '=') ||	/* missing value is a no-no too */
				!(text = ParseWord((s = ++text), ptspecials)))
				param2 = "UNKNOWN_PARAMETER_VALUE";
			else			/* good, have equals sign */
			{
				c = *text;					/* remember delimiter */
				*text = '\0';				/* tie off value */
				Skipws(&s);					/* skip leading value whitespace */
				param2 = Quote(s);
				*text = c;		/* restore delimiter */
				Skipws(&text);
			}
		}

		if (disposition)
			body->GetContent().SetContentDispositionParameter(param1, param2);
		else
			body->GetContent().SetContentParameter(param1, param2);
	}
}

CAddressList* CRFC822Parser::ParseAddressList(char* string)
{
	try
	{
		// Create new address list
		mAddrList = new CAddressList;

		// Loop over entire string
		while(string)
		{
			CAddress* addr = ParseAddress(&string);
			if (addr)
			{
				// Check remainder
				if (string)
				{
					Skipws(&string);
					char c;
					switch(c = *string)
					{
					case ',':
						string++;
						break;
					default:
						// This is an error state
						mAddrList->push_back(new CAddress("INVALID_ADDRESS", NULL));
					case 0:
						string = NULL;
						break;
					}
				}
			}
			else if (string)
			{
				// This is an error state
				Skipws(&string);
				string = NULL;

				mAddrList->push_back(new CAddress("INVALID_ADDRESS", NULL));
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Just empty the address list
		if (mAddrList)
			mAddrList->clear();
	}

	return mAddrList;
}

CAddress* CRFC822Parser::ParseAddress(char** string)
{
	CAddress* adr;
	if (!*string) return NULL;
	Skipws(string);
	if (!**string) return NULL;
	if ((adr = ParseGroup(string)) != NULL)
	{
	}
	else if ((adr = ParseMailbox(string)) != NULL)
	{
		// Add to list now!
		mAddrList->push_back(adr);
	}
	else if (*string)
		adr = NULL;

	return adr;
}

CAddress* CRFC822Parser::ParseGroup(char** string)
{
	char* p;
	char* s;
	CAddress* adr;

	if (!*string) return NULL;	/* no string */
	Skipws(string);	/* skip leading WS */

	if (!**string ||		/* trailing whitespace or not group */
		((*(p = *string) != ':') && !(p = ParsePhrase(*string))))
		return NULL;

	s = p;			/* end of candidate phrase */
	Skipws(&s);		/* find delimiter */

	if (*s != ':')	/* not really a group */
		return NULL;

	*p = '\0';			/* tie off group name */
	p = ++s;			/* continue after the delimiter */
	Skipws(&p);			/* skip subsequent whitespace */

	/* write as address */
	adr = new CAddress;
	cdstring cpy(*string);
	Quote(cpy.c_str_mod());
	adr->SetMailbox(cpy);
	mAddrList->push_back(adr);

	*string = p;			/* continue after this point */
	while (*string && **string && (**string != ';'))
	{
		if ((adr = ParseAddress(string)) != NULL)
		{
			if (*string)
			{		/* anything more? */
				Skipws(string);	/* skip whitespace */
				switch(**string)	/* see what follows */
				{
				case ',':		/* another address? */
					++*string;		/* yes, skip past the comma */
				case ';':		/* end of group? */
				case '\0':		/* end of string */
					break;
				default:
					*string = NULL;	/* cancel remainder of parse */
					mAddrList->push_back(new CAddress("UNEXPECTED_DATA_AFTER_ADDRESS_IN_GROUP", NULL));
				}
			}
		}
		else
		{
			*string = NULL;		/* cancel remainder of parse */
			mAddrList->push_back(new CAddress("INVALID_ADDRESS_IN_GROUP", NULL));
		}
	}

	if (*string)		/* skip close delimiter */
	{
		if (**string == ';')
			++*string;
		Skipws(string);
	}
	/* append end of address mark to the list */
	return adr;			/* return the tail */
}

CAddress* CRFC822Parser::ParseMailbox(char **string)
{
	CAddress* adr = NULL;
	char* s;
	char* end;
	//parsephrase_t pp = (parsephrase_t) mail_parameters (NIL,GET_PARSEPHRASE,NIL);

	if (!*string) return NULL;	/* no string */
	Skipws(string);	/* flush leading whitespace */

	if (!**string) return NULL;	/* empty string */
	if (*(s = *string) == '<') 	/* note start, handle case of phraseless RA */
		adr = ParseRouteAddr(s, string);

	/* otherwise, expect at least one word */
	else if ((end = ParsePhrase(s)) != NULL)
	{
		if ((adr = ParseRouteAddr(end, string)) != NULL)
		{
			/* phrase is a personal name */
			*end = '\0';		/* tie off phrase */
			cdstring cpy(s);
			Quote(cpy.c_str_mod());
			adr->SetName(cpy);
			CRFC822::TextFrom1522(adr->GetName());
		}
		/* call external phraseparser if phrase only */
		//else if (pp && PhraseOnly(end) && (adr = (*pp) (s, end)))
		//{
		//	*string = end;		/* update parse pointer */
		//	Skipws(string);	/* skip WS in the normal way */
		//}
		else
			adr = ParseAddrSpec(s,string);
	}

	return adr;			/* return the address */
}

long CRFC822Parser::PhraseOnly(char *end)
{
	while (*end == ' ') ++end;	/* call rfc822_skipws() instead?? */
	switch (*end)
	{
	case '\0':
	case ',':
	case ';':
		return 1L;		/* is a phrase only */
	}
	return 0;			/* something other than phase is here */
}

CAddress* CRFC822Parser::ParseRouteAddr(char *string, char **ret)
{
	CAddress* adr;
	char *adl = NULL;
	char *routeend = NULL;
	if (!string) return NULL;
	Skipws(&string);	/* flush leading whitespace */

	/* must start with open broket */
	if (*string != '<') return NULL;
	if (string[1] == '@')	/* have an A-D-L? */
	{
		adl = ++string;				/* yes, remember that fact */
		while (*string != ':')		/* search for end of A-D-L */
		{
			/* punt if never found */
			if (!*string) return NULL;
			++string;			/* try next character */
		}
		*string = '\0';			/* tie off A-D-L */
		routeend = string;		/* remember in case need to put back */
	}

	/* parse address spec */
	if (!(adr = ParseAddrSpec(++string, ret)))
	{
		if (adl) *routeend = ':';	/* put colon back since parse barfed */
		return NULL;
	}

	/* have an A-D-L? */
	if (adl)
		adr->SetADL(adl);

	if (*ret)
	{
		if (**ret == '>')		/* make sure terminated OK */
		{
			++*ret;				/* skip past the broket */
			Skipws(ret);		/* flush trailing WS */
			if (!**ret)			/* wipe pointer if at end of string */
				*ret = NULL;
			return adr;			/* return the address */
		}
	}

	mAddrList->push_back(new CAddress("MISSING_MAILBOX_TERMINATOR"));
	return adr;			/* return the address */
}

CAddress* CRFC822Parser::ParseAddrSpec(char *string, char **ret)
{
	CAddress* adr;
	char *end;
	char c,*s,*t;

	if (!string) return NULL;	/* no string */
	Skipws(&string);			/* flush leading whitespace */
	if (!*string) return NULL;	/* empty string */

	/* find end of mailbox */
	if (!(end = ParseWord(string, NULL))) return NULL;

	adr = new CAddress;		/* create address block */
	c = *end;				/* remember delimiter */
	*end = '\0';			/* tie off mailbox */

	/* copy mailbox */
	cdstring cpy(string);
	Quote(cpy.c_str_mod());
	adr->SetMailbox(cpy);

	*end = c;			/* restore delimiter */
	t = end;			/* remember end of mailbox for no host case */
	Skipws(&end);		/* skip whitespace */

	if (*end == '@')	/* have host name? */
	{
		++end;				/* skip delimiter */
		Skipws(&end);		/* skip whitespace */
		if (*end == '[')	/* domain literal? */
		{
			string = end;		/* start of domain literal */
			if ((end = ParseWord(string + 1, "]\\")) != NULL)
			{
				size_t len = ++end - string;
				cdstring dup(string, len);
				adr->SetHost(dup);
			}
			else
			{
				adr->SetHost("INVALID_DOMAIN_LITETAL");
			}
		}

		/* search for end of host */
		else if ((end = ParseWord((string = end), wspecials)) != NULL)
		{
			c = *end;			/* remember delimiter */
			*end = '\0';		/* tie off host */
			/* copy host */
			cdstring cpy(string);
			Quote(cpy.c_str_mod());
			adr->SetHost(cpy);
			*end = c;			/* restore delimiter */
		}
		else
		{
			adr->SetHost("MISSING_INVALID_HOST_NAME");
		}
	}
	else
		end = t;			/* make person name default start after mbx */

	/* default host if missing */
	//if (!adr->host) adr->host = cpystr (defaulthost);

	if (end && adr->GetName().empty())	/* try person name in comments if missing */
	{
		while (*end == ' ') ++end;	/* see if we can find a person name here */
		if ((*end == '(') && ((s = SkipComment(&end, 1L)) != NULL) && ::strlen(s))
		{
			cdstring cpy(s);
			Quote(cpy.c_str_mod());
			adr->SetName(cpy);
			CRFC822::TextFrom1522(adr->GetName());
		}
		Skipws(&end);	/* skip any other WS in the normal way */
	}

	/* set return to end pointer */
	*ret = (end && *end) ? end : NULL;
	return adr;			/* return the address we got */
}

char* CRFC822Parser::ParsePhrase(char *s)
{
	char* curpos;
	if (!s) return NULL;		/* no-op if no string */

	/* find first word of phrase */
	curpos = ParseWord(s, NULL);
	if (!curpos) return NULL;	/* no words means no phrase */
	if (!*curpos) return curpos;	/* check if string ends with word */
	s = curpos;			/* sniff past the end of this word and WS */
	Skipws(&s);		/* skip whitespace */

	/* recurse to see if any more */
	return (s = ParsePhrase(s)) ? s : curpos;
}

char* CRFC822Parser::ParseWord(char *s, const char *delimiters)
{
	char *st,*str;
	if (!s) return NULL;		/* no string */
	Skipws(&s);					/* flush leading whitespace */

	if (!*s) return NULL;		/* empty string */

	/* default delimiters to standard */
	if (!delimiters) delimiters = wspecials;
	str = s;			/* hunt pointer for strpbrk */

	while(true)
	{			/* look for delimiter */
		if (!(st = ::strpbrk (str, delimiters)))
		{
			while (*s) ++s;		/* no delimiter, hunt for end */
				return s;			/* return it */
		}
		switch (*st)		/* dispatch based on delimiter */
		{
		case '"':			/* quoted string */
			/* look for close quote */
			while (*++st != '"')
			{
				switch (*st)
				{
				case '\0':			/* unbalanced quoted string */
					return NULL;	/* sick sick sick */
				case '\\':			/* quoted character */
					if (!*++st)		/* skip the next character */
						return NULL;
				default:			/* ordinary character */
					break;			/* no special action */
				}
			}
			str = ++st;				/* continue parse */
			break;
		case '\\':			/* quoted character */
			/* This is wrong; a quoted-pair can not be part of a word.  However,
			* domain-literal is parsed as a word and quoted-pairs can be used
			* *there*.  Either way, it's pretty pathological.
			*/
			if (st[1])
			{		/* not on NUL though... */
				str = st + 2;		/* skip quoted character and go on */
				break;
			}
		default:			/* found a word delimiter */
			return (st == s) ? NULL : st;
		}
	}
}

char* CRFC822Parser::Quote(char *src)
{
	char* ret = src;
	if (::strpbrk (src,"\\\""))	/* any quoting in string? */
	{
		char* dst = ret;
		while(*src)		/* copy string */
		{
			if (*src == '\"')
				src++;		/* skip double quote entirely */
			else
			{
				if (*src == '\\')
					src++;	/* skip over single quote, copy next always */
				*dst++ = *src++;	/* copy character */
			}
		}
		*dst = '\0';		/* tie off string */
	}
	return ret;			/* return our string */
}

void CRFC822Parser::Skipws(char** s)
{
  while (true)
  {
    if ((**s == ' ') || (**s == '\t'))
    	++*s;
    else if ((**s != '(') || !SkipComment(s, 0L))
    	return;
  }
}

char* CRFC822Parser::SkipComment(char** s, long trim)
{
	char *ret;
	char *s1 = *s;
	char *t = NULL;

		/* skip past whitespace */
	for (ret = ++s1; *ret == ' '; ret++) {}

	do
	{
		switch (*s1)
		{
		case '(':			/* nested comment? */
			if (!SkipComment(&s1, 0L))
				return NULL;
			t = --s1;			/* last significant char at end of comment */
			break;

		case ')':			/* end of comment? */
			*s = ++s1;			/* skip past end of comment */
			if (trim)			/* if level 0, must trim */
			{
				if (t)
					t[1] = '\0';	/* tie off comment string */
				else
					*ret = '\0';	/* empty comment */
			}
			return ret;

		case '\\':			/* quote next character? */
			if (*++s1)		/* next character non-null? */
			{
				t = s1;			/* update last significant character pointer */
				break;			/* all OK */
			}

		case '\0':			/* end of string */
			**s = '\0';			/* nuke duplicate messages in case reparse */
			return NULL;		/* this is wierd if it happens */

		case ' ':			/* whitespace isn't significant */
			break;

		default:			/* random character */
			t = s1;			/* update last significant character pointer */
			break;
		}
	} while (s1++);

	return NULL;			/* impossible, but pacify lint et al */
}

void CRFC822Parser::InitProgress(std::istream& in, CProgress* progress)
{
	// Initialise progress indicator
	if (progress)
	{
		mProgress = progress;
		mProgress->SetPercentage(0);
		mProgressStart = in.tellg();
		mProgressTotal = ::StreamLength(in);
	}
}

void CRFC822Parser::UpdateProgress(std::istream& in)
{
	UpdateProgress(in.tellg());
}

void CRFC822Parser::UpdateProgress(std::streamoff pos)
{
	if (mProgress)
	{
		std::streamoff current = (pos < 0) ? mProgressTotal : pos;
		unsigned long percent = 0;

		// These numbers may get big an overflow integer arithmetic
		if (mProgressTotal > 0x01000000)
		{
			double current_d = current - mProgressStart;
			current_d *= 100.0;
			double percent_d = current_d / ((double) mProgressTotal);
			percent = (unsigned long) percent_d;
		}
		else if (mProgressTotal > 0)
			percent = (100 * (current - mProgressStart)) / mProgressTotal;
		mProgress->SetPercentage(percent);
	}
}

long CRFC822Parser::SafeTellg(std::istream& in, bool& has_eof) const
{
	long end_pos = in.tellg();
	if (end_pos == -1L)
	{
		// Clear EOF error and seek to end
		in.clear();
		in.seekg(0, std::ios_base::end);
		end_pos = in.tellg();
		has_eof = true;
	}
	else
		has_eof = false;
	
	return end_pos;
}
