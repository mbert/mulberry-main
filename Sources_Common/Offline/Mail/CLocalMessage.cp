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


// Source for Local Message class

#include "CLocalMessage.h"

#include "CLocalAttachment.h"
#include "CMbox.h"
#include "CStreamAttachment.h"
#include "CStreamType.h"
#include "CStreamUtils.h"

#include "cdiomanip.h"

#include <istream>

// Constructed from owning mbox only
CLocalMessage::CLocalMessage(CMbox* owner)
	: CMessage(owner)
{
	InitLocalMessage();
}

// Constructed from owning message only
CLocalMessage::CLocalMessage(CMessage* owner)
	: CMessage(owner)
{
	InitLocalMessage();
}

// Copy constructor
CLocalMessage::CLocalMessage(const CLocalMessage &copy)
	: CMessage(copy), mEnvelopeIndex(copy.GetEnvelopeIndex())
{
	mHasIndex = copy.mHasIndex;
	mIndexStart = copy.mIndexStart;
	mIndexLength = copy.mIndexLength;
	mIndexHeaderStart = copy.mIndexHeaderStart;
	mIndexHeaderLength = copy.mIndexHeaderLength;
	mIndexBodyStart = copy.mIndexBodyStart;
	mIndexBodyLength = copy.mIndexBodyLength;
}

// Init
void CLocalMessage::InitLocalMessage()
{
	mHasIndex = false;
	mIndexStart = 0;
	mIndexLength = 0;
	mIndexHeaderStart = 0;
	mIndexHeaderLength = 0;
	mIndexBodyStart = 0;
	mIndexBodyLength = 0;
}

// Do reconstruct recovery test
bool CLocalMessage::RecoverCompare(const CLocalMessage& comp) const
{
	// Test all the bits of this message against the one passed in to be sure its the same
	
	// First look at starts & lengths of entire message
	if (mIndexLength != comp.mIndexLength)
		return false;
	if (mIndexHeaderStart != comp.mIndexHeaderStart)
		return false;
	if (mIndexHeaderLength != comp.mIndexHeaderLength)
		return false;
	if (mIndexBodyStart != comp.mIndexBodyStart)
		return false;
	if (mIndexBodyLength != comp.mIndexBodyLength)
		return false;

	// Now do the envelope data
	if (!GetEnvelope()->RecoverCompare(*comp.GetEnvelope()))
		return false;

	// Now do the same for the attachments
	if (!GetBody() || !comp.GetBody() ||
		!static_cast<const CLocalAttachment*>(GetBody())->RecoverCompare(*static_cast<const CLocalAttachment*>(comp.GetBody())))
		return false;
	
	// Must be equal
	return true;
}

unsigned long CLocalMessage::GetIndexStart() const
{
	if (mIndexStart == cStreamAttachmentIndexStart)
		return 0;
	else
	{
		const CLocalMessage* owner = NULL;
		if (GetOwner() && ((owner = dynamic_cast<const CLocalMessage*>(GetOwner())) != NULL))
			return owner->GetIndexStart();
		else
			return mIndexStart;
	}
		
}

void CLocalMessage::SetIndexStart(unsigned long start)
{
	const CLocalMessage* owner = NULL;
	if ((start != cStreamAttachmentIndexStart) && GetOwner() && ((owner = dynamic_cast<const CLocalMessage*>(GetOwner())) != NULL))
		mIndexStart = owner->GetIndexStart();
	else
		mIndexStart = start;
}

