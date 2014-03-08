/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for Envelope class

#include "CEnvelope.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CINETCommon.h"
#include "CRFC822.h"
#include "CRFC822Parser.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"

#include "cdiomanip.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if __dest_os == __win32_os
#include <winsock.h>
#elif __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________CEnvelope::CEnvelopeIndex

CEnvelope::CEnvelopeIndex::CEnvelopeIndex()
{
	mDateStart = 0;
	mZoneStart = 0;
	mSubjectStart = 0;
	mSubjectLength = 0;
	mFromStart = 0;
	mFromLength = 0;
	mSenderStart = 0;
	mSenderLength = 0;
	mReplyToStart = 0;
	mReplyToLength = 0;
	mToStart = 0;
	mToLength = 0;
	mCCStart = 0;
	mCCLength = 0;
	mBccStart = 0;
	mBccLength = 0;
	mInReplyToStart = 0;
	mInReplyToLength = 0;
	mMessageIDStart = 0;
	mMessageIDLength = 0;
}

CEnvelope::CEnvelopeIndex::CEnvelopeIndex(const CEnvelopeIndex& copy)
{
	mDateStart = copy.mDateStart;
	mZoneStart = copy.mZoneStart;
	mSubjectStart = copy.mSubjectStart;
	mSubjectLength = copy.mSubjectLength;
	mFromStart = copy.mFromStart;
	mFromLength = copy.mFromLength;
	mSenderStart = copy.mSenderStart;
	mSenderLength = copy.mSenderLength;
	mReplyToStart = copy.mReplyToStart;
	mReplyToLength = copy.mReplyToLength;
	mToStart = copy.mToStart;
	mToLength = copy.mToLength;
	mCCStart = copy.mCCStart;
	mCCLength = copy.mCCLength;
	mBccStart = copy.mBccStart;
	mBccLength = copy.mBccLength;
	mInReplyToStart = copy.mInReplyToStart;
	mInReplyToLength = copy.mInReplyToLength;
	mMessageIDStart = copy.mMessageIDStart;
	mMessageIDLength = copy.mMessageIDLength;
}

// Stream ops
void CEnvelope::CEnvelopeIndex::WriteIndexToStream(std::ostream& out, long offset) const
{
	unsigned long items[20];

	items[0] = htonl(mDateStart ? mDateStart + offset : 0);
	items[1] = htonl(mZoneStart ? mZoneStart + offset : 0);

	items[2] = htonl(mSubjectStart ? mSubjectStart + offset : 0);
	items[3] = htonl(mSubjectLength);

	items[4] = htonl(mFromStart ? mFromStart + offset : 0);
	items[5] = htonl(mFromLength);

	items[6] = htonl(mSenderStart ? mSenderStart + offset : 0);
	items[7] = htonl(mSenderLength);

	items[8] = htonl(mReplyToStart ? mReplyToStart + offset : 0);
	items[9] = htonl(mReplyToLength);

	items[10] = htonl(mToStart ? mToStart + offset : 0);
	items[11] = htonl(mToLength);

	items[12] = htonl(mCCStart ? mCCStart + offset : 0);
	items[13] = htonl(mCCLength);

	items[14] = htonl(mBccStart ? mBccStart + offset : 0);
	items[15] = htonl(mBccLength);

	items[16] = htonl(mInReplyToStart ? mInReplyToStart + offset : 0);
	items[17] = htonl(mInReplyToLength);

	items[18] = htonl(mMessageIDStart ? mMessageIDStart + offset : 0);
	items[19] = htonl(mMessageIDLength);

	out.write(reinterpret_cast<const char*>(items), 20 * sizeof(unsigned long));
	out << cd_endl;
}

