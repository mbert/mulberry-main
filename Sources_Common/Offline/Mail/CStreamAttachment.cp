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


// Source for Local Message class

#include "CStreamAttachment.h"

#include "CMessage.h"
#include "CProgress.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStreamUtils.h"

// Constructed from owning mbox only
CStreamAttachment::CStreamAttachment()
	: CLocalAttachment()
{
	InitStreamAttachment();
}

// Copy constructor
CStreamAttachment::CStreamAttachment(const CStreamAttachment &copy)
	: CLocalAttachment(copy)
{
	InitStreamAttachment();
}

CStreamAttachment::~CStreamAttachment()
{
	delete mStream;
	mStream = NULL;
	delete mStreamData;
	mStreamData = NULL;

	// File stream must be deleted
	if (mStreamName.length())
	{
		// Must not fail
		try
		{
			::remove_utf8(mStreamName);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must not fail
		}
	}
}

// Init
void CStreamAttachment::InitStreamAttachment()
{
	mStream = NULL;
	mStreamData = NULL;
}

#pragma mark ____________________________Stream ops

std::istream* CStreamAttachment::GetStream()
{
	if (mParent && dynamic_cast<CStreamAttachment*>(mParent))
		return static_cast<CStreamAttachment*>(mParent)->GetStream();
	else
		return mStream;
}

const char*	CStreamAttachment::ReadPart(CMessage* owner)
{
	// Just get owner to do it
	return CAttachment::ReadPart(owner);

#if 0
	// Read from stream into internal buffer
	std::istream* stream = GetStream();
	if (stream && mIndexBodyLength)
	{
		// Create data space
		std::auto_ptr<char> data(new char[mIndexBodyLength + 1]);
		
		// Read stream into buffer
		stream->seekg(mIndexBodyStart);
		stream->read(data.get(), mIndexBodyLength);
		data.get()[mIndexBodyLength] = 0;
		SetData(data.release());

		// Maybe EOF so clear
		if (stream->eof())
			stream->clear();
	}
	
	return GetData();
#endif
}

// Read into stream
void CStreamAttachment::ReadAttachment(CMessage* msg, LStream* aStream, bool peek)
{
	// Read from stream into internal buffer
	std::istream* stream = GetStream();
	if (stream && mIndexBodyLength)
		::StreamCopy(*stream, *aStream, mIndexBodyStart, mIndexBodyLength);
}

// Copy into stream
void CStreamAttachment::ReadAttachment(CMessage* msg, costream* aStream, bool peek, unsigned long count, unsigned long start) const
{
	// Read from stream into internal buffer
	std::istream* stream = const_cast<CStreamAttachment*>(this)->GetStream();
	if (stream && mIndexBodyLength)
	{
		// Check for filter requirements
		if (aStream->IsLocalType())
			::StreamCopy(*stream, aStream->Stream(), mIndexBodyStart + start - 1, count ? count : mIndexBodyLength);
		else
		{
			CStreamFilter filter(new crlf_filterbuf(aStream->GetEndlType()), aStream->GetStream());
			::StreamCopy(*stream, filter, mIndexBodyStart + start - 1, count ? count : mIndexBodyLength);
		}
	}
}

// Write attachment to stream
void  CStreamAttachment::WriteToStream(costream& stream, unsigned long& level, bool dummy_files, CProgress* progress, CMessage* owner,
										unsigned long count, unsigned long start) const
{
	// Bump up level for each new header
	level++;

	// Just output the entire stream to preserve its content completely
	std::istream* astream = const_cast<CStreamAttachment*>(this)->GetStream();
	if (astream && mIndexBodyLength)
	{
		// Check for filter requirements
		if (stream.IsLocalType())
			::StreamCopy(*astream, stream.Stream(), mIndexStart + start - 1, count ? count : mIndexLength);
		else
		{
			CStreamFilter filter(new crlf_filterbuf(stream.GetEndlType()), stream.GetStream());
			::StreamCopy(*astream, filter, mIndexStart + start - 1, count ? count : mIndexLength);
		}
	}
}

// Write header to stream
void CStreamAttachment::WriteHeaderToStream(std::ostream& stream) const
{
	// Just output the entire stream to preserve its content completely
	std::istream* astream = const_cast<CStreamAttachment*>(this)->GetStream();
	if (astream && mIndexBodyLength)
		::StreamCopy(*astream, stream, mIndexHeaderStart, mIndexHeaderLength);
}

// Write data to stream
void CStreamAttachment::WriteDataToStream(costream& stream, bool dummy_files, CProgress* progress, CMessage* owner,
												unsigned long count, unsigned long start) const
{
	// Bump progress counter
	if (progress)
		progress->BumpCount();

	// Only do this if cached
	if (IsNotCached())
		return;

	// If owned message read from message
	if (owner)
	{
		// Bump up to root message
		while (owner->IsSubMessage())
			owner = owner->GetOwner();

		// Tell mbox to read this part
		ReadAttachment(owner, &stream, true, count, start);
	}

	// Read from stream into internal buffer
	std::istream* astream = const_cast<CStreamAttachment*>(this)->GetStream();
	if (astream && mIndexBodyLength)
	{
		// Check for filter requirements
		if (stream.IsLocalType())
			::StreamCopy(*astream, stream.Stream(), mIndexBodyStart + start - 1, count ? count : mIndexBodyLength);
		else
		{
			CStreamFilter filter(new crlf_filterbuf(stream.GetEndlType()), stream.GetStream());
			::StreamCopy(*astream, filter, mIndexBodyStart + start - 1, count ? count : mIndexBodyLength);
		}
	}
}

