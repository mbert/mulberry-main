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


// Header for Stream Attachment class

#ifndef __CSTREAMATTACHMENT__MULBERRY__
#define __CSTREAMATTACHMENT__MULBERRY__

#include "CLocalAttachment.h"

#include <istream>

// Classes

class CStreamAttachment : public CLocalAttachment
{
public:
	CStreamAttachment();									// Constructed from owning mbox only
	CStreamAttachment(const CStreamAttachment &copy);		// Copy constructor
	~CStreamAttachment();

	void SetStream(std::istream* stream, const char* data, const cdstring& name)
		{ mStream = stream; mStreamData = data; mStreamName = name; }
	std::istream* GetStream();

	virtual const char*	ReadPart(CMessage* owner);

	virtual void		WriteHeaderToStream(std::ostream& stream) const;

protected:
	std::istream* mStream;
	const char* mStreamData;
	cdstring mStreamName;

	virtual void	ReadAttachment(CMessage* msg,
									LStream* aStream,
									bool peek = false);					// Read into stream
	virtual void	ReadAttachment(CMessage* msg,						// Copy into stream
							costream* aStream,
							bool peek = false,
							unsigned long count = 0,
							unsigned long start = 1) const;

	virtual void		WriteToStream(costream& stream,
										unsigned long& level,
										bool dummy_files,
										CProgress* progress,
										CMessage* owner = NULL,
										unsigned long count = 0,
										unsigned long start = 1) const;
	virtual void		WriteDataToStream(costream& stream,
											bool dummy_files,
											CProgress* progress,
											CMessage* owner = NULL,
											unsigned long count = 0,
											unsigned long start = 1) const;

private:
	void	InitStreamAttachment();

};

#endif
