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


//	A File which uses a Stream to access its data fork

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <WIN_LFileStream.h>


// ---------------------------------------------------------------------------
//		¥ LFileStream
// ---------------------------------------------------------------------------
//	Default Contructor

LFileStream::LFileStream()
{
}


// ---------------------------------------------------------------------------
//		¥ LFileStream(FSSpec&)
// ---------------------------------------------------------------------------
//	Contruct a FileStream from a Toolbox File System Specification

LFileStream::LFileStream(
	LPCTSTR	inFileName,
	UINT flags)
		: CFile(inFileName, flags)
{
}

// ---------------------------------------------------------------------------
//		¥ ~LFileStream
// ---------------------------------------------------------------------------
//	Destructor

LFileStream::~LFileStream()
{
}


// ---------------------------------------------------------------------------
//		¥ SetMarker
// ---------------------------------------------------------------------------
//	Place the Read/Write Marker at an offset from a specified position
//
//	inFromWhere can be streamFrom_Start, streamFrom_End, or streamFrom_Marker

void
LFileStream::SetMarker(
	SInt64		inOffset,
	EStreamFrom	inFromWhere)
{
	UINT where;
	switch(inFromWhere)
	{
	case streamFrom_Start:
		where = CFile::begin;
		break;
	case streamFrom_End:
		where = CFile::end;
		break;
	case streamFrom_Marker:
		where = CFile::current;
		break;
	}
	Seek(inOffset, where);
}


// ---------------------------------------------------------------------------
//		¥ GetMarker
// ---------------------------------------------------------------------------
//	Return the Read/Write Marker position
//
//	Position is a byte offset from the start of the data fork

SInt64
LFileStream::GetMarker() const
{
	return GetPosition();
}


// ---------------------------------------------------------------------------
//		¥ SetLength
// ---------------------------------------------------------------------------
//	Set the length, in bytes, of the data fork of a FileStream

void
LFileStream::SetLength(
	SInt64	inLength)
{
	CFile::SetLength(inLength);
	LStream::SetLength(inLength);
}


// ---------------------------------------------------------------------------
//		¥ GetLength
// ---------------------------------------------------------------------------
//	Return the length, in bytes, of the data fork of a FileStream

UInt64
LFileStream::GetLength() const
{
	return CFile::GetLength();
}


// ---------------------------------------------------------------------------
//		¥ PutBytes
// ---------------------------------------------------------------------------
//	Write bytes from a buffer to a DataStream
//
//	Returns an error code and passes back the number of bytes actually
//	written, which may be less than the number requested if an error occurred.

ExceptionCode
LFileStream::PutBytes(
	const void	*inBuffer,
	SInt32		&ioByteCount)
{
	Write(inBuffer, ioByteCount);
	return noErr;
}


// ---------------------------------------------------------------------------
//		¥ GetBytes
// ---------------------------------------------------------------------------
//	Read bytes from a DataStream to a buffer
//
//	Returns an error code and passes back the number of bytes actually
//	read, which may be less than the number requested if an error occurred.

ExceptionCode
LFileStream::GetBytes(
	void	*outBuffer,
	SInt32	&ioByteCount)
{
	ioByteCount = Read(outBuffer, ioByteCount);
	return (ioByteCount ? noErr : eofErr);
}