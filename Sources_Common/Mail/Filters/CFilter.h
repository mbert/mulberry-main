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

#ifndef __CFILTER__MULBERRY__
#define __CFILTER__MULBERRY__

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
#include <WIN_LStream.h>
#elif __dest_os == __linux_os
#include "UNX_LStream.h"
#endif


#include "cdstring.h"


// Consts
const long	cMaxBuffer = 8192L;

// Classes

class CProgress;

class CFilter : public LStream
{
protected:

	unsigned char* 	mBuffer;
	unsigned char*	mBufferPos;
	SInt32			mBufferLength;
	LStream*		mStream;
	CProgress*		mProgress;
	EEndl			mOutEndl;
	bool			mAutoCreate;

public:
					CFilter(LStream* aStream = nil, CProgress* progress = nil);
	virtual			~CFilter();

	virtual void	SetForNetwork(EEndl out_endl)
						{ mOutEndl = out_endl; }
	virtual void	SetStream(LStream* aStream)
						{ mStream = aStream; }
	virtual void	SetProgress(CProgress* progress)
						{ mProgress = progress; }
	virtual void	SetLength(SInt32	inLength)
						{ mStream->SetLength(inLength); }
	virtual void	SetAutoCreate(bool	auto_create)
						{ mAutoCreate = auto_create; }
	virtual bool	Complete() const;

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

protected:
			bool	CheckBuffer(bool force, SInt32& written);

};

class CFilterEndls : public CFilter {

	enum EReadStatus
	{
		eLineBuild = 1,
		eLineCopy
	};

private:
	unsigned char*	mLineBuffer;
	unsigned char*	mLinePos;
	SInt32			mLineLength;
	EReadStatus		mStatus;

public:
					CFilterEndls(LStream* aStream = nil, CProgress* progress = nil);
	virtual			~CFilterEndls();

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

};

#endif
