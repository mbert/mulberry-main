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


//	Abstract class for reading/writing an ordered sequence of bytes

#include <WIN_LStream.h>
//#include <UMemoryMgr.h>

#include "CGeneralException.h"
#include "CLog.h"

const	SInt32	length_NilBlock = -1;

#pragma mark === Construction & Destruction

// ---------------------------------------------------------------------------
//		¥ LStream
// ---------------------------------------------------------------------------
//	Default Constructor

LStream::LStream()
{
#ifndef qNeoAccess
	mMarker = 0;
	mLength = 0;
#endif
}


// ---------------------------------------------------------------------------
//		¥ ~LStream
// ---------------------------------------------------------------------------
//	Destructor

LStream::~LStream()
{
}

#pragma mark === Accessors ===

// ---------------------------------------------------------------------------
//		¥ SetMarker
// ---------------------------------------------------------------------------
//	Place the Read/Write Marker at an offset from a specified position
//
//	inFromWhere can be streamFrom_Start, streamFrom_End, or streamFrom_Marker

void
LStream::SetMarker(
	SInt64		inOffset,
	EStreamFrom	inFromWhere)
{
#ifdef qNeoAccess
	SInt32	newMarker = getMark();
#else
	SInt64	newMarker = mMarker;
#endif

	switch (inFromWhere) {

		case streamFrom_Start:
			newMarker = inOffset;
			break;

		case streamFrom_End:
			newMarker = GetLength() - inOffset;
			break;

		case streamFrom_Marker:
			newMarker += inOffset;
			break;
	}

	if (newMarker < 0) {				// marker must be between 0 and
		newMarker = 0;					// Length, inclusive
	} else if (newMarker > GetLength()) {
		newMarker = GetLength();
	}

#ifdef qNeoAccess
	CNeoStream::setMark(newMarker);
#else
	mMarker = newMarker;
#endif
}


// ---------------------------------------------------------------------------
//		¥ GetMarker
// ---------------------------------------------------------------------------
//	Return the Read/Write Marker position
//
//	Position is a byte offset from the start of the Stream

SInt64
LStream::GetMarker() const
{
#ifdef qNeoAccess
	return CNeoStream::getMark();
#else
	return mMarker;
#endif
}


// ---------------------------------------------------------------------------
//		¥ SetLength
// ---------------------------------------------------------------------------
//	Set the length, in bytes, of the Stream

void
LStream::SetLength(
	SInt64	inLength)
{
	SInt64	oldLength = GetLength();

#ifdef qNeoAccess
	CNeoStream::setLength(inLength);
#else
	mLength = inLength;
#endif
										// If making Stream shorter, call
										//   SetMarker to make sure that
	if (oldLength < inLength) {			//   marker is not past the end
		SetMarker(GetMarker(), streamFrom_Start);
	}
}


// ---------------------------------------------------------------------------
//		¥ GetLength
// ---------------------------------------------------------------------------
//	Return the length, in bytes, of the Stream

UInt64
LStream::GetLength() const
{
#ifdef qNeoAccess
	return CNeoStream::getLength();
#else
	return mLength;
#endif
}

#pragma mark === Low-Level I/O ===

// ---------------------------------------------------------------------------
//		¥ PutBytes
// ---------------------------------------------------------------------------
//	Write bytes from a buffer to a Stream
//
//	Returns an error code and passes back the number of bytes actually
//	written, which may be less than the number requested if an error occurred.
//
//	Subclasses must override this function to support writing.
//
//	NOTE: You should not throw an Exception out of this function.

ExceptionCode
LStream::PutBytes(
	const void*	/* inBuffer */,
	SInt32		&ioByteCount)
{
	ioByteCount = 0;
	return -1;
}


// ---------------------------------------------------------------------------
//		¥ WriteBlock
// ---------------------------------------------------------------------------
//	Write data, specified by a pointer and byte count, to a Stream

void
LStream::WriteBlock(
	const void	*inBuffer,
	SInt32		inByteCount)
{
	ExceptionCode ex =  PutBytes(inBuffer, inByteCount);
	if (ex)
	{
		CLOG_LOGTHROW(CGeneralException, ex);
		throw CGeneralException(ex);
	}
}


// ---------------------------------------------------------------------------
//		¥ GetBytes
// ---------------------------------------------------------------------------
//	Read bytes from a Stream to a buffer
//
//	Returns an error code and passes back the number of bytes actually
//	read, which may be less than the number requested if an error occurred.
//
//	Subclasses must override this function to support reading.
//
//	NOTE: You should not throw an Exception out of this function.

ExceptionCode
LStream::GetBytes(
	void*	/* outBuffer */,
	SInt32	&ioByteCount)
{
	ioByteCount = 0;
	return -1;
}


// ---------------------------------------------------------------------------
//		¥ ReadBlock
// ---------------------------------------------------------------------------
//	Read data from a Stream to a buffer

void
LStream::ReadBlock(
	void	*outBuffer,
	SInt32	inByteCount)
{
	ExceptionCode ex = GetBytes(outBuffer, inByteCount);
	if (ex)
	{
		CLOG_LOGTHROW(CGeneralException, ex);
		throw CGeneralException(ex);
	}
}


// ---------------------------------------------------------------------------
//		¥ PeekData
// ---------------------------------------------------------------------------
//	Read data from a Stream to a buffer, without moving the Marker
//
//	Return the number of bytes actually read, which may be less than the
//	number requested if an error occurred

SInt32
LStream::PeekData(
	void	*outBuffer,
	SInt32	inByteCount)
{
	SInt32	currentMarker = GetMarker();

	SInt32	bytesToPeek = inByteCount;
	GetBytes(outBuffer, bytesToPeek);

	SetMarker(currentMarker, streamFrom_Start);

	return bytesToPeek;
}
