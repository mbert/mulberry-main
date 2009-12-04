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


// Header for Local Attachment class

#ifndef __CLOCALATTACHMENT__MULBERRY__
#define __CLOCALATTACHMENT__MULBERRY__

#include "CAttachment.h"

#include "templs.h"

// Classes

class CLocalMessage;

class CLocalAttachment : public CAttachment
{
public:
	CLocalAttachment();									// Constructed from owning mbox only
	CLocalAttachment(const CLocalAttachment &copy);		// Copy constructor
	~CLocalAttachment() {}

	bool RecoverCompare(const CLocalAttachment& comp) const;	// Do reconstruct recovery test

	unsigned long GetIndexStart() const
		{ return mIndexStart; }
	void SetIndexStart(unsigned long start)
		{ mIndexStart = start; }
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

	void WriteIndexToStream(std::ostream& out, ulvector* text, long offset = 0) const;
	void ReadIndexFromStream(std::istream& in, unsigned long vers);
	void WriteCacheToStream(std::ostream& out) const;
	void ReadCacheFromStream(std::istream& in, CLocalMessage* owner, unsigned long vers);

protected:
	unsigned long	mIndexStart;
	unsigned long	mIndexLength;
	unsigned long	mIndexHeaderStart;
	unsigned long	mIndexHeaderLength;
	unsigned long	mIndexBodyStart;
	unsigned long	mIndexBodyLength;

private:
	void	InitLocalAttachment();

};

#endif
