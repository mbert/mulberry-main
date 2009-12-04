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


// Header for CUUFilters classes

#ifndef __CUUFILTER__MULBERRY__
#define __CUUFILTER__MULBERRY__

#include "CCreateFileFilter.h"

class CUUFilter : public CCreateFileFilter
{

public:
					CUUFilter(LStream* aStream = nil, CProgress* progress = nil);
	virtual			~CUUFilter();

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

private:
	enum EEncodeStatus
	{
		eStart = 1,
		eCopyStart,
		eLineBuild,
		eLineCopy,
		eEnd,
		eCopyEnd
	};
	enum EDecodeStatus
	{
		eDecodeStart = 1,
		eGetCount,
		eDecode,
		eGotCR,
		eGotLF,
		eDecodeClose,
		eDecodeEnd
	};

	struct TBase64
	{
#ifdef big_endian
		unsigned b0 : 6;
		unsigned b1 : 6;
		unsigned b2 : 6;
		unsigned b3 : 6;
#else
		unsigned b3 : 6;
		unsigned b2 : 6;
		unsigned b1 : 6;
		unsigned b0 : 6;
#endif
	};

	union TAtom
	{
		TBase64			base64;
		unsigned char	base256[3];
	};

	unsigned char*		mLineBuffer;
	unsigned char*		mLinePos;
	SInt32				mLineLength;
	EEncodeStatus		mEncodeStatus;
	EDecodeStatus		mDecodeStatus;
	bool				mDecodedOne;
	bool				mUUBase64;
	const signed char*	mDecodeLookup;
	TAtom				mAtom;
	short				mAtomPos;

	virtual SInt32	Encode(unsigned char* dest, unsigned char* src, long len);
	virtual void	CreateHeader();
	virtual void	InputFileInit();
};

#endif
