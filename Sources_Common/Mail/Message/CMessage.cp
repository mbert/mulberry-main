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


// Source for Message class

#include "CMessage.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CDataAttachment.h"
#include "CFilter.h"
#include "CFullFileStream.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessageAttachment.h"
#include "CMIMESupport.h"
#include "CRFC822.h"
#include "CRFC822Parser.h"
#include "CPreferences.h"
#include "CSMTPSender.h"
#include "CStreamAttachment.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStringUtils.h"

#include "ctrbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <strstream>

extern const char* cSpace;

// Get message flags namespace
using namespace NMessage;

#pragma mark ____________________________CMessageCache

CMessage::CMessageCache::CMessageCache()
{
	mEnvelope = NULL;
	mSize = 0;
	mInternalDate = 0;
	mInternalZone = 0;
	mHeader = NULL;
	mBody = NULL;
	mOwner = NULL;
	mCryptoInfo = NULL;
}

CMessage::CMessageCache::CMessageCache(const CMessageCache& copy)
{
	if (copy.mEnvelope)
		mEnvelope = new CEnvelope(*copy.mEnvelope);
	else
		mEnvelope = NULL;
	mSize = copy.mSize;

	mTxt_date = copy.mTxt_date;
	mTxt_ldate = copy.mTxt_ldate;
	mInternalDate = copy.mInternalDate;
	mInternalZone = copy.mInternalZone;

	// NB Header and body data are not cached in the copy - will be read on demand
	mHeader = NULL;
	if (copy.mBody)
		mBody = CAttachment::CopyAttachment(*copy.mBody);
	else
		mBody = NULL;

	mOwner = const_cast<CMessageCache&>(copy).mOwner;
	
	mCryptoInfo = copy.mCryptoInfo ? new CMessageCryptoInfo(*copy.mCryptoInfo) : NULL;
}

CMessage::CMessageCache::~CMessageCache()
{
	delete mEnvelope;
	delete mHeader;

	// Body owned by owner
	if (!mOwner)
		delete mBody;

	delete mCryptoInfo;

	mEnvelope = NULL;
	mHeader = NULL;
	mBody = NULL;
	mCryptoInfo = NULL;
}

#pragma mark ____________________________CMessage
// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E
// __________________________________________________________________________________________________

bool CMessage::sAllowClear = true;				// Indicates if delete flag clears

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructed from owning mbox only
CMessage::CMessage(CMbox* owner)
{
	// Common init
	InitMessage();

	SetMbox(owner);

} // CMessage::CMessage

// Constructed from owning message only
CMessage::CMessage(CMessage* owner)
{
	// Common init
	InitMessage();

	SetMbox(owner->GetMbox());

	// Must force to cached state since owner is also cached
	mCache = new CMessageCache;
	SetOwner(owner);
	if (mMbox && !GetOwner())
		mMbox->CachedMessage(this);

} // CMessage::CMessage

// Copy constructor
CMessage::CMessage(const CMessage &copy)
{
	// Common init
	InitMessage();

	SetMbox(const_cast<CMessage&>(copy).GetMbox());
	mNumber = copy.mNumber;
	mUID = copy.mUID;
	mFlags = copy.mFlags;

	if (copy.mCache)
		mCache = new CMessageCache(*copy.mCache);
	
	if (copy.mThread)
		mThread = new CThreadCache(*copy.mThread);

} // CMessage::CMessage

