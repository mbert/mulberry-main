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


//	A Stream whose bytes are in block of memory defined by a pointer to
//	the first byte and a byte count

#ifndef __UNX_LDATASTREAM__MULBERRY__
#define __UNX_LDATASTREAM__MULBERRY__

#include "UNX_LStream.h"


class	LDataStream : public LStream {
public:
					LDataStream();
					LDataStream(
							void		*inBuffer,
							SInt32		inLength);
	virtual			~LDataStream();

	virtual void	SetBuffer(
							void		*inBuffer,
							SInt32		inLength)
					{
						mBuffer = inBuffer;
						SetLength(inLength);
					}
					
	virtual void*	GetBuffer()			{ return mBuffer; }
	
	virtual ExceptionCode	PutBytes(
								const void		*inBuffer,
								SInt32			&ioByteCount);
	virtual ExceptionCode	GetBytes(
								void			*outBuffer,
								SInt32			&ioByteCount);

protected:
	void			*mBuffer;
};	

#endif
