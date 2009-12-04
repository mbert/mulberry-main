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


//	CMessageAttachment.cp

#include "CMessageAttachment.h"

#include "CAliasAttachment.h"
#include "CAttachmentList.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CRFC822.h"
#include "CPreferences.h"
#include "CStreamFilter.h"
#include "CStreamType.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
#include <WIN_LDataStream.h>
#endif

#include <stdio.h>
#include <string.h>

// Default constructor
CMessageAttachment::CMessageAttachment(CMessage* msg, CMessage* submsg)
{
	// Init to default values
	InitMessageAttachment();

	GetContent().SetContent(eContentMessage, eContentSubRFC822);
	GetContent().SetTransferEncoding(eNoTransferEncoding);
	GetContent().SetTransferMode(eMIMEMode);

	GetContent().SetContentSize(submsg->GetSize());
	GetContent().SetMappedName(submsg->GetEnvelope()->GetSubject());
	SetName(submsg->GetEnvelope()->GetSubject());

	// Set original message
	mOriginal = msg;
	mSubMessage = submsg;

	// Make aliases of original body
	if (mSubMessage->GetBody()->IsMessage())
		AddPart(new CMessageAttachment(mOriginal, mSubMessage->GetBody()->GetMessage()));
	else
		AddPart(new CAliasAttachment(mOriginal, mSubMessage->GetBody()));
}

// Copy constructor
CMessageAttachment::CMessageAttachment(const CMessageAttachment& copy)
	: CAttachment(copy)
{
	// Init to default values
	InitMessageAttachment();

	// Original message remains the same
	mOriginal = copy.mOriginal;
	mSubMessage = copy.mSubMessage;
}

// Destructor
CMessageAttachment::~CMessageAttachment()
{
	// Clear original message
	mOriginal = NULL;
	mSubMessage = NULL;
}

// Common init
void CMessageAttachment::InitMessageAttachment()
{
	mOriginal = NULL;
	mSubMessage = NULL;
}

// Process files for sending
void CMessageAttachment::ProcessSend()
{
	// Already prepared
}

void CMessageAttachment::ProcessContent()
{
	// Already set as message/rfc822
}

// Check whether any part owned by mbox
bool CMessageAttachment::TestOwnership(const CMbox* owner) const
{
	// Test this part
	if (mOriginal && (mOriginal->GetMbox() == owner))
		return true;

	// Test any subparts
	if (mParts)
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if ((*iter)->TestOwnership(owner))
				return true;
		}

	// All done
	return false;
}

// Server contained
bool CMessageAttachment::ContainsServer(const CMboxProtocol* proto) const
{
	return (mOriginal != NULL) &&
			((mOriginal->GetMbox()->GetProtocol() == proto) ||
				(mOriginal->GetMbox()->GetMsgProtocol() == proto));
}

// Mailbox contained
bool CMessageAttachment::ContainsMailbox(const CMbox* mbox) const
{
	return (mOriginal != NULL) && (mOriginal->GetMbox() == mbox);
}

// Message contained
bool CMessageAttachment::ContainsMessage(const CMessage* msg) const
{
	return (mOriginal != NULL) && (mOriginal == msg);
}

// Write attachment to stream
void  CMessageAttachment::WriteToStream(costream& stream, unsigned long& level, bool dummy_files, CProgress* progress, CMessage* owner,
										unsigned long count, unsigned long start) const
{
	{
		// Bump up level for each new header (not the first which must remain at 0)
		level++;

		// Send content type header (multipart)
		const_cast<CMessageAttachment*>(this)->mContent.SetMultiLevel(level);
		cdstring content = CMIMESupport::GenerateContentHeader(this, dummy_files, stream.GetEndlType());
		if (!content.empty())
			stream.Stream() << content << stream.endl();

		// Send description
		cdstring description = CMIMESupport::GenerateContentDescription(this, stream.GetEndlType());
		if (!description.empty())
			stream.Stream() << description << stream.endl();
		
		// Send id
		cdstring id = CMIMESupport::GenerateContentId(this, stream.GetEndlType());
		if (!id.empty())
			stream.Stream() << id << stream.endl();
		
		// Send disposition
		cdstring disposition = CMIMESupport::GenerateContentDisposition(this, stream.GetEndlType());
		if (!disposition.empty())
			stream.Stream() << disposition << stream.endl();

		// Generate boundary between header and nody
		stream.Stream() << stream.endl();
	}

	// Only do this if cached
	if (IsNotCached())
		return;

	// Use dummy text for files?
	if (!dummy_files)
	{
		// Just get message to write itself to the stream
		mSubMessage->WriteToStream(stream, dummy_files, progress);
	}
	else
	{
		// Send attachment name
		stream.Stream() << "Message sent: \"";
		stream.Stream() << mOriginal->GetEnvelope()->GetSubject();
		stream.Stream() << "\"";
		stream.Stream() << stream.endl() << stream.endl();
	}
}