// Create RFC822 message from fields
CMessage::CMessage(const CIdentity* identity,
					const CDSN* dsn,
					CAddressList* toList,
					CAddressList* ccList,
					CAddressList* bccList,
					const char* subjectText,
					CAttachment* body,
					const CMessageList* replies,
					const char* bounced)
{
	// Common init
	InitMessage();

	const CIdentity* default_id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	try
	{
		// Force cache
		CacheMessage();

		// Create an envelope and assign to message
		CEnvelope* env = new CEnvelope;
		SetEnvelope(env);

		cdstring date = CRFC822::GetRFC822Date();
		env->SetDate(date);

		// Do not allow CRLFs in subject text
		env->GetSubject() = subjectText;
		char* subj = (char*) env->GetSubject().c_str();
		std::replace(subj, subj + ::strlen(subj), '\r', ' ');
		std::replace(subj, subj + ::strlen(subj), '\n', ' ');

		// From address
		if (!CAdminLock::sAdminLock.mLockIdentityFrom)
		{
			const cdstring& addrs = identity->GetFrom(true);
			env->SetFrom(new CAddressList(addrs, addrs.length()));
		}
		else
		{
			const cdstring& addrs = default_id->GetFrom(true);
			env->SetFrom(new CAddressList(addrs, addrs.length()));
		}

		// Reply-To address
		if (!CAdminLock::sAdminLock.mLockIdentityReplyTo)
		{
			// Just ignore if empty
			const cdstring& addrs = identity->GetReplyTo(true);
			if (!addrs.empty())
				env->SetReplyTo(new CAddressList(addrs, addrs.length()));
		}
		else if (default_id->UseReplyTo())
		{
			// Just ignore if empty
			const cdstring& addrs = default_id->GetReplyTo(true);
			if (!addrs.empty())
				env->SetReplyTo(new CAddressList(addrs, addrs.length()));
		}

		// Sender address
		if (!CAdminLock::sAdminLock.mLockIdentitySender)
		{
			// Just ignore if empty
			const cdstring& addrs = identity->GetSender(true);
			if (!addrs.empty())
				env->SetSender(new CAddressList(addrs, addrs.length()));
		}
		else if (default_id->UseSender())
		{
			// Just ignore if empty
			const cdstring& addrs = default_id->GetSender(true);
			if (!addrs.empty())
				env->SetSender(new CAddressList(addrs, addrs.length()));
		}

		// To address
		env->SetTo(toList);

		// CC address
		env->SetCC(ccList);

		// Bcc address
		env->SetBcc(bccList);

		// Message-ID generate
		{
			// Get left-side of message-id (first 24 chars of MD5 digest of time, clock and ctr)
			static unsigned long ctr = 1;
			cdstring lhs_txt;
			lhs_txt.reserve(256);
			::snprintf(lhs_txt.c_str_mod(), 256, "%lu.%lu.%lu", (time_t) clock(), time(NULL), ctr++);
			cdstring lhs;
			lhs_txt.md5(lhs);
			lhs[24UL] = 0;

			// Get right side (domain) of message-id
			cdstring rhs;
			cdstring host = CTCPSocket::TCPGetLocalHostName();
			host.trimspace();
			if (host.length())
			{
				// Must put IP numbers inside [..]
				if (CTCPSocket::TCPIsHostName(host))
					rhs = host;
				else
				{
					rhs = "[";
					rhs += host;
					rhs += "]";
				}
			}
			else
			{
				// Get the first from address
				cdstring domain;
				if (env->GetFrom() && (env->GetFrom()->size() != 0))
				{
					const CAddress* addr = env->GetFrom()->front();
					domain = addr->GetHost();
				}
				if (domain.empty())
				{
					domain = "localhost";
					domain += cdstring(ctr);
				}
					
				// Use first 24 chars of MD5 digest of the domain as the right-side of message-id
				domain.md5(rhs);
				rhs[24UL] = 0;
			}

			// Generate the message-id string
			cdstring msg_id;
			msg_id += "<";
			msg_id += lhs;
			msg_id += "@";
			msg_id += rhs;
			msg_id += ">";
			env->GetMessageID() = msg_id;
		}

		// In-Reply-To/References generate
		if (replies && replies->size())
		{
			// In-reply-to
			cdstring& in_reply_to = env->GetInReplyTo();
			bool done_refs = false;
			bool first = true;
			for(CMessageList::const_iterator iter = replies->begin(); iter != replies->end(); iter++)
			{
				// Accumulate reply to message ids into In-Reply-to
				if (first)
					first = false;
				else
					in_reply_to += cSpace;
				in_reply_to += (*iter)->GetEnvelope()->GetMessageID();
			}

			// References from first replied to message only
			// Cannot do multiple references chains
			cdstring refs;
			
			// Get references from cache or from headers
			if (replies->front()->GetEnvelope()->HasReferences())
				refs = replies->front()->GetEnvelope()->GetReferences();
			else
				replies->front()->GetHeaderField("References", refs);
			if (refs.length() != 0)
				refs += cSpace;
			refs += replies->front()->GetEnvelope()->GetMessageID();
			env->SetReferences(refs);
		}

		// Duplicate attachments and process for sending
		if (body)
			SetBody(CAttachment::CopyAttachment(*body));
		else
		{
			// Must always have some kind of body
			SetBody(new CDataAttachment((char*) NULL));
			GetBody()->GetContent().SetContent(eContentText, eContentSubPlain);
		}
		GetBody()->ProcessSend();

		// Create message header (must be done after body is assigned)
		// Create header for sending via smtp (i.e. bcc, (no identity), x-mulberries)
		CRFC822::CreateHeader(this,
								static_cast<CRFC822::ECreateHeaderFlags>(CRFC822::eAddBcc |
																			CRFC822::eAddXMulberry |
																			(GetBody()->IsDSN() ? CRFC822::eRejectDSN : 0) |
																			(GetBody()->IsMDN() ? CRFC822::eMDN : 0)),
								identity, dsn, bounced);

		// Calculate size - NB treats CRLF as one char
		SetSize((mCache->mHeader ? ::strlen(mCache->mHeader) : 0) + (GetBody() ? GetBody()->GetTotalSize() : 0));

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Delete bits
		delete mCache;
		mCache = NULL;
	}

} // CMessage::CMessage

// Default destructor
CMessage::~CMessage()
{
	UncacheMessage();
	ClearThreadInfo();

	mMbox = NULL;
	mCache = NULL;
	mThread = NULL;

} // CMessage::~CMessage


// O T H E R  M E T H O D S ____________________________________________________________________________

// Common init
void CMessage::InitMessage()
{
	SetMbox(NULL);
	mNumber = 0;
	mUID = 0;
	mFlags = eSeen;		// Messages start out seen and this flag is subtracted
	mCache = NULL;
	mThread = NULL;

} // CMessage::CMessage

// Get position in mbox
unsigned long CMessage::GetMessageNumber() const
{
	return (GetOwner() ? GetOwner()->GetMessageNumber() : mNumber);
}

// Create the cache
bool CMessage::CacheMessage()
{
	// Check whether required
	if (IsCached())
		// Must force caching of envelope/body if those are not present
		return !IsFullyCached();
	else
	{
		// Create brand new cache
		mCache = new CMessageCache;
		if (mMbox && !GetOwner())
			mMbox->CachedMessage(this);
		return true;
	}
}

// Make sure cached data is consistent otherwise uncache
void CMessage::ValidateCache()
{
	// Check whether cache is present
	if (IsCached())
	{
		// Look at cache items and make sure they are all present
		if (!mCache->mEnvelope || !mCache->mBody)
			UncacheMessage();
	}
}

// Remove the cached data
void CMessage::UncacheMessage()
{
	if (mCache && mMbox && !GetOwner())
		mMbox->UncachedMessage(this);
	delete mCache;
	mCache = NULL;
}

// Read flags of the message
void CMessage::ReadCache()
{
	// Read it in from mailbox if not already done
	if (!GetEnvelope() || !GetBody())
		GetMbox()->ReadCache(this);
} // CMessage::ReadFlags

// Get message URL
cdstring CMessage::GetURL(bool full) const
{
	cdstring url = mMbox->GetURL(full);
	url += "/;UID=";
	url += cdstring(mUID);
	
	return url;
}

// Set UID
void CMessage::SetUID(unsigned long uid, bool dont_sync)
{
	mUID = uid;

	// See if remote <-> disconected sync required
	if (!dont_sync && GetMbox() && !GetMbox()->IsSynchronising())
		GetMbox()->SyncDisconnectedMessage(this);
}

// Change flags
void CMessage::RemapUID(unsigned local_uid, unsigned long uid)
{
	// Change server UID to new one using loca_uid as index
	GetMbox()->RemapUID(local_uid, uid);

} // CMessage::ChangeFlags

// Get UID
unsigned long CMessage::GetUID() const
{
	return GetOwner() ? GetOwner()->GetUID() : mUID;
}


#pragma mark ____________________________Thread Cache

void CMessage::SetThreadInfo(unsigned long depth, const CMessage* parent, const CMessage* child, const CMessage* prev, const CMessage* next)
{
	if (!mThread)
		mThread = new CThreadCache;
	mThread->mDepth = depth;
	mThread->mParent = parent;
	mThread->mChild = child;
	mThread->mPrevious = prev;
	mThread->mNext = next;
}

void CMessage::ThreadPromote()
{
	// Require valid thread info
	if (!mThread)
		return;

	// Change depth of this one
	mThread->mDepth--;

	// Do for each child
	CMessage* child = const_cast<CMessage*>(GetThreadChild());
	while(child)
	{
		child->ThreadPromote();
		child = const_cast<CMessage*>(child->GetThreadNext());
	}
}

// Make it a fake message
void CMessage::MakeFake()
{
	// Cache it
	CacheMessage();
	
	// Set appropriate details
	GetFlags().Set(eFake | eDeleted);
	
	// Create a fake envelope
	if (!GetEnvelope())
		SetEnvelope(new CEnvelope);
	
	// Create a fake subject we can recognise later
	GetEnvelope()->FakeSubject();
	
	// Create a fake body
	if (!GetBody())
		SetBody(new CAttachment);
}

// Make it a fake message
void CMessage::MakeFakeSubject()
{
	// Re-do the fake to make sure its cached
	MakeFake();
	
	// Find a suitable child
	const CMessage* child = FindChildMsg();
	if (child && child->IsFullyCached())
		GetEnvelope()->SetSubject(child->GetEnvelope()->GetSubject());
}

// Find a valid (non-NULL) child message
const CMessage* CMessage::FindChildMsg() const
{
	// First look at each child
	const CMessage* child = GetThreadChild();
	while(child)
	{
		if (!child->IsFake())
			return child;
		child = child->GetThreadNext();
	}
	
	// Look at children of first child
	return child ? child->FindChildMsg() : NULL;
}

#pragma mark ____________________________Flags

// Set IMAP flags
bool CMessage::SetFlags(SBitFlags& new_flags)
{
	bool changed = false;
	bool has_mbox = GetMbox();

	// Handle partial flag: always set - never reset
	bool was_partial = IsPartial();

	// First check to see if different
	if ((GetFlags().Get() & eServerFlags) != (new_flags.Get() & eServerFlags))
	{
		// If change in Seen flag then update mailbox
		if ((GetFlags().IsSet(eSeen) ^ new_flags.IsSet(eSeen)) && has_mbox)
			GetMbox()->SetNumberUnseen(GetMbox()->GetNumberUnseen() + (new_flags.IsSet(eSeen) ? -1 : 1));

		// Reset flags
		GetFlags().Set(eServerFlags, false);
		GetFlags().Set(new_flags.Get() & eServerFlags);

		// If its deleted delete the header/text cache
		if (sAllowClear && GetFlags().IsSet(eDeleted) && CPreferences::sPrefs->mCloseDeleted.GetValue() && has_mbox)
			ClearMessage();

		// If new flag is seen always remove mbox recent flag
		if (GetFlags().IsSet(eMboxRecent) && GetFlags().IsSet(eSeen) && has_mbox)
		{
			GetFlags().Set(eMboxRecent, false);
			GetMbox()->SetMboxRecent(GetMbox()->GetMboxRecent() - 1);
		}

		changed = true;
	}

	if (was_partial)
		GetFlags().Set(ePartial);
	else if (new_flags.IsSet(ePartial))
	{
		GetFlags().Set(ePartial);
		GetFlags().Set(ePartialLocal);
	}
	else if (GetMbox() && GetMbox()->GetProtocol()->IsDisconnected())
		GetFlags().Set(eFullLocal);

	// Check whether flags have been previously set
	return changed;

} // CMessage::SetFlags

// Change flags
void CMessage::ChangeFlags(NMessage::EFlags flags, bool set)
{
	// Only change server related flags
	if (flags & eServerFlags)
		GetMbox()->SetFlagMessage(GetMessageNumber(), false, flags, set);
	else
		GetFlags().Set(flags, set);

} // CMessage::ChangeFlags

// Read flags of the message
void CMessage::ReadFlags()
{
	// Read it in from mailbox
	GetMbox()->ReadMessageFlags(this);

} // CMessage::ReadFlags

// Check seen flag (cannot be inline because of recursion)
bool CMessage::IsRecent() const
{
	return (GetOwner() ? GetOwner()->IsRecent() : GetFlags().IsSet(eRecent));
}

// Check new recent pseudo flag (cannot be inline because of recursion)
bool CMessage::IsCheckRecent() const
{
	return (GetOwner() ? GetOwner()->IsCheckRecent() : GetFlags().IsSet(eCheckRecent));
}

// Check new recent pseudo flag (cannot be inline because of recursion)
bool CMessage::IsMboxRecent() const
{
	return (GetOwner() ? GetOwner()->IsMboxRecent() : GetFlags().IsSet(eMboxRecent));
}

// Check flag (cannot be inline because of recursion)
bool CMessage::HasFlag(NMessage::EFlags flag) const
{
	return (GetOwner() ? GetOwner()->HasFlag(flag) : GetFlags().IsSet(flag));
}

// Check seen flag (cannot be inline because of recursion)
bool CMessage::IsUnseen() const
{
	return (GetOwner() ? GetOwner()->IsUnseen() : !GetFlags().IsSet(eSeen));
}

// Check deleted flag (cannot be inline because of recursion)
bool CMessage::IsDeleted() const
{
	return (GetOwner() ? GetOwner()->IsDeleted() : GetFlags().IsSet(eDeleted));
}

// Check answered flag (cannot be inline because of recursion)
bool CMessage::IsAnswered() const
{
	return (GetOwner() ? GetOwner()->IsAnswered() : GetFlags().IsSet(eAnswered));
}

// Check flagged flag (cannot be inline because of recursion)
bool CMessage::IsFlagged() const
{
	return (GetOwner() ? GetOwner()->IsFlagged() : GetFlags().IsSet(eFlagged));
}

// Check draft flag (cannot be inline because of recursion)
bool CMessage::IsDraft() const
{
	return (GetOwner() ? GetOwner()->IsDraft() : GetFlags().IsSet(eDraft));
}

// Check MDNSent flag (cannot be inline because of recursion)
bool CMessage::IsMDNSent() const
{
	return (GetOwner() ? GetOwner()->IsMDNSent() : GetFlags().IsSet(eMDNSent));
}

// Check Partial flag (cannot be inline because of recursion)
bool CMessage::IsPartial() const
{
	return (GetOwner() ? GetOwner()->IsPartial() : GetFlags().IsSet(ePartial));
}

// Check Error flag (cannot be inline because of recursion)
bool CMessage::IsError() const
{
	return (GetOwner() ? GetOwner()->IsError() : GetFlags().IsSet(NMessage::eError));
}