void CEnvelope::CEnvelopeIndex::ReadIndexFromStream(std::istream& in, unsigned long vers)
{
	// Read in array of items
	unsigned long items[20];
	in.read(reinterpret_cast<char*>(items), 20 * sizeof(unsigned long));
	
	// Assign items to indices
	mDateStart = ntohl(items[0]);
	mZoneStart = ntohl(items[1]);

	mSubjectStart = ntohl(items[2]);
	mSubjectLength = ntohl(items[3]);

	mFromStart = ntohl(items[4]);
	mFromLength = ntohl(items[5]);

	mSenderStart = ntohl(items[6]);
	mSenderLength = ntohl(items[7]);

	mReplyToStart = ntohl(items[8]);
	mReplyToLength = ntohl(items[9]);

	mToStart = ntohl(items[10]);
	mToLength = ntohl(items[11]);

	mCCStart = ntohl(items[12]);
	mCCLength = ntohl(items[13]);

	mBccStart = ntohl(items[14]);
	mBccLength = ntohl(items[15]);

	mInReplyToStart = ntohl(items[16]);
	mInReplyToLength = ntohl(items[17]);

	mMessageIDStart = ntohl(items[18]);
	mMessageIDLength = ntohl(items[19]);

	in.ignore();
}

#pragma mark ____________________________CEnvelope

// Default constructor
CEnvelope::CEnvelope()
{
	InitEnvelope();

	// Must always have these
	mFrom = new CAddressList;
	mSender = new CAddressList;
	mReply_to = new CAddressList;
	mTo = new CAddressList;
	mCc = new CAddressList;
	mBcc = new CAddressList;
}

// Constructor from bits
CEnvelope::CEnvelope(char* theDate,
						char* subject,
						CAddressList* from,
						CAddressList* sender,
						CAddressList* reply_to,
						CAddressList* to,
						CAddressList* cc,
						CAddressList* bcc,
						char* in_reply_to,
						char* message_id)
{
	InitEnvelope();

	SetDate(theDate);
	mSubject = subject;
	mFrom = from ? from : new CAddressList;
	mSender = sender ? sender : new CAddressList;
	mReply_to = reply_to ? reply_to : new CAddressList;
	mTo = to ? to : new CAddressList;
	mCc = cc ? cc : new CAddressList;
	mBcc = bcc ? bcc : new CAddressList;
	mIn_reply_to = in_reply_to;
	mMessage_id = message_id;

}

// Copy constructor
CEnvelope::CEnvelope(const CEnvelope &copy)
{
	InitEnvelope();

	// Now set from copy
	mDate = copy.mDate;
	mZone = copy.mZone;
	mTxt_date = copy.mTxt_date;
	mTxt_ldate = copy.mTxt_ldate;
	mSubject = copy.mSubject;
	if (copy.mFrom)
		mFrom = new CAddressList(*copy.mFrom);
	if (copy.mSender)
		mSender = new CAddressList(*copy.mSender);
	if (copy.mReply_to)
		mReply_to = new CAddressList(*copy.mReply_to);
	if (copy.mTo)
		mTo = new CAddressList(*copy.mTo);
	if (copy.mCc)
		mCc = new CAddressList(*copy.mCc);
	if (copy.mBcc)
		mBcc = new CAddressList(*copy.mBcc);
	mIn_reply_to = copy.mIn_reply_to;
	mMessage_id = copy.mMessage_id;
	mReferences = copy.mReferences;

}

// Default destructor
CEnvelope::~CEnvelope()
{
	delete mFrom;
	delete mSender;
	delete mReply_to;
	delete mTo;
	delete mCc;
	delete mBcc;

	mFrom = NULL;
	mSender = NULL;
	mReply_to = NULL;
	mTo = NULL;
	mCc = NULL;
	mBcc = NULL;
}


