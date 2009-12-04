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


//	A Stream whose bytes are in a Handle block in memory

#ifndef __WIN_LMEMFILESTREAM__MULBERRY__
#define __WIN_LMEMFILESTREAM__MULBERRY__

#include "UNX_LStream.h"

#include <strstream>

class LMemFileStream : public LStream
{
public:
	LMemFileStream();
	LMemFileStream(char* inData);
	virtual ~LMemFileStream();

	virtual void SetLength(SInt32 inLength);
	virtual UInt32 GetLength() const;

	virtual ExceptionCode PutBytes(const void* inBuffer, SInt32& ioByteCount);
	virtual ExceptionCode GetBytes(void* outBuffer, SInt32& ioByteCount);

	char* DetachData();

protected:
	void*				mDataF;
	std::ostrstream		mOutStream;
	unsigned long		offset;
	unsigned long		datalen;
};


#endif