// Check MDNSent flag (cannot be inline because of recursion)
bool CMessage::HasLabel(unsigned long index) const
{
	if (index < eMaxLabels)
		return (GetOwner() ? GetOwner()->HasLabel(index) : GetFlags().IsSet(eLabel1 << index));
	else
		return false;
}

// Check Fake flag (cannot be inline because of recursion)
bool CMessage::IsFake() const
{
	return (GetOwner() ? GetOwner()->IsFake() : GetFlags().IsSet(NMessage::eFake));
}

// Check new recent pseudo flag (cannot be inline because of recursion)
bool CMessage::IsSearch() const
{
	return (GetOwner() ? GetOwner()->IsSearch() : GetFlags().IsSet(eSearch));
}

// Check full local pseudo flag (cannot be inline because of recursion)
bool CMessage::IsFullLocal() const
{
	return (GetOwner() ? GetOwner()->IsFullLocal() : GetFlags().IsSet(eFullLocal));
}

// Check partial local pseudo flag (cannot be inline because of recursion)
bool CMessage::IsPartialLocal() const
{
	return (GetOwner() ? GetOwner()->IsPartialLocal() : GetFlags().IsSet(ePartialLocal));
}

// Check for smart From address
bool CMessage::IsSmartFrom() const
{
	if (!GetFlags().IsSet(eSmartCheck))
		const_cast<CMessage*>(this)->TestSmartAddress();

	return GetFlags().IsSet(eSmartFrom);
}

// Check for smart To address
bool CMessage::IsSmartTo() const
{
	if (!GetFlags().IsSet(eSmartCheck))
		const_cast<CMessage*>(this)->TestSmartAddress();

	return GetFlags().IsSet(eSmartTo);
}

// Check for smart CC address
bool CMessage::IsSmartCC() const
{
	if (!GetFlags().IsSet(eSmartCheck))
		const_cast<CMessage*>(this)->TestSmartAddress();

	return GetFlags().IsSet(eSmartCC);
}

// Check for smart list address
bool CMessage::IsSmartList() const
{
	if (!GetFlags().IsSet(eSmartCheck))
		const_cast<CMessage*>(this)->TestSmartAddress();

	return GetFlags().IsUnset(eSmartFrom | eSmartTo | eSmartCC);
}

// Smart addresses reset
void CMessage::ResetSmart()
{
	// Reset flags
	GetFlags().Set(eSmartCheck, false);
}

// Do smart address checks
void CMessage::TestSmartAddress()
{
	// Only if envelope
	if (!GetEnvelope())
		return;

	// Reset flags
	GetFlags().Set(eSmartFrom | eSmartTo | eSmartCC, false);

	// Look for smart address in all froms
	for(CAddressList::const_iterator iter = GetEnvelope()->GetFrom()->begin(); iter != GetEnvelope()->GetFrom()->end(); iter++)
	{
		// Compare address with current user
		if (CPreferences::TestSmartAddress(**iter))
		{
			// Set smart-from flag
			GetFlags().Set(eSmartFrom);
			break;
		}
	}

	// Look for smart address in all tos
	for(CAddressList::const_iterator iter = GetEnvelope()->GetTo()->begin(); iter != GetEnvelope()->GetTo()->end(); iter++)
	{
		// Compare address with current user
		if (CPreferences::TestSmartAddress(**iter))
		{
			// Set smart-to flag
			GetFlags().Set(eSmartTo);
			break;
		}
	}

	// Look for smart address in all Ccs
	for(CAddressList::const_iterator iter = GetEnvelope()->GetCC()->begin(); iter != GetEnvelope()->GetCC()->end(); iter++)
	{
		// Compare address with current user
		if (CPreferences::TestSmartAddress(**iter))
		{
			// Set smart-cc flag
			GetFlags().Set(eSmartCC);
			break;
		}
	}

	// Set smart test flag
	GetFlags().Set(eSmartCheck);
}

#pragma mark ____________________________Others

// Read size of the message
void CMessage::ReadSize()
{
	// Read it in from mailbox
	GetMbox()->ReadSize(this);

} // CMessage::ReadSize

// Get size of a part
unsigned long CMessage::GetPartSize(const CAttachment* attach) const
{
	// Must have body for this to work
	if (GetBody() && !attach)
		attach = GetBody()->FirstDisplayPart();

	return attach ? attach->GetSize() : 0;

} // CMessage::GetPartSize

