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


// Header for CMIMEFilters classes

#ifndef __CMIMEFILTERS__MULBERRY__
#define __CMIMEFILTERS__MULBERRY__

#include "CFilter.h"
#include "CCharsetCodes.h"

const unsigned long cWrapLength = 76;

class CMIMEFilter : public CFilter
{
public:
					CMIMEFilter(i18n::ECharsetCode charset = i18n::eUSASCII,
								bool is_text = true,
								bool is_flowed = false,
								bool is_delsp = false,
								LStream* aStream = NULL,
								CProgress* progress = NULL);
	virtual			~CMIMEFilter();

	virtual bool	Complete() const;

protected:
	enum ESigDashState
	{
		eSigDashNone = 0,
		eSigDash1,
		eSigDash2,
		eSigDash3
	};

	unsigned char*	mLineBuffer;
	SInt32			mLineBufferLength;
	unsigned char*	mLinePos;
	SInt32			mLineLength;

	i18n::ECharsetCode	mCharset;
	char				mLastChar;
	bool				mIsText;
	bool				mIsFlowed;
	bool				mIsDelsp;
	bool				mGotCRLF;
	bool				mLineQuoted;
	ESigDashState		mSigDashState;
	
			bool	FlowProcess(unsigned char c);
};

class C8bitFilter : public CMIMEFilter {		// Convert to/from 8bit with charset

public:
					C8bitFilter(i18n::ECharsetCode charset = i18n::eUSASCII,
								bool is_text = true,
								bool is_flowed = false,
								bool is_delsp = false,
								LStream* aStream = NULL,
								CProgress* progress = NULL);
	virtual			~C8bitFilter();

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

private:
	enum EReadStatus {
		eLineBuild = 1,
		eLineCopy
	};

	EReadStatus		mStatus;
	bool			mCRLast;
};

class CQPFilter : public CMIMEFilter {		// Convert to/from quoted-printable

public:
					CQPFilter(i18n::ECharsetCode charset = i18n::eUSASCII,
								bool is_text = true,
								bool is_flowed = false,
								bool is_delsp = false,
								LStream* aStream = NULL,
								CProgress* progress = NULL);
	virtual			~CQPFilter();

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

private:
	enum EReadStatus {
		eReadNormal = 1,
		eLineBuild,
		eLineCopy
	};

	enum EWriteStatus {
		eWriteNormal = 1,
		eQuoteLast,
		eQuoteChar1Last,
		eQuoteEndlLast
	};

	unsigned char*	mTempBuffer;
	EReadStatus		mReadStatus;
	EWriteStatus	mWriteStatus;
	unsigned char	mQuotedChar;
	unsigned char	mQuotedCharLast;
	unsigned char	mQuoted1;
	unsigned char	mQuoted2;
	char			mEndlLast;

	virtual SInt32	CopyQuoted(unsigned char* dest, unsigned char* src, long len);

};

class CBase64Filter : public CMIMEFilter {		// Convert to/from Base64

public:
					CBase64Filter(i18n::ECharsetCode charset = i18n::eUSASCII,
									bool is_text = true,
									bool is_flowed = false,
									bool is_delsp = false,
									LStream* aStream = NULL,
									CProgress* progress = NULL);
	virtual			~CBase64Filter();

	virtual ExceptionCode	GetBytes(void *outBuffer, SInt32& inByteCount);
	virtual ExceptionCode	PutBytes(const void *inBuffer, SInt32& inByteCount);

private:
	enum EReadStatus {
		eReadNormal = 1,
		eLineBuild,
		eLineCopy
	};

	struct TBase64 {
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

	union TAtom {
		TBase64			base64;
		unsigned char	base256[3];
	};

	EReadStatus		mReadStatus;
	TAtom			mAtom;
	short			mAtomPos;
	unsigned char	mCharLast;

	virtual SInt32	Encode(unsigned char* dest, unsigned char* src, long len);
			void	PutChar(unsigned char c);
};

#endif
