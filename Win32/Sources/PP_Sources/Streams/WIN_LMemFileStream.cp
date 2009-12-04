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

#include <WIN_LMemFileStream.h>


// ---------------------------------------------------------------------------
//		¥ LMemFileStream
// ---------------------------------------------------------------------------
//	Default Constructor

LMemFileStream::LMemFileStream()
{
}


// ---------------------------------------------------------------------------
//		¥ LMemFileStream(Handle)
// ---------------------------------------------------------------------------
//	Construct from an existing Handle
//
//	The LMemFileStream object assumes control of the Handle

LMemFileStream::LMemFileStream(
	char*	inData)
{
	mDataF.Attach((unsigned char*) inData, ::strlen(inData));
	LStream::SetLength(::strlen(inData));	// This is dodgy - is data really c-string?
}


// ---------------------------------------------------------------------------
//		¥ ~LMemFileStream
// ---------------------------------------------------------------------------
//	Destructor

LMemFileStream::~LMemFileStream()
{
}


// ---------------------------------------------------------------------------
//		¥ SetLength
// ---------------------------------------------------------------------------
//	Set the length, in bytes, of the HandleStream

void
LMemFileStream::SetLength(
	SInt64	inLength)
{
	mDataF.SetLength(inLength);
	LStream::SetLength(inLength);
}


// ---------------------------------------------------------------------------
//		¥ GetLength
// ---------------------------------------------------------------------------
//	Return the length, in bytes, of the Stream

UInt64
LMemFileStream::GetLength() const
{
	return mDataF.GetLength();
}

// ---------------------------------------------------------------------------
//		¥ PutBytes
// ---------------------------------------------------------------------------
//	Write bytes from a buffer to a HandleStream
//
//	Returns an error code and passes back the number of bytes actually
//	written, which may be less than the number requested if an error occurred.
//
//	Grows data Handle if necessary.
//
//	Errors:
//		memFullErr		Growing Handle failed when trying to write past
//							the current end of the Stream

ExceptionCode
LMemFileStream::PutBytes(
	const void	*inBuffer,
	SInt32		&ioByteCount)
{
	mDataF.Write(inBuffer, ioByteCount);
	SetMarker(ioByteCount, streamFrom_Marker);
	return noErr;
}


// ---------------------------------------------------------------------------
//		¥ GetBytes
// ---------------------------------------------------------------------------
//	Read bytes from a HandleStream to a buffer
//
//	Returns an error code and passes back the number of bytes actually
//	read, which may be less than the number requested if an error occurred.
//
//	Errors:
//		readErr		Attempt to read past the end of the HandleStream

ExceptionCode
LMemFileStream::GetBytes(
	void	*outBuffer,
	SInt32	&ioByteCount)
{
	mDataF.Read(outBuffer, ioByteCount);
	SetMarker(ioByteCount, streamFrom_Marker);

	return noErr;
}


// ---------------------------------------------------------------------------
//		¥ DetachDataHandle
// ---------------------------------------------------------------------------
//	Dissociate the data Handle from a HandleStream.
//
//	Creates a new, empty data Handle and passes back the existing Handle.
//	Caller assumes ownership of the Handle.

char*
LMemFileStream::DetachData()
{
	char* detached = (char*) mDataF.Detach();

	SetMarker(0, streamFrom_Start);
	LStream::SetLength(0);

	return detached;
}
