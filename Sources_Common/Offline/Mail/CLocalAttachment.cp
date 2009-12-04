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

#include "CLocalAttachment.h"

#include "CAttachmentList.h"
#include "CGeneralException.h"
#include "CLocalMessage.h"
#include "CStreamUtils.h"

#include "cdiomanip.h"

#include <istream>

// Constructed from owning mbox only
CLocalAttachment::CLocalAttachment()
	: CAttachment()
{
	InitLocalAttachment();
}

// Copy constructor
CLocalAttachment::CLocalAttachment(const CLocalAttachment &copy)
	: CAttachment(copy)
{
	mIndexStart = copy.mIndexStart;
	mIndexLength = copy.mIndexLength;
	mIndexHeaderStart = copy.mIndexHeaderStart;
	mIndexHeaderLength = copy.mIndexHeaderLength;
	mIndexBodyStart = copy.mIndexBodyStart;
	mIndexBodyLength = copy.mIndexBodyLength;
}

// Init
void CLocalAttachment::InitLocalAttachment()
{
	mIndexStart = 0;
	mIndexLength = 0;
	mIndexHeaderStart = 0;
	mIndexHeaderLength = 0;
	mIndexBodyStart = 0;
	mIndexBodyLength = 0;
}

// Do reconstruct recovery test
bool CLocalAttachment::RecoverCompare(const CLocalAttachment& comp) const
{
	// MIME data must be the same
	if (GetContent().RecoverCompare(comp.GetContent()))
		return false;
	
	// Must have the same number of parts
	if (((GetParts() != NULL) ^ (comp.GetParts() != NULL)) ||
		GetParts() && (GetParts()->size() != comp.GetParts()->size()))
		return false;

	// Do the same test for each child part
	if (GetParts())
	{
		CAttachmentList::const_iterator iter1 = GetParts()->begin();
		CAttachmentList::const_iterator iter2 = comp.GetParts()->begin();
		while(iter1 != GetParts()->end())
		{
			if (!static_cast<const CLocalAttachment*>(*iter1++)->RecoverCompare(*static_cast<const CLocalAttachment*>(*iter2++)))
				return false;
		}
	}
	
	// Must match at this point
	return true;
}

#pragma mark ____________________________Stream ops

void CLocalAttachment::WriteIndexToStream(std::ostream& out, ulvector* text, long offset) const
{
	// IMPORTANT: This will be called twice
	// Once with text non-NULL, in which case only text indices calculation should be done
	// and once with text NULL, in which case output to stream should take place

	// Write basic indexes
	unsigned long temp = 0;
	if (!text)
	{
		unsigned long items[6];
		items[0] = htonl(mIndexStart ? mIndexStart + offset : 0);
		items[1] = htonl(mIndexLength);
		items[2] = htonl(mIndexHeaderStart ? mIndexHeaderStart + offset : 0);
		items[3] = htonl(mIndexHeaderLength);
		items[4] = htonl(mIndexBodyStart ? mIndexBodyStart + offset : 0);
		items[5] = htonl(mIndexBodyLength);
		out.write(reinterpret_cast<const char*>(items), 6 * sizeof(unsigned long));
	}

	// Store index if this is a searchable text part
	if (text && IsText())
	{
		text->push_back(mIndexBodyStart ? mIndexBodyStart + offset : 0);
		text->push_back(mIndexBodyLength);
		text->push_back(GetContent().GetTransferEncoding());
	}

	// Multipart => write out each part
	if (IsMultipart() && mParts && mParts->size())
	{
		// Write number of parts
		if (!text)
		{
			::WriteHost(out, mParts->size());
			out << cd_endl;
		}

		// Write out each part
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			static_cast<CLocalAttachment*>(*iter)->WriteIndexToStream(out, text, offset);
	}
	else if (IsMessage())
	{
		// Write out message flag
		if (!text)
		{
			::WriteHost(out, 0xFFFFFFFF);
			out << cd_endl;
		}

		// If text indices only, make sure only sub-message text parts get written out
		static_cast<CLocalMessage*>(mMessage)->WriteIndexToStream(out, offset, text);
	}
	else
	{
		if (!text)
		{
			::WriteHost(out, 0L);
			out << cd_endl;
		}
	}
}

void CLocalAttachment::ReadIndexFromStream(std::istream& in, unsigned long vers)
{
	// Read basic indexes
	unsigned long items[7];
	in.read(reinterpret_cast<char*>(items), 7 * sizeof(unsigned long));
	in.ignore();

	mIndexStart = ntohl(items[0]);
	mIndexLength = ntohl(items[1]);
	mIndexHeaderStart = ntohl(items[2]);
	mIndexHeaderLength = ntohl(items[3]);
	mIndexBodyStart = ntohl(items[4]);
	mIndexBodyLength = ntohl(items[5]);

	unsigned long count = ntohl(items[6]);

	// Multipart => write out each part
	if (IsMultipart() && mParts && mParts->size())
	{
		// Check number of parts
		if (count != mParts->size())
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Read each part
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			static_cast<CLocalAttachment*>(*iter)->ReadIndexFromStream(in, vers);
	}
	else if (IsMessage())
	{
		// Check message flag and verify
		if (count != 0xFFFFFFFF)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		static_cast<CLocalMessage*>(mMessage)->ReadIndexFromStream(in, NULL, vers);
	}
	else
	{
		// Check flag and verify
		if (count != 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
	}

	// Set certain internal parameters
	GetContent().SetContentSize(mIndexBodyLength);
}

void CLocalAttachment::WriteCacheToStream(std::ostream& out) const
{
	// Write out this part
	GetContent().WriteCacheToStream(out);

	// Write out any subparts
	if (IsMultipart() && mParts && mParts->size())
	{
		// Write number of parts
		::WriteHost(out, mParts->size());
		out << cd_endl;

		// Write out each part
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
			static_cast<CLocalAttachment*>(*iter)->WriteCacheToStream(out);
	}
	else if (IsMessage())
	{
		// Write out message flag
		::WriteHost(out, 0xFFFFFFFF);
		out << cd_endl;

		static_cast<CLocalMessage*>(mMessage)->WriteCacheToStream(out);
	}
	else
	{
		::WriteHost(out, 0L);
		out << cd_endl;
	}
}

void CLocalAttachment::ReadCacheFromStream(std::istream& in, CLocalMessage* owner, unsigned long vers)
{
	// Read basic indexes
	GetContent().ReadCacheFromStream(in, vers);

	// Multipart => write out each part
	if (IsMultipart())
	{
		// Read number of parts and verify
		unsigned long parts = 0;
		::ReadHost(in, parts);
		in.ignore();

		// Create and read each part
		for(unsigned long i = 0; i < parts; i++)
		{
			CLocalAttachment* attach = new CLocalAttachment;
			attach->ReadCacheFromStream(in, owner, vers);
			AddPart(attach);
		}
	}
	else if (IsMessage())
	{
		// Read message flag and verify
		unsigned long msg = 0;
		::ReadHost(in, msg);
		in.ignore();
		if (msg != 0xFFFFFFFF)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Create and read message
		CLocalMessage* lmsg = new CLocalMessage(owner);
		lmsg->ReadCacheFromStream(in, vers);
		SetMessage(lmsg);
	}
	else
	{
		// Read flag and verify
		unsigned long nums = 0;
		::ReadHost(in, nums);
		in.ignore();
		if (nums != 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
	}
}
