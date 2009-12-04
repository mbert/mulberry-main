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


//	CAliasAttachment.cp

#include "CAliasAttachment.h"

#include "CAttachmentList.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageAttachment.h"
#include "CPreferences.h"

#if __dest_os == __win32_os
#include <WIN_LDataStream.h>
#endif

#include <stdio.h>
#include <string.h>

// Default constructor
CAliasAttachment::CAliasAttachment(CMessage* owner, const CAttachment* original)
{
	// Init to default values
	InitAliasAttachment();

	// Set original message & attachment
	mOwner = owner;
	mOriginal = (CAttachment*) original;

	// Copy bits from original attachment
	mContent = mOriginal->GetContent();
	mName = mOriginal->GetName();

	// Copy submesssage first
	if (mOriginal->GetMessage())
		SetMessage(new CMessage(*mOriginal->GetMessage()));

	// Duplicate all parts and add to this one (if not message)
	else if (mOriginal->GetParts())
	{
		// For all entries
		for(CAttachmentList::iterator iter = mOriginal->GetParts()->begin(); iter != mOriginal->GetParts()->end(); iter++)
		{
			// Make aliases of original items
			if ((*iter)->IsMessage())
				AddPart(new CMessageAttachment(mOwner, (*iter)->GetMessage()));
			else
				AddPart(new CAliasAttachment(mOwner, *iter));
		}
	}
}

// Copy constructor
CAliasAttachment::CAliasAttachment(const CAliasAttachment& copy)
	: CAttachment(copy)
{
	// Init to default values
	InitAliasAttachment();

	// Original message & attachment remains the same
	mOwner = copy.mOwner;
	mOriginal = copy.mOriginal;
}

// Destructor
CAliasAttachment::~CAliasAttachment()
{
	mOriginal = NULL;
	mOwner = NULL;
}

// Common init
void CAliasAttachment::InitAliasAttachment()
{
}

// Process files for sending
void CAliasAttachment::ProcessSend()
{
	// Already prepared
}

void CAliasAttachment::ProcessContent()
{
	// Already set
}

// Check whether any part owned by mbox
bool CAliasAttachment::TestOwnership(const CMbox* owner) const
{
	// Test this part
	if (mOwner && (mOwner->GetMbox() == owner))
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
bool CAliasAttachment::ContainsServer(const CMboxProtocol* proto) const
{
	return (mOwner != NULL) &&
			((mOwner->GetMbox()->GetProtocol() == proto) ||
				(mOwner->GetMbox()->GetMsgProtocol() == proto));
}

// Mailbox contained
bool CAliasAttachment::ContainsMailbox(const CMbox* mbox) const
{
	return (mOwner != NULL) && (mOwner->GetMbox() == mbox);
}

// Message contained
bool CAliasAttachment::ContainsMessage(const CMessage* msg) const
{
	return (mOwner != NULL) && (mOwner == msg);
}

// Write original attachment to stream
void CAliasAttachment::WriteDataToStream(costream& stream, bool dummy_files, CProgress* progress, CMessage* owner,
											unsigned long count, unsigned long start) const
{
	// Just use original message and attachment
	mOriginal->WriteDataToStream(stream, dummy_files, progress, mOwner, count, start);
}
