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


// Header for Local Message class

#ifndef __CLOCALMESSAGE__MULBERRY__
#define __CLOCALMESSAGE__MULBERRY__

#include "CMessage.h"
#include "CEnvelope.h"

#include "templs.h"

const unsigned long cStreamAttachmentIndexStart = 0xFFFFFFFF;

// Classes

class CLocalMessage : public CMessage
{
public:
	CLocalMessage(CMbox* owner);					// Constructed from owning mbox only
	CLocalMessage(CMessage* owner);					// Constructed from owning mbox only
	CLocalMessage(const CLocalMessage &copy);		// Copy constructor
	virtual ~CLocalMessage() {}

	bool RecoverCompare(const CLocalMessage& comp) const;	// Do reconstruct recovery test

	bool HasIndex() const
		{ return mHasIndex;}
	void RefreshIndex()
		{ mHasIndex = false;}

	unsigned long GetIndexStart() const;
	void SetIndexStart(unsigned long start);
	unsigned long GetIndexLength() const
		{ return mIndexLength; }
	void SetIndexLength(unsigned long length)
		{ mIndexLength = length; }
	unsigned long GetIndexHeaderStart() const
		{ return mIndexHeaderStart; }
	void SetIndexHeaderStart(unsigned long start)
		{ mIndexHeaderStart = start; }
	unsigned long GetIndexHeaderLength() const
		{ return mIndexHeaderLength; }
	void SetIndexHeaderLength(unsigned long length)
		{ mIndexHeaderLength = length; }
	unsigned long GetIndexBodyStart() const
		{ return mIndexBodyStart; }
	void SetIndexBodyStart(unsigned long start)
		{ mIndexBodyStart = start; }
	unsigned long GetIndexBodyLength() const
		{ return mIndexBodyLength; }
	void SetIndexBodyLength(unsigned long length)
		{ mIndexBodyLength = length; }

	CEnvelope::CEnvelopeIndex& GetEnvelopeIndex()
		{ return mEnvelopeIndex; }
	const CEnvelope::CEnvelopeIndex& GetEnvelopeIndex() const
		{ return mEnvelopeIndex; }

	void WriteIndexToStream(std::ostream& out, long offset = 0, ulvector* text_indices = NULL) const;
	void ReadIndexFromStream(std::istream& in, ulvector* text_indices, unsigned long vers);
	void WriteCacheToStream(std::ostream& out) const;
	void ReadCacheFromStream(std::istream& in, unsigned long vers);

	virtual void	WriteHeaderToStream(costream& stream);		// Write message header to output stream
	virtual void	WriteToStream(costream& stream,
							bool dummy_files,
							CProgress* progress);				// Write message to output stream

protected:
	bool						mHasIndex;
	unsigned long				mIndexStart;
	unsigned long				mIndexLength;
	unsigned long				mIndexHeaderStart;
	unsigned long				mIndexHeaderLength;
	unsigned long				mIndexBodyStart;
	unsigned long				mIndexBodyLength;
	CEnvelope::CEnvelopeIndex	mEnvelopeIndex;

private:
	void	InitLocalMessage();

};

#endif