// Do common init
void CEnvelope::InitEnvelope()
{
	// Nil out first
	mDate = 0;
	mZone = 0;
	mFrom = NULL;
	mSender = NULL;
	mReply_to = NULL;
	mTo = NULL;
	mCc = NULL;
	mBcc = NULL;
	
	mReferences = "\1";		// Fake value to indicate its not been intialised

	mSubjectHash = 0;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do reconstruct recovery test
bool CEnvelope::RecoverCompare(const CEnvelope& comp) const
{
	// Each field must match exactly
	if ((mDate != comp.mDate) ||
		(mZone != comp.mZone) ||
		(mSubject != comp.mSubject) ||
		(mIn_reply_to != comp.mIn_reply_to) ||
		(mMessage_id != comp.mMessage_id))
		return false;
	
	// Test each address list as well
	if (((mFrom != NULL) ^ (comp.mFrom != NULL)) ||
		mFrom && !(*mFrom == *comp.mFrom))
		return false;

#if 0	// Can't compare these because of stupid IMAP defaulting procedure
	if (((mSender != NULL) ^ (comp.mSender != NULL)) ||
		mSender && (*mSender != *comp.mSender))
		return false;
	if (((mReply_to != NULL) ^ (comp.mReply_to != NULL)) ||
		mReply_to && (*mReply_to != *comp.mReply_to))
		return false;
#endif

	if (((mTo != NULL) ^ (comp.mTo != NULL)) ||
		mTo && (*mTo != *comp.mTo))
		return false;
	if (((mCc != NULL) ^ (comp.mCc != NULL)) ||
		mCc && (*mCc != *comp.mCc))
		return false;
	if (((mBcc != NULL) ^ (comp.mBcc != NULL)) ||
		mBcc && (*mBcc != *comp.mBcc))
		return false;

	// Must match at this point
	return true;
}

void CEnvelope::SetAddressField(CAddressList*& old_field, CAddressList* new_field)
{
	// Delete and replace the old one
	delete old_field;
	old_field = new_field;
	
	// Must always have some list present
	if (!old_field)
		old_field = new CAddressList;
}

// Set date from text
void CEnvelope::SetDate(char* theDate)
{
	CRFC822::SetDate(theDate, mDate, mZone);
}

// Copy date text
cdstring CEnvelope::GetTextDate(bool for_display, bool long_date) const
{
	if (for_display)
	{
		cdstring& date = long_date ? mTxt_ldate : mTxt_date;
		
		// Create date string if not already specified
		if (date.empty() && mDate)
			date = CRFC822::GetTextDate(mDate, mZone, for_display, long_date);

		return date;
	}
	else
		return CRFC822::GetTextDate(mDate, mZone, for_display, long_date);

}

// Get date based on UTC
time_t CEnvelope::GetUTCDate() const
{
	// Convert time zone into seconds
	unsigned long zone_secs = ((abs(mZone) / 100) * 60 + (abs(mZone) % 100)) * 60;

	if (mZone < 0)
		return mDate + zone_secs;
	else
		return mDate - zone_secs;
}

// Get subject as thread
unsigned long CEnvelope::GetThreadHash() const
{
	if (!mSubjectHash)
		const_cast<CEnvelope*>(this)->GenerateThreadData();
	return mSubjectHash;
}

// Get subject as thread
const char*	CEnvelope::GetThread(bool& re_subject) const
{
	if (!mSubjectHash)
		const_cast<CEnvelope*>(this)->GenerateThreadData();

	re_subject = (mSubjectNorm.length() != mSubject.length());
	return mSubjectNorm.c_str();
}

// Get normalised subject without collapsing whitespace
cdstring CEnvelope::GetMatchSubject() const
{
	return NormaliseSubject(true);
}


// Generate ordered subject info
void CEnvelope::GenerateThreadData()
{
	// Save the results, including a hash for fast comparisons
	mSubjectNorm = NormaliseSubject(false);
	mSubjectHash = mSubjectNorm.hash();
	if (!mSubjectHash)
		mSubjectHash = -1;
}

// Get normalised subject data
cdstring CEnvelope::NormaliseSubject(bool matching) const
{
	cdstring temp = mSubject;
	
	// Preserve case if matching
	if (!matching)
		::strlower(temp.c_str_mod());

	// Step 1 - remove multiple space, convert whitespace to space (only if not matching)
	char* p = temp.c_str_mod();
	if (!matching)
	{
		char* q = p;
		bool was_space = false;
		while(*p)
		{
			bool is_space = isspace(*p);
			if (is_space && was_space)
				p++;
			else
			{
				was_space = is_space;
				*q++ = (is_space ? ' ' : *p);
				p++;
			}
		}
		*p = 0;
		p = temp.c_str_mod();
	}

	// Step 6 may cause repeat
	bool repeat = true;
	while(repeat)
	{
		repeat = false;

		// Step 2 - remove trailer repeatedly
		bool changed = true;
		char* end_p = p + ::strlen(p) - 1;
		while(changed)
		{
			changed = false;
			
			// Remove trailing space
			while(p != end_p)
			{
				if (isspace(*end_p))
				{
					*end_p-- = 0;
					changed = true;
				}
				else
					break;
			}
			
			// Remove (fwd) at end
			if ((end_p - p >= 5) &&
				!strncmpnocase(end_p - 4, "(fwd)", 5))
			{
				end_p -= 4;
				*end_p-- = 0;
				changed = true;
			}
		}
		end_p = p + ::strlen(p) - 1;

		// Step 5 - repeat of 3 & 4
		changed = true;
		while(changed)
		{
			changed = false;
			
			// Step 3 - 
			{
				while(isspace(*p)) p++;

				// *sub-blob
				char* q = p;
				while(*q == '[')
				{
					while(*q && (*q != '[') && (*q != ']')) q++;
					if (*q == ']')
						q++;
					else
					{
						// Not a valid sub-blob - reset to beginning
						q = p;
						break;
					}
					while(isspace(*q)) q++;
				}
				
				// sub-refwd
				if ((!::strncmpnocase(q, "re", 2) && (q = q + 2) ||
					 !::strncmpnocase(q, "fwd", 3) && (q = q + 3) ||
					 !::strncmpnocase(q, "fw", 2) && (q = q + 2)))
				{
					while(isspace(*q)) q++;
					if (*q == '[')
					{
						while(*q && (*q != '[') && (*q != ']')) q++;
						if (*q == ']')
							q++;
						while(isspace(*q)) q++;
					}
					if (*q == ':')
					{
						p = ++q;
						changed = true;
					}
				}
			}
			
			// Step 4 - 
			{
				// sub-blob non-empty
				char* q = p;
				while(*q == '[')
				{
					while(*q && (*q != '[') && (*q != ']')) q++;
					if (*q == ']')
						q++;
					else
					{
						q = p;
						break;
					}
					while(isspace(*q)) q++;
				}
				if (*q && (p != q))
				{
					p = q;
					changed = true;
				}
			}
		}
		
		// Step 6
		if (!::strncmpnocase(p, "[fwd:", 5) && (*end_p == ']'))
		{
			p += 5;
			*end_p-- = 0;
			repeat = true;
		}
	}

	cdstring result(p);
	return result;
}

// Get summary headers to stream
void CEnvelope::GetSummary(std::ostream& out) const
{
	// From
	if (GetFrom()->size())
		CRFC822::AddAddressList(cHDR_FROM, false, GetFrom(), out, false);
	else
		out << cHDR_FROM << os_endl;

	// To
	if (GetTo()->size())
		CRFC822::AddAddressList(cHDR_TO, false, GetTo(), out, false);
	else
		out << cHDR_TO << os_endl;

	// CC
	if (GetCC()->size())
		CRFC822::AddAddressList(cHDR_CC, false, GetCC(), out, false);
	
	// BCC
	if (GetBcc()->size())
		CRFC822::AddAddressList(cHDR_BCC, false, GetBcc(), out, false);
	
	// Subject
	out << cHDR_SUBJECT << GetSubject() << os_endl;

	// Date Sent
	out << "Date-Sent: " << GetTextDate(true, true) << os_endl2;
}

// Write to cache
void CEnvelope::WriteCacheToStream(std::ostream& out, CEnvelopeIndex& index) const
{
	unsigned long offset = out.tellp();

	// Write out envelope information first
	index.SetDateIndex(0);
	::WriteHost(out, mDate);
	index.SetZoneIndex(out.tellp() - std::streamoff(offset));
	::WriteHost(out, mZone);
	out << cd_endl;

	index.SetSubjectIndex(out.tellp() - std::streamoff(offset));
	::Write1522(out, mSubject);
	index.SetSubjectLength(out.tellp() - std::streamoff(offset + index.GetSubjectIndex() + 1));

	index.SetFromIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mFrom);
	index.SetFromLength(out.tellp() - std::streamoff(offset + index.GetFromIndex() + 1));

	index.SetSenderIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mSender);
	index.SetSenderLength(out.tellp() - std::streamoff(offset + index.GetSenderIndex() + 1));

	index.SetReplyToIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mReply_to);
	index.SetReplyToLength(out.tellp() - std::streamoff(offset + index.GetReplyToIndex() + 1));

	index.SetToIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mTo);
	index.SetToLength(out.tellp() - std::streamoff(offset + index.GetToIndex() + 1));

	index.SetCCIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mCc);
	index.SetCCLength(out.tellp() - std::streamoff(offset + index.GetCCIndex() + 1));

	index.SetBccIndex(out.tellp() - std::streamoff(offset));
	WriteAddressListToStream(out, mBcc);
	index.SetBccLength(out.tellp() - std::streamoff(offset + index.GetBccIndex() + 1));

	index.SetInReplyToIndex(out.tellp() - std::streamoff(offset));
	index.SetInReplyToLength(mIn_reply_to.length());
	out << mIn_reply_to << cd_endl;

	index.SetMessageIDIndex(out.tellp() - std::streamoff(offset));
	index.SetMessageIDLength(mMessage_id.length());
	out << mMessage_id << cd_endl;
}