// Stream ops
void CLocalMessage::WriteIndexToStream(std::ostream& out, long offset, ulvector* text_indices) const
{
	// If this is a sub-message and the text attachment indices only are being written, 
	// then parts_only will be true => only write text attachment indices to stream

	// Write basic index entries
	if (!text_indices)
	{
		// NB If there's an offset then the size of the top-level message must also be adjusted
		// This can happen if the unix mbox 'From ...' header changes in size e.g. when copying
		// from a POP3 INBOX to some other mailbox

		unsigned long items[5];
		items[0] = htonl(mIndexLength + (IsSubMessage() ? 0 : offset));
		items[1] = htonl(mIndexHeaderStart ? mIndexHeaderStart + offset : 0);
		items[2] = htonl(mIndexHeaderLength);
		items[3] = htonl(mIndexBodyStart ? mIndexBodyStart + offset : 0);
		items[4] = htonl(mIndexBodyLength);
		out.write(reinterpret_cast<const char*>(items), 5 * sizeof(unsigned long));
		out << cd_endl;

		// Write out envelope index
		mEnvelopeIndex.WriteIndexToStream(out, offset);
	}

	// Write body index entries
	const CLocalAttachment* local = NULL;
	if (IsFullyCached() && ((local = dynamic_cast<const CLocalAttachment*>(GetBody())) != NULL))
	{
		// Do this twice:

		// Top level message has text_indices = NULL
		if (text_indices)
			// First time calculates text indices
			local->WriteIndexToStream(out, text_indices, offset);
		else if (!IsSubMessage())
		{
			// First time calculates text indices
			ulvector new_text_indices;
			local->WriteIndexToStream(out, &new_text_indices, offset);

			// Now write indices to stream
			::WriteHost(out, new_text_indices);
		}
		
		// Now do second time and write actual attachment indices to stream
		if (!text_indices)
			local->WriteIndexToStream(out, NULL, offset);
	}
}

void CLocalMessage::ReadIndexFromStream(std::istream& in, ulvector* text_indices, unsigned long vers)
{
	// Read basic index entries
	unsigned long items[5];
	in.read(reinterpret_cast<char*>(items), 5 * sizeof(unsigned long));
	
	//mIndexStart = items[..]; <-- actually stored in index file
	mIndexLength = ntohl(items[0]);
	mIndexHeaderStart = ntohl(items[1]);
	mIndexHeaderLength = ntohl(items[2]);
	mIndexBodyStart = ntohl(items[3]);
	mIndexBodyLength = ntohl(items[4]);
	in.ignore();

	// Read envelope index
	mEnvelopeIndex.ReadIndexFromStream(in, vers);

	// Read text indices
	if (!IsCached() || !IsSubMessage())
		::ReadHost(in, text_indices);

	// Read body entries
	CLocalAttachment* local = NULL;
	if (IsFullyCached() && ((local = dynamic_cast<CLocalAttachment*>(GetBody())) != NULL))
		local->ReadIndexFromStream(in, vers);

	// Set certain internal parameters
	if (IsCached())
		SetSize(mIndexLength);

	mHasIndex = true;
}

void CLocalMessage::WriteCacheToStream(std::ostream& out) const
{
	// Write out envelope information first
	GetEnvelope()->WriteCacheToStream(out, const_cast<CEnvelope::CEnvelopeIndex&>(mEnvelopeIndex));

	::WriteHost(out, mCache->mInternalDate);
	::WriteHost(out, mCache->mInternalZone);
	out << cd_endl;

	// Now write out the body
	static_cast<const CLocalAttachment*>(GetBody())->WriteCacheToStream(out);
}

void CLocalMessage::ReadCacheFromStream(std::istream& in, unsigned long vers)
{
	// Must be cached
	CacheMessage();

	// Read in envelope
	if (!GetEnvelope())
		SetEnvelope(new CEnvelope);
	GetEnvelope()->ReadCacheFromStream(in, vers);
	::ReadHost(in, (long&)mCache->mInternalDate);
	::ReadHost(in, mCache->mInternalZone);
	in.ignore();

	// Now read in the body
	CLocalAttachment* attach = new CLocalAttachment;
	attach->ReadCacheFromStream(in, this, vers);
	SetBody(attach);
}

// Write message header to output stream
void CLocalMessage::WriteHeaderToStream(costream& stream)
{
	// Only relevant for local message
	CStreamAttachment* streama = dynamic_cast<CStreamAttachment*>(GetBody());
	if (streama)
	{
		std::istream* in = streama->GetStream();
		if ((in != NULL) && mIndexHeaderLength)
			::StreamCopy(*in, stream.Stream(), mIndexHeaderStart, mIndexHeaderLength);
	}

}

// Write message to output stream
void CLocalMessage::WriteToStream(costream& stream, bool dummy_files, CProgress* progress)
{
	// Check for no mailbox
	if (GetMbox() == NULL)
		CMessage::WriteToStream(stream, dummy_files, progress);
	else if (GetMbox()->IsFullOpen())
		GetMbox()->CopyMessage(this, &stream);
	else
		GetMbox()->CopyMessage(GetMessageNumber(), false, &stream);
}
