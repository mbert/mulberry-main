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


// Header for CFilter class

#ifndef __CBINHEXFILTER__MULBERRY__
#define __CBINHEXFILTER__MULBERRY__

#include "CCreateFileFilter.h"

// Classes

class CProgress;

class CBinHexFilter : public CCreateFileFilter
{

	enum EBinHexStatus
	{
		eStart1 = 1,
		eCopyStart,
		eHeader,
		eCopyHeader,
		ePutHeaderCRC,
		eDataForkCopy,
		ePutDataCRC,
		eRsrcForkCopy,
		ePutRsrcCRC,
		eTerminate,
		ePutLast,
		eEnd1
	};

	enum EBinHexSubStatus
	{
		eNormal = 1,
		eCRNext,
		eLFNext
	};

	enum EBinHexProcess
	{
		eFirstChar = 1,
		eNextChar,
		eRunNext,
		eRepeater
	};

	enum EDeBinHexStatus
	{
		eStart2 = 1,
		eGetStrLen,
		eGetStr,
		eGetStrNull,
		eGetType,
		eGetCreator,
		eGetFlags,
		eGetDataLen,
		eGetRsrcLen,
		eGetHeadCRC,
		eGetData,
		eGetDataCRC,
		eGetRsrc,
		eGetRsrcCRC,
		eGetEndColon,
		eEnd2
	};

	enum EDeBinHexSubStatus
	{
		eNormalChar = 1,
		eStartRun,
		eDoingRun
	};

	enum EDeBinHexError
	{
		eNoErr = 0,
		eIllegalCode,
		eStrToLong,
		eNoStrNull,
		eWrongHeadCRC,
		eWrongDataCRC,
		eWrongRsrcCRC,
		eNoTerminator,
		eNoMore
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

private:
	EBinHexStatus		mStatus1;
	EBinHexSubStatus	mSubStatus1;
	EBinHexProcess		mProcess;
	EDeBinHexStatus		mStatus2;
	EDeBinHexSubStatus	mSubStatus2;
	bool				mDecodedOne;
	unsigned short		mCRC;
	short				mLineCount;
	TAtom				mCurrentAtom;
	short				mAtomPos;
	unsigned char		mLastChar;
	unsigned char		mRunLength;
	SInt32				mDataLen;
	SInt32				mRsrcLen;
	unsigned short		mCRCCalc;
	unsigned short		mCRCFound;
	unsigned char*		mFileOutBuffer;

public:
					CBinHexFilter(LStream* aStream = nil, CProgress* progress = nil);
	virtual			~CBinHexFilter();

private:
	virtual void	InitBinHexFilter();

public:
	virtual bool	Complete() const;

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

private:
	virtual void	InputFileInit();
	virtual void	CreateHeader();
	virtual void	Encode(unsigned char** outbuffer, long* total, long maxbuffer);

	virtual void	DecodeIt(unsigned char** inbuffer,
								bool to_data,
								bool to_rsrc);

	virtual void	IncrementCRC(unsigned char c);
};

#endif
