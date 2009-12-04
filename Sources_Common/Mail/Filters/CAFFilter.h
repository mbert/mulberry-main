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


// Header for CAFFilter class

#ifndef __CAFFILTER__MULBERRY__
#define __CAFFILTER__MULBERRY__

#include "CFilter.h"
#include "CFullFileStream.h"

// Constants


// Classes

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#elif __dest_os == __win32_os
#pragma pack(push, 2)
#elif __dest_os == __linux_os
//gcc uses the packed attribute (see below)
#else
#error __dest_os
#endif

struct SAppleFileEntry
{
	unsigned long entryID;
	unsigned long entryOffset;
	unsigned long entryLen;
};

struct SAppleFileHeader
{
	unsigned long magicNum;
	unsigned long versionNum;
	unsigned char filler[16];
	unsigned short numEntries;
}
#if __dest_os == __linux_os
__attribute__ ((packed))
#endif
;

struct SAppleFileIndex
{
	SAppleFileHeader header;
#if __dest_os == __linux_os
	SAppleFileEntry entries[6] __attribute__ ((packed)); // Only encode/decode 6 entry types
#else
	SAppleFileEntry entries[6]; // Only encode/decode 6 entry types
#endif
};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#elif __dest_os == __win32_os
#pragma pack(pop)
#elif __dest_os == __linux_os
//gcc uses the packed attribute
#else
#error __dest_os
#endif

class CAFFilter : public CFilter
{

	enum EAFStatus
	{
		eStart1 = 1,
		eCreateHeader,
		eHeaderCopy,
		eRsrcForkCopy,
		eDataForkCopy,
		eEnd1
	};

	enum EDeAFStatus
	{
		eStart2 = 1,
		eGetHeader,
		eGetEntries,
		eProcessEntries,
		eGetRealName,
		eGetFinderInfo,
		eGetFileDates,
		eGetComment,
		eGetRsrc,
		eGetData,
		eEnd2
	};

public:
					CAFFilter(LStream* aStream = nil, CProgress* progress = nil);
	virtual			~CAFFilter();

	virtual void	SetStream(LStream* aStream, bool single)
						{ mStream = aStream;
						  mFileStream = (CFullFileStream*) aStream;
						  mSingle = single; }

	virtual bool	Complete() const;

	virtual ExceptionCode	GetBytes(void *outBuffer, long& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, long& inByteCount);

private:
	CFullFileStream*	mFileStream;
	bool				mSingle;
	SAppleFileIndex		mIndex;
	char*				mHeaderBuffer;
	unsigned long		mHeaderBufferLen;
	unsigned long		mGetOffset;
	unsigned long		mGetTotal;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CInfoPBRec			mGetInfo;
	Str255				mGetRealName;
	Str255				mGetComment;
#else
	cdstring			mGetRealName;
#endif
	EAFStatus			mStatus1;
	EDeAFStatus			mStatus2;

	virtual void	InitAFFilter();
	virtual void	CreateHeader();

};

#endif
