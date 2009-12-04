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


// Source for CFullFileStream class

#include "CFullFileStream.h"

// __________________________________________________________________________________________________
// C L A S S __ C F U L L F I L E S T R E A M
// __________________________________________________________________________________________________

CFullFileStream::CFullFileStream()
{
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
CFullFileStream::CFullFileStream(const PPx::FSObject &inFileSpec) :
	LFileStream(inFileSpec)
#elif __dest_os == __win32_os
CFullFileStream::CFullFileStream(LPCTSTR inFileName, UINT flags) :
	LFileStream(inFileName,flags)
#elif __dest_os == __linux_os
	CFullFileStream::CFullFileStream(const char* inFileName, UInt32 flags) :
	LFileStream(inFileName,flags)
#endif
{
}

CFullFileStream::~CFullFileStream()
{
	// Must do this here as ~LFile calls its own version: LFile::CloseResourceFork
	CloseResourceFork();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const short refNum_Undefined	= -1;

// Open for block level access
SInt16 CFullFileStream::OpenResourceFork(SInt16 inPrivileges)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	return LFileStream::OpenResourceFork(inPrivileges);
#else
	return 0;
#endif
}

// Close after block level access
void CFullFileStream::CloseResourceFork(void)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	LFileStream::CloseResourceFork();
#endif
}

void CFullFileStream::SetRsrcMarker(SInt32 inOffset, EStreamFrom inFromWhere)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// LStream uses positive offsets from the end to mean move
		// backwards. But, SetFPos always uses negative offsets for
		// moving backwards. To allow people to use LFileStream from
		// a LStream pointer, we need to support the PP convention
		// by switching positive offsets from the end to the negative
		// value so that SetFPos does what was intended.

	if ((inFromWhere == streamFrom_End) && (inOffset > 0)) {
		inOffset = -inOffset;
	}

	OSStatus err = ::FSSetForkPosition(GetResourceForkRefNum(), fsFromStart, inOffset);
	ThrowIfOSErr_(err);
#endif
}


ExceptionCode CFullFileStream::GetResourceBytes(void* outBuffer, SInt32& inNumBytes)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	ByteCount bytesRead;
	OSStatus err = ::FSReadFork(GetResourceForkRefNum(), fsFromMark, 0,
								inNumBytes, outBuffer, &bytesRead);

	inNumBytes = bytesRead;

	return err;
#else
	// Windows files do not have any resource bytes
	inNumBytes = 0;
	return eofErr;
#endif
}

ExceptionCode CFullFileStream::PutResourceBytes(const void* inBuffer, SInt32& inNumBytes)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	ByteCount bytesWritten;
	OSStatus err = ::FSWriteFork(GetResourceForkRefNum(), fsFromMark, 0,
									inNumBytes, inBuffer, &bytesWritten);

	inNumBytes = bytesWritten;

	return err;
#else
	// No resource fork in Windows files - write to nowhere
	return noErr;
#endif
}

// Get length of both forks
#if __dest_os == __mac_os || __dest_os == __mac_os_x
SInt32 CFullFileStream::GetLength() const
#elif __dest_os == __win32_os || __dest_os == __linux_os
UInt32 CFullFileStream::GetLength() const
#else
#error __dest_os
#endif
{
	SInt32	theLength;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (GetResourceForkRefNum() != refNum_Undefined)
	{
		SInt64		forkSize;
		OSStatus	err = ::FSGetForkSize(GetDataForkRefNum(), &forkSize);
		ThrowIfOSErr_(err);
		theLength = forkSize;
	}
	else
		theLength = 0;

	if (GetDataForkRefNum() != refNum_Undefined)
		theLength += LFileStream::GetLength();

#elif __dest_os == __win32_os || __dest_os == __linux_os
	theLength = LFileStream::GetLength();
#else
#error __dest_os
#endif

	return theLength;
}

// Get pos of both forks
SInt32 CFullFileStream::GetMarker() const
{
	SInt32	theMarker;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (GetResourceForkRefNum() != refNum_Undefined) {
		SInt64 forkPos;
		OSStatus err = ::FSGetForkPosition(GetDataForkRefNum(), &forkPos);
		ThrowIfOSErr_(err);
		theMarker = forkPos;
	}
	else
		theMarker = 0;
	if (GetDataForkRefNum() != refNum_Undefined)
		theMarker += LFileStream::GetMarker();

#elif __dest_os == __win32_os || __dest_os == __linux_os
	theMarker = LFileStream::GetMarker();
#else
#error __dest_os
#endif

	return theMarker;
}