// Check if greater than size warning
unsigned long CMessage::CheckSizeWarning(bool all) const
{
	if (all)
	{
		return CPreferences::sPrefs->mDoSizeWarn.GetValue() &&
				(CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
				(GetSize() > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024L) ? GetSize() : 0;
	}
	else if (HasText())
	{
		const CAttachment* attach = const_cast<CMessage*>(this)->FirstDisplayPart();
		return !HasData(attach) &&
				CPreferences::sPrefs->mDoSizeWarn.GetValue() &&
				(CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
				(GetPartSize(attach) > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024L) ? GetPartSize(attach) : 0;
	}
	else
		return 0;
}

// Determine if a multipart message
bool CMessage::IsMultipart() const
{
	return (GetBody()) ? GetBody()->IsMultipart() : false;

} // CMessage::IsMultipart

// Set date from text
void CMessage::SetInternalDate(char* theDate, bool unixd)
{
	if (unixd)
		CRFC822::SetUNIXDate(theDate, mCache->mInternalDate, mCache->mInternalZone);
	else
		CRFC822::SetDate(theDate, mCache->mInternalDate, mCache->mInternalZone);

} // CMessage::SetDate

// Copy date text
cdstring CMessage::GetTextInternalDate(bool for_display, bool long_date) const
{
	if (for_display)
	{
		if (long_date)
		{
			// Create date string if not already specified
			if (mCache->mTxt_ldate.empty() && mCache->mInternalDate)
				mCache->mTxt_ldate = CRFC822::GetTextDate(mCache->mInternalDate, mCache->mInternalZone, for_display, long_date);

			return mCache->mTxt_ldate;
		}
		else
		{
			// Create date string if not already specified
			if (mCache->mTxt_date.empty() && mCache->mInternalDate)
				mCache->mTxt_date = CRFC822::GetTextDate(mCache->mInternalDate, mCache->mInternalZone, for_display, long_date);

			return mCache->mTxt_date;
		}
	}
	else
		return CRFC822::GetTextDate(mCache->mInternalDate, mCache->mInternalZone, for_display, long_date);

}

// Get date based on UTC
time_t CMessage::GetUTCInternalDate() const
{
	// Convert time zone into seconds
	unsigned long zone_secs = ((abs(mCache->mInternalZone) / 100) * 60 + (abs(mCache->mInternalZone) % 100)) * 60;

	if (mCache->mInternalZone < 0)
		return mCache->mInternalDate + zone_secs;
	else
		return mCache->mInternalDate - zone_secs;
}

// Get message header
char* CMessage::GetHeader()
{
	// Read it in if not already available
	if (!HasHeader())
		ReadHeader();

	return mCache->mHeader;
}

// Read in message header
void CMessage::ReadHeader()
{
	// Need header (not sub-message)
	if (!HasHeader())
	{
		// Do not allow delete while reading header
		sAllowClear = false;

		try
		{
			// Check whether this is part of a stream attachment
			if (dynamic_cast<CStreamAttachment*>(GetBody()) != NULL)
			{
				// Read from stream
				std::ostrstream out;
				costream stream_out(&out, lendl);
				WriteHeaderToStream(stream_out);
				out << std::ends;
				cdstring temp(out.str());
				SetHeader(temp.grab_c_str());
			}
			else
				GetMbox()->ReadHeader(this);
			if (GetOwner() != NULL)
			{
				// If still empty regenerate from envelope
				if (!HasHeader())
					// Add in bcc
					CRFC822::CreateHeader(this, CRFC822::eAddBcc, NULL, NULL, NULL, true);
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must reset flag
			sAllowClear = true;

			CLOG_LOGRETHROW;
			throw;
		}

		// Must reset flag
		sAllowClear = true;

	}
}

// Fetch specific header field
bool CMessage::GetHeaderField(const cdstring& field, cdstring& result)
{
	// For time being use sledge hammer approach and read in the entire header
	// Ultimately we want to do a header field retrieval from the server
	if (!HasHeader())
		ReadHeader();

	// Get it from existing cached header if available
	if (HasHeader())
		return CRFC822::HeaderSearch(GetHeader(), field, result);
	else
#if 0
		// Fetch specific header
		GetMbox()->FetchHeader(this, field, result);
#else
		return false;
#endif
}

// Read in part text
const char* CMessage::ReadPart(CAttachment* attach, bool peek)
{
	// If no attachment try first text part
	if (!attach)
	{
		attach = FirstDisplayPart();
	
		// Must read in via attachment
		if (attach)
			return attach->ReadPart(this, peek);
	}

	// Check that part exists and can be read
	if (attach && attach->CanDisplay())
	{
		// Do not allow delete while reading header
		sAllowClear = false;

		try
		{
			// Make its mailbox read it in if not already here or a different part
			ReadAttachment(attach, peek);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must reset flag
			sAllowClear = true;

			CLOG_LOGRETHROW;
			throw;
		}

		// Must reset flag
		sAllowClear = true;

		// Force seen flag to true if writeable and not peek
		if (!peek && IsUnseen() && !GetMbox()->IsReadOnly())
			ChangeFlags(eSeen, true);

		return attach->GetData();
	}
	else
		// Not found
		return NULL;

} // CMessage::ReadPart

// Read in part text
void CMessage::ReadAttachment(CAttachment* attach, bool peek, bool filter)
{
	// Do not allow delete while reading header
	sAllowClear = false;

	try
	{
		attach->ReadAttachment(this, peek, filter);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset flag
		sAllowClear = true;

		CLOG_LOGRETHROW;
		throw;
	}

	// Must reset flag
	sAllowClear = true;

} // CMessage::ReadAttachment

// Read in entire raw body
void CMessage::GetRawBody(std::ostream& out, bool peek)
{
	// Do not allow delete while reading data
	sAllowClear = false;

	try
	{
		// Check whether this is part of a stream attachment
		if (dynamic_cast<CStreamAttachment*>(GetBody()) != NULL)
		{
			// Read from stream
			costream stream_out(&out, lendl);
			unsigned long temp = 0;
			GetBody()->WriteToStream(stream_out, temp, false, NULL);
		}
		else
		{
			// Create a conversion stream and write to it
			costream stream_out(&out, lendl);
			GetMbox()->CopyAttachment(this, NULL, &stream_out, peek);
		}
			
		// Must add c-string terminator
		out << std::ends;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset flag
		sAllowClear = true;

		CLOG_LOGRETHROW;
		throw;
	}

	// Must reset flag
	sAllowClear = true;

}

// Are there any text parts?
bool CMessage::HasText() const
{
	// Test already done?
	if (!GetFlags().IsSet(eHasTextChecked))
	{
		// Do test and set flags
		const_cast<CMessage*>(this)->GetFlags().Set(eHasText, GetBody() ? (const_cast<CMessage*>(this)->FirstDisplayPart() != NULL) : false);
		const_cast<CMessage*>(this)->GetFlags().Set(eHasTextChecked);
	}

	return GetFlags().IsSet(eHasText);
}

// Read in part text
bool CMessage::HasData(const CAttachment* attach) const
{
	// If no number try first text part
	if (!attach)
		attach = const_cast<CMessage*>(this)->FirstDisplayPart();

	// Find attachment
	return attach && (attach->GetData() != NULL);
}

// Return pointer to complete RFC822 message with CRLFs
char* CMessage::GetRFC822CRLF() const
{
	std::ostrstream stream;

	// Get filtered header
	if (HasHeader())
	{
		CStreamFilter filter(new crlf_filterbuf(eEndl_CRLF), &stream);
		filter.write(mCache->mHeader, ::strlen(mCache->mHeader));
	}

	// Check for contents
	if (GetBody())
	{
		// Need suitable stream type
		costream stream_out(&stream, eEndl_CRLF);

		// Write using dummy files
		unsigned long temp = 0;
		GetBody()->WriteToStream(stream_out, temp, true, NULL);
		stream.flush();
	}

	// Convert stream to string
	stream << std::ends;
	return stream.str();
}

// Write message header to output stream
void CMessage::WriteHeaderToStream(costream& stream)
{
	// Only relevant for local message
}

// Write message to output stream
void CMessage::WriteToStream(costream& stream, bool dummy_files, CProgress* progress, bool always_mbox_copy)
{
	// If this message resides in a mailbox then do mailbox -> mailbox copy
	// If the message's main body is a stream attachment (i.e. its been modified locally by e.g. decryption/verification)
	// then do not do the mailbox copy instead do a stream copy
	// Had to add always_mbox_copy override to ensure that appends of a message from one store to another
	// use the original message, not the substitute stream, i.e. copy of a decrypted message copies
	// the encrypted content NOT the decrypted content
	if (GetMbox() && (always_mbox_copy || (dynamic_cast<CStreamAttachment*>(GetBody()) == NULL)))
	{
		// Special behaviour for partial message copy
		if (IsPartial())
		{
			// Send header first
			{
				// Must cache header
				bool has_header = HasHeader();
				cdstring temp_header;
				temp_header.steal(const_cast<char*>(CRFC822::RemoveMIMEHeaders(GetHeader())));

				// Get filtered header
				if (stream.IsLocalType())
					stream.Stream() << temp_header;
				else
				{
					CStreamFilter filter(new crlf_filterbuf(stream.GetEndlType()), stream.GetStream());
					filter << temp_header;
				}

				// Remove header if not previously cached
				if (!has_header)
				{
					delete mCache->mHeader;
					mCache->mHeader = NULL;
				}
			}

			// Must add back in MIME Version
			stream.Stream() << cHDR_MIME_VERSION << stream.endl();

			// Check for contents
			if (GetBody())
			{
				// Clone body
				CAttachment body_clone(*GetBody());

				// Find the first display part
				CAttachment* displayable = body_clone.FirstDisplayPart();

				// Now scan the parts and set them all to disposition missing, except the displayable one
				body_clone.SetNotCached(displayable);

				// Get count and start info from mailbox
				unsigned long count = GetMbox()->GetPartialCount();
				unsigned long start = GetMbox()->GetPartialStart();

				// Write using full files
				unsigned long temp = 0;
				body_clone.WriteToStream(stream, temp, dummy_files, progress, this, count, start);
				stream.Stream().flush();
			}
		}
		else
		{
			// Make sure partial flag gets turned off
			GetFlags().Set(ePartial, false);
			GetMbox()->CopyMessage(this, &stream);
		}
	}
	else
	{
		// Process as draft

		// Get filtered header
		cdstring temp_header;
		temp_header.steal(const_cast<char*>(CRFC822::RemoveMIMEHeaders(GetHeader())));

		if (stream.IsLocalType())
			stream.Stream().write(temp_header.c_str(), temp_header.length());
		else
		{
			CStreamFilter filter(new crlf_filterbuf(stream.GetEndlType()), stream.GetStream());
			filter.write(temp_header.c_str(), temp_header.length());
		}

		// Must add back in MIME Version
		stream.Stream() << cHDR_MIME_VERSION << stream.endl();

		// Check for contents
		if (GetBody())
		{
			// Write using full files
			unsigned long temp = 0;
			GetBody()->WriteToStream(stream, temp, dummy_files, progress);
			stream.Stream().flush();
		}
	}
}

// Set a new body
void CMessage::SetBody(CAttachment* body, bool delete_it)
{
	// Delete then reset the main body part
	if (delete_it)
		delete mCache->mBody;
	mCache->mBody = body;
	
	// Reset cached body data
	GetFlags().Set(eHasTextChecked, false);
}

// Set a new body and adjust headers
void CMessage::ReplaceBody(CStreamAttachment* body)
{
	std::ostrstream hdrs;

	// First grab old headers with MIME stuff stripped out
	cdstring temp_header;
	temp_header.steal(const_cast<char*>(CRFC822::RemoveMIMEHeaders(GetHeader())));
	
	// Add old headers to stream
	hdrs.write(temp_header.c_str(), temp_header.length());

	// Must add back in MIME Version
	hdrs << cHDR_MIME_VERSION << os_endl;

	// Now add in new body's MIME headers
	body->WriteHeaderToStream(hdrs);
	hdrs << std::ends;

	// Now set a new header
	cdstring temp(hdrs.str());
	SetHeader(temp.grab_c_str());

	// Now replace the actual body in this message
	SetBody(body);
}

// Extract part to disk
void CMessage::ExtractPart(CAttachment* part, bool view)
{
	// Find attachment
	if (part)
	{
		CMessage* top_message;
		if (GetOwner())
		{
			top_message = GetOwner();
			while(top_message->GetOwner())
				top_message = top_message->GetOwner();

		}
		else
			top_message = this;

		part->Extract(top_message, view);
	}
} // CMessage::ExtractPart

// Clear cached text
unsigned long CMessage::ClearMessage()
{
	unsigned long freed = 0;

	// Only do this if cached and not a fake
	if (!IsFake() && IsFullyCached())
	{
		// Clear header
		if (mCache->mHeader)
		{
			freed = ::strlen(mCache->mHeader) + 1;
			SetHeader(NULL);	// deletes it
		}

		// Clear all parts
		if (GetBody())
			freed += GetBody()->ClearAllData();
	}

	return freed;

} // CMessage::ClearMessage

#pragma mark ____________________________Digest

// Get number of messages in digest
unsigned long CMessage::CountDigest() const
{
	// See whether parent part is a digest (need to go up two levels as next level up is message/rfc822
	CAttachment* parent = GetBody()->GetParent();
	if (parent)
		parent = parent->GetParent();
	if (parent &&
		(parent->GetContent().GetContentSubtype() == eContentSubDigest))
		return parent->GetParts() ? parent->GetParts()->size() : 0;

	return 0;
}

// Get position in digest
unsigned long CMessage::GetDigestNumber() const
{
	// See whether parent part is a digest (need to go up two levels as next level up is message/rfc822
	CAttachment* parent = GetBody()->GetParent();
	if (parent)
		parent = parent->GetParent();
	if (parent &&
		(parent->GetContent().GetContentSubtype() == eContentSubDigest))
	{
		// Get position in parent
		const CAttachmentList* list = parent->GetParts();
		if (list)
		{
			CAttachmentList::const_iterator found = std::find(list->begin(), list->end(), GetBody()->GetParent());
			if (found != list->end())
				return (found - list->begin()) + 1;
		}
	}

	return 0;
}

// Get previous message in digest
CMessage* CMessage::GetPrevDigest() const
{
	// See whether parent part is a digest (need to go up two levels as next level up is message/rfc822
	CAttachment* parent = GetBody()->GetParent();
	if (parent)
		parent = parent->GetParent();
	if (parent &&
		(parent->GetContent().GetContentSubtype() == eContentSubDigest))
	{
		// Get position in parent
		const CAttachmentList* list = parent->GetParts();
		if (list)
		{
			CAttachmentList::const_iterator found = std::find(list->begin(), list->end(), GetBody()->GetParent());
			if (found != list->end())
			{
				// Bump down iterator and return message if not already at first
				while(found != list->begin())
				{
					found--;
					if ((*found)->IsMessage())
						return (*found)->GetMessage();
				}
			}
		}
	}

	return NULL;
}

// Get next message in digest
CMessage* CMessage::GetNextDigest() const
{
	// See whether parent part is a digest (need to go up two levels as next level up is message/rfc822
	CAttachment* parent = GetBody()->GetParent();
	if (parent)
		parent = parent->GetParent();
	if (parent &&
		(parent->GetContent().GetContentSubtype() == eContentSubDigest))
	{
		// Get position in parent
		const CAttachmentList* list = parent->GetParts();
		if (list)
		{
			CAttachmentList::const_iterator found = std::find(list->begin(), list->end(), GetBody()->GetParent());
			if (found != list->end())
			{
				// Bump up iterator and return message if not already at last
				while(found != list->end() - 1)
				{
					found++;
					if ((*found)->IsMessage())
						return (*found)->GetMessage();
				}
			}
		}
	}

	return NULL;
}


#pragma mark ____________________________MDNs

bool CMessage::MDNRequested()
{
	// Lokk for the header
	cdstring mdn_to;
	if (GetHeaderField("Disposition-Notification-To", mdn_to))
	{
		// Get the return path header
		cdstring return_path;
		cdstring received_from;
		cdstring received_by;
		cdstring received_for;
		cdstring received_date;
		
		GetDeliveryData(return_path, received_from, received_by, received_for, received_date);
		
		// Compare D-N-T with return-path (as required by RFC these MUST match)
		CAddress mdn_to_addr(mdn_to);
		CAddress return_addr(return_path);
		
		return mdn_to_addr == return_addr;
	}
	else
		return false;
}

// Generate the MDN message body
CAttachment* CMessage::CreateMDNSeenBody(const CIdentity* id, bool automatic)
{
	const CIdentity* default_id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Get the header
	cdstring mdn_to;
	cdstring orcpt;
	GetMDNData(mdn_to, orcpt);
	if (mdn_to.empty())
		return NULL;

	// Convert to address
	CAddress to_addr(mdn_to);

	// Create report type
	CAttachment* report = new CAttachment;

	// Add content type
	report->GetContent().SetContentType(eContentMultipart);
	
	// Add content subtype
	report->GetContent().SetContentSubtype(eContentSubReport);
	
	// Add parameters
	report->GetContent().SetContentParameter(cMIMEParameter[eReportType], cMIMEReportTypeParameter[eReportTypeMDN]);

	// Add text part
	CDataAttachment* text = new CDataAttachment;
	report->AddPart(text);

	// Add content type
	text->GetContent().SetContentType(eContentText);
	
	// Add content subtype
	text->GetContent().SetContentSubtype(eContentSubPlain);
	
	// Add parameters
	
	// Add data
	{
		std::ostrstream out;
		out << "The message sent on " << GetEnvelope()->GetTextDate(false);
		out << " to " << orcpt << " with subject \"" << GetEnvelope()->GetSubject();
		out << "\" has been displayed. This is no guarantee that the message has been read or understood." << os_endl;
		out << os_endl << std::ends;
		text->SetData(out.str());
	}

	// Add dsn part
	CDataAttachment* dsn = new CDataAttachment;
	report->AddPart(dsn);

	// Add content type
	dsn->GetContent().SetContentType(eContentMessage);
	
	// Add content subtype
	dsn->GetContent().SetContentSubtype(eContentSubDispositionNotification);
	
	// Add parameters

	// Add data
	{
		std::ostrstream out;

		// Reporting-UA:
		out << "Reporting-UA: ";
		cdstring host = CTCPSocket::TCPGetLocalHostName();
		host.trimspace();
		if (host.length())
		{
			// Must put IP numbers inside [..]
			if (CTCPSocket::TCPIsHostName(host))
				out << host.c_str() << "; ";
			else
				out << "[" << host.c_str() << "]; ";
		}
		out << CPreferences::sPrefs->GetMailerDetails(true) << os_endl;

		// Original-Recipient:
		if (!orcpt.empty())
			out << "Original-Recipient: rfc822; " << orcpt << os_endl;

		// Final-Recipient:
		out << "Final-Recipient: rfc822; ";
		if (!CAdminLock::sAdminLock.mLockIdentityFrom)
			out << id->GetFrom(true) << os_endl;
		else
			out << default_id->GetFrom(true) << os_endl;

		// Original-Message-ID:
		out << "Original-Message-ID: " << GetEnvelope()->GetMessageID() << os_endl;

		// Disposition:
		out << "Disposition: manual-action/";
		if (automatic)
			out << "MDN-sent-automatically; ";
		else
			out << "MDN-sent-manually; ";
		out << "displayed" << os_endl;

		// End of report
		out << os_endl << std::ends;
		dsn->SetData(out.str());
	}

	// Add headers part
	CDataAttachment* hdrs = new CDataAttachment;
	report->AddPart(hdrs);

	// Add content type
	hdrs->GetContent().SetContentType(eContentText);
	
	// Add content subtype
	hdrs->GetContent().SetContentSubtype(eContentSubRFC822Headers);
	
	// Add parameters

	// Add data
	hdrs->SetData(::strdup(GetHeader()));
	
	return report;
}

// Get MDN address details
void CMessage::GetMDNData(cdstring& mdn_to, cdstring& orcpt)
{
	// Get disposition notification to header
	GetHeaderField("Disposition-Notification-To", mdn_to);
	GetHeaderField("Original-Recipient", orcpt);

	// Get orcpt from received headers if not otherwise present
	if (!mdn_to.empty() && orcpt.empty())
	{
		// Read in the required DSN params
		cdstring return_path;
		cdstring received_from;
		cdstring received_for;
		cdstring received_date;
		
		GetDeliveryData(return_path, received_from, orcpt, received_for, received_date);
	}
}

#pragma mark ____________________________Reject

// Can reject this message?
bool CMessage::CanReject()
{
	// Read in the required DSN params
	cdstring return_path;
	cdstring received_from;
	cdstring received_by;
	cdstring received_for;
	cdstring received_date;
	
	GetDeliveryData(return_path, received_from, received_by, received_for, received_date);
	
	// Return-path must be a valid address
	CAddress addr(return_path);
	if (!addr.IsValid())
		return false;
	
	// Received: by must be valid
	if (received_by.empty())
		return false;
	
	// Received: for must be valid
	if (received_for.empty())
		return false;
	
	return true;
}

// Generate the reject message body
CAttachment* CMessage::CreateRejectDSNBody(bool return_msg)
{
	// Read in the required DSN params
	cdstring return_path;
	cdstring received_from;
	cdstring received_by;
	cdstring received_for;
	cdstring received_date;
	
	GetDeliveryData(return_path, received_from, received_by, received_for, received_date);
	CAddress to_addr(return_path);
	CAddress for_addr(received_for);

	// Create report type
	CAttachment* report = new CAttachment;

	// Add content type
	report->GetContent().SetContentType(eContentMultipart);
	
	// Add content subtype
	report->GetContent().SetContentSubtype(eContentSubReport);
	
	// Add parameters
	report->GetContent().SetContentParameter(cMIMEParameter[eReportType], cMIMEReportTypeParameter[eReportTypeDSN]);

	// Add text part
	CDataAttachment* text = new CDataAttachment;
	report->AddPart(text);

	// Add content type
	text->GetContent().SetContentType(eContentText);
	
	// Add content subtype
	text->GetContent().SetContentSubtype(eContentSubPlain);
	
	// Add parameters
	
	// Add data
	{
		std::ostrstream out;
		out << "The original message was received at " << received_date << os_endl;
		out << "from " << received_from << os_endl;
		out << os_endl;
		out << "   ----- The following addresses had permanent fatal errors -----" << os_endl;
		out << received_for << os_endl;
		out << os_endl;
		out << "   ----- Transcript of session follows -----" << os_endl;
		out << for_addr.GetMailbox() << ": Mailbox does not exist" << os_endl;
		out << "550 <" << for_addr.GetMailAddress() << ">... User unknown" << os_endl;
		out << os_endl << std::ends;
		text->SetData(out.str());
	}

	// Add dsn part
	CDataAttachment* dsn = new CDataAttachment;
	report->AddPart(dsn);

	// Add content type
	dsn->GetContent().SetContentType(eContentMessage);
	
	// Add content subtype
	dsn->GetContent().SetContentSubtype(eContentSubDeliveryStatus);
	
	// Add parameters

	// Add data
	{
		std::ostrstream out;
		out << "Reporting-MTA: dns; " << received_by << os_endl;
		out << "Arrival-Date: " << CRFC822::GetRFC822Date(GetInternalDate(), GetInternalZone()) << os_endl;
		out << os_endl;
		out << "Final-Recipient: rfc822; <" << for_addr.GetMailAddress() << ">" << os_endl;
		out << "Action: failed" << os_endl;
		out << "Status: 5.0.0" << os_endl;
		out << "Last-Attempt-Date: " << CRFC822::GetRFC822Date(GetInternalDate(), GetInternalZone()) << os_endl;
		out << os_endl << std::ends;
		dsn->SetData(out.str());
	}

	// Check for full message or headers
	if (return_msg)
	{
		// Add message part
		CMessageAttachment* msg = new CMessageAttachment(this, this);
		report->AddPart(msg);
	}
	else
	{
		// Add headers part
		CDataAttachment* hdrs = new CDataAttachment;
		report->AddPart(hdrs);

		// Add content type
		hdrs->GetContent().SetContentType(eContentText);
		
		// Add content subtype
		hdrs->GetContent().SetContentSubtype(eContentSubRFC822Headers);
		
		// Add parameters

		// Add data
		hdrs->SetData(::strdup(GetHeader()));
	}
	
	return report;
}

// Get delivery details
void CMessage::GetDeliveryData(cdstring& return_path, cdstring& received_from, cdstring& received_by,
								cdstring& received_for, cdstring& received_date)
{
	// Return path
	GetHeaderField("Return-Path", return_path);
	
	// Get first received
	cdstring received;
	GetHeaderField("Received", received);
	
	CRFC822Parser parser;

	// Parse received into parts we want
	char* p = received.c_str_mod();
	char c;
	
	// Get from token
	char* q = parser.ParseWord(p, " \t;");
	if (q)
	{
		c = *q;
		*q++ = 0;
	
		// Must have 'from'
		if (::strcmpnocase(p, "from"))
			return;
		p = q;
	}
	else
		return;

	// Get from domain
	q = parser.ParseWord(p, " \t;");
	if (q)
	{
		c = *q;
		*q++ = 0;
		received_from = p;
		p = q;
	}
	else
		return;

	// Punt space
	parser.Skipws(&p);
	
	// Get by token
	q = parser.ParseWord(p, " \t;");
	if (q)
	{
		c = *q;
		*q++ = 0;
	
		// Must have 'by'
		if (::strcmpnocase(p, "by"))
			return;
		p = q;
	}
	else
		return;

	// Get by domain
	q = parser.ParseWord(p, " \t;");
	if (q)
	{
		c = *q;
		*q++ = 0;
		received_by = p;
		p = q;
	}
	else
		return;

	// Punt space
	parser.Skipws(&p);
	
	// Look for optional info
	while (c != ';')
	{
		q = parser.ParseWord(p, " \t;");
		if (!q)
			return;
		c = *q;
		*q++ = 0;

		// Via/With/ID - ignore atom
		if (!::strcmpnocase(p, "via") ||
			!::strcmpnocase(p, "with") ||
			!::strcmpnocase(p, "id"))
		{
			p = q;
			q = parser.ParseWord(p, " \t;");
			if (!q)
				return;
			c = *q;
			*q++ = 0;
		}

		// For - grab it!
		else if (!::strcmpnocase(p, "for"))
		{
			p = q;
			q = parser.ParseWord(p, " \t;");
			if (!q)
				return;
			c = *q;
			*q++ = 0;
			received_for = p;
		}
		else
			return;
		
		p = q;
	}

	// Punt space
	parser.Skipws(&p);
	
	// Remainder is date
	received_date = p;
	
	// If no for, use To address
	if (received_for.empty() && GetEnvelope() &&
		GetEnvelope()->GetTo() && GetEnvelope()->GetTo()->size())
	{
		received_for = GetEnvelope()->GetTo()->front()->GetMailAddress();
	}
}