// Write to cache
void CEnvelope::WriteAddressListToStream(std::ostream& out, const CAddressList* list) const
{
	// Write out address list in RFC822 style but not wrapped
	if (list)
	{
		bool add_comma = false;
		for(CAddressList::const_iterator iter = list->begin(); iter != list->end(); iter++)
		{
			// Form first address (encode for RFC1522 so cache is portable)
			cdstring addr = (*iter)->GetFullAddress(true);

			// Add comma if required
			if (add_comma)
				out << COMMA_SPACE;

			// Add next address to line and make sure comma gets used if more to come
			out << addr;
			add_comma = true;
		}
	}

	out << cd_endl;
}

// Read from cache
void CEnvelope::ReadCacheFromStream(std::istream& in, unsigned long vers)
{
	// Write out envelope information first
	::ReadHost(in, (long&)mDate);
	::ReadHost(in, mZone);
	in.ignore();
	::Read1522(in, mSubject);
	SetFrom(ReadAddressListFromStream(in, vers));
	SetSender(ReadAddressListFromStream(in, vers));
	SetReplyTo(ReadAddressListFromStream(in, vers));
	SetTo(ReadAddressListFromStream(in, vers));
	SetCC(ReadAddressListFromStream(in, vers));
	SetBcc(ReadAddressListFromStream(in, vers));
	getline(in, mIn_reply_to);
	getline(in, mMessage_id);
	
	// Must default Reply-To and Sender to From if empty (IMAP-ism)
	//if (!GetReplyTo()->size())
	//	SetReplyTo(new CAddressList(*GetFrom()));
	//if (!GetSender()->size())
	//	SetSender(new CAddressList(*GetFrom()));
}

// Read from cache
CAddressList* CEnvelope::ReadAddressListFromStream(std::istream& in, unsigned long vers) const
{
	cdstring temp;
	getline(in, temp);
	CRFC822Parser parser;

	return (temp.empty() ? new CAddressList : parser.ParseAddressList(temp));
}
