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

#include "UNX_LDataStream.h"

#include <stddef.h>
#include <memory.h>

// ---------------------------------------------------------------------------
//		 LDataStream
// ---------------------------------------------------------------------------
//	Default Constructor

LDataStream::LDataStream()
{
	mBuffer = nil;					// Buffer is undefined
}


// ---------------------------------------------------------------------------
//		 LDataStream(void*, SInt32)
// ---------------------------------------------------------------------------
//	Construct from a pointer and a byte count
//
//	inBuffer points to the first byte of the Stream, which is inLength
//	bytes long

LDataStream::LDataStream(
	void	*inBuffer,
	SInt32	inLength)
{
	SetBuffer(inBuffer, inLength);
}


// ---------------------------------------------------------------------------
//		 ~LDataStream
// ---------------------------------------------------------------------------
//	Destructor

LDataStream::~LDataStream()
{
}


// ---------------------------------------------------------------------------
//		 PutBytes
// ---------------------------------------------------------------------------
//	Write bytes from a buffer to a DataStream
//
//	Returns an error code and passes back the number of bytes actually
//	written, which may be less than the number requested if an error occurred.
//
//	Errors:
//		writeErr		Attempt to write past end of DataStream

ExceptionCode
LDataStream::PutBytes(
	const void	*inBuffer,
	SInt32		&ioByteCount)
{
	ExceptionCode	err = noErr;

	if (GetMarker() + UInt32(ioByteCount) > GetLength()) {
		ioByteCount = GetLength() - GetMarker();
		err = writErr;
	}

	::memmove( (unsigned char*)(mBuffer) + GetMarker(), inBuffer, ioByteCount);
	SetMarker(ioByteCount, streamFrom_Marker);

	return err;
}


// ---------------------------------------------------------------------------
//		 GetBytes
// ---------------------------------------------------------------------------
//	Read bytes from a DataStream to a buffer
//
//	Returns an error code and passes back the number of bytes actually
//	read, which may be less than the number requested if an error occurred.
//
//	Errors:
//		readErr		Attempt to read past the end of the DataStream

ExceptionCode
LDataStream::GetBytes(
	void	*outBuffer,
	SInt32	&ioByteCount)
{
	ExceptionCode	err = noErr;
									// Upper bound is number of bytes from
									//   marker to end
	if (GetMarker() + UInt32(ioByteCount) > GetLength()) {
		ioByteCount = GetLength() - GetMarker();
		err = readErr;
	}

	::memmove(outBuffer, (unsigned char*)(mBuffer) + GetMarker(), ioByteCount);
	SetMarker(ioByteCount, streamFrom_Marker);

	return err;
}
