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

#include "UNX_LFileStream.h"

#include "CLog.h"

#include <unistd.h>
#include <jDirUtil.h>
#include <JString.h>

// ---------------------------------------------------------------------------
//		 LFileStream
// ---------------------------------------------------------------------------
//	Default Contructor

LFileStream::LFileStream(): theFilePointer(-1)
{
}


// ---------------------------------------------------------------------------
//		 LFileStream(FSSpec&)
// ---------------------------------------------------------------------------
//	Contruct a FileStream from a Toolbox File System Specification

LFileStream::LFileStream(
	const char* inFileName,
	int flags) : theFilePointer(-1)
		
{
	Open(inFileName, flags);
}

// ---------------------------------------------------------------------------
//		 ~LFileStream
// ---------------------------------------------------------------------------
//	Destructor

LFileStream::~LFileStream()
{
	Close();
}


// ---------------------------------------------------------------------------
//		 SetMarker
// ---------------------------------------------------------------------------
//	Place the Read/Write Marker at an offset from a specified position
//
//	inFromWhere can be streamFrom_Start, streamFrom_End, or streamFrom_Marker

void
LFileStream::SetMarker(
	SInt32		inOffset,
	EStreamFrom	inFromWhere)
{
	int where;
	switch(inFromWhere)
	{
	case streamFrom_Start:
		where = SEEK_SET;
		break;
	case streamFrom_End:
		where = SEEK_END;
		break;
	case streamFrom_Marker:
		where = SEEK_CUR;
		break;
	}
	//Seek(inOffset, where);
	lseek(theFilePointer, inOffset, where);
}


// ---------------------------------------------------------------------------
//		 GetMarker
// ---------------------------------------------------------------------------
//	Return the Read/Write Marker position
//
//	Position is a byte offset from the start of the data fork

SInt32
LFileStream::GetMarker() const
{
	//return GetPosition();
	return lseek(theFilePointer, 0, SEEK_CUR);
}


// ---------------------------------------------------------------------------
//		 SetLength
// ---------------------------------------------------------------------------
//	Set the length, in bytes, of the data fork of a FileStream

void
LFileStream::SetLength(
	SInt32	inLength)
{
	//CFile::SetLength(inLength);
	ftruncate(theFilePointer, inLength);
	LStream::SetLength(inLength);
}


// ---------------------------------------------------------------------------
//		 GetLength
// ---------------------------------------------------------------------------
//	Return the length, in bytes, of the data fork of a FileStream

UInt32
LFileStream::GetLength() const
{
	//return CFile::GetLength();
	UInt32 length, currentPos;
	currentPos = lseek(theFilePointer, 0, SEEK_CUR);
	length = lseek(theFilePointer, 0, SEEK_END);
	lseek(theFilePointer, currentPos, SEEK_SET);
	return length;
}


// ---------------------------------------------------------------------------
//		 PutBytes
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
	//Write(inBuffer, ioByteCount);
	write(theFilePointer, inBuffer, ioByteCount);
	return noErr;
}


// ---------------------------------------------------------------------------
//		 GetBytes
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
	SInt32 amtRead;
	SInt32 totalRead = 0;
	ExceptionCode ret = noErr;
	//ioByteCount = Read(outBuffer, ioByteCount);
	do {
		amtRead = read(theFilePointer, outBuffer, ioByteCount);
		totalRead += amtRead;
	} while (totalRead < ioByteCount && amtRead > 0);
	
	ioByteCount = totalRead;
	if (amtRead == 0)
		return eofErr;
	else if (amtRead < 0)
	{
		CLOG_LOGTHROW(CantReadFromFile, 0);
		throw CantReadFromFile();
	}
	return noErr;
}

void LFileStream::Open(const char* name, int flags)
{
	if (theFilePointer != -1) {
		Close();
	}
	theFilePointer = open(name,flags, S_IRUSR | S_IWUSR);
	if (theFilePointer < 0)
	{
		CLOG_LOGTHROW(CantOpenFile, 0);
		throw CantOpenFile();
	}
	//fseek(theFilePointer, 0, SEEK_SET);
	SetSpecifier(cdstring(name));
}
void LFileStream::Close()
{
	if (theFilePointer > -1) {
		close(theFilePointer);
		theFilePointer = -1;
	}
}

void LFileStream::GetStatus(struct stat& buf) 
{
	fstat(theFilePointer, &buf);
}

void LFileStream::SetSpecifier(const char* spec) {
	mFileName = spec;
}

const cdstring& LFileStream::GetFileName() const
{	
	//I could try to remember I figured this out before, but if I did that
	//I'd have to rely on the fact that subclasses always did the right thing
	//(which would basically be to call SetSpecifier instead of setting mFileName
	//directly.  I could ensure this by making mFileName private, but existing code
	//assume that it is at most protected, which is part of hte reason I don't trust
	//it to do the right thing.
	//But I really want this method to be const, and I want it to return a cdstring&
	//instead of a cdstring to avoid copying, so mFileNamePart has to be mutable.
	JString path, filename;
	JSplitPathAndName(mFileName, &path, &filename);
	mFileNamePart = filename;
	return mFileNamePart;
}
