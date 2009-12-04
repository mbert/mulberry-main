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


// Header for CMIMEContent class

#ifndef __CMIMECONTENT__MULBERRY__
#define __CMIMECONTENT__MULBERRY__

#include "cdstring.h"
#include "CCharsetCodes.h"
#include "CMIMETypes.h"

// Class

class CMIMEContent
{
public:
	CMIMEContent();
	CMIMEContent(ETransferMode mode, EContentType type,
					EContentSubType subtype, EContentTransferEncoding encoding = eNoTransferEncoding);
	explicit CMIMEContent(unsigned long level);
	CMIMEContent(const CMIMEContent& copy)
		{ _copy(copy); }
	~CMIMEContent();

	CMIMEContent& operator=(const CMIMEContent& copy)	// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	bool RecoverCompare(const CMIMEContent& comp) const;	// Do reconstruct recovery test

	void		NullDiff(CMIMEContent& content);
	void		NullAdd(CMIMEContent& content);

	void		SetTransferMode(ETransferMode mode);
	ETransferMode GetTransferMode() const
				 		{ return mTransferMode; }

	void	ConvertDummy();
	bool	IsDummy() const;

	void	SetContent(EContentType type, EContentSubType subtype);
	void	SetContent(const char* xtype, EContentSubType subtype);
	void	SetContent(EContentType type, const char* xsubtype);
	void	SetContent(const char* xtype, const char* xsubtype);
	void	SetContent(const unsigned char* mime);
	void	SetContent(const char* mime);

	void	SetContentType(const char* type);
	void	SetContentType(EContentType type);
	EContentType GetContentType() const;
	cdstring GetContentTypeText() const;

	void	SetContentSubtype(const char* subtype);
	void	SetContentSubtype(EContentSubType subtype);
	EContentSubType GetContentSubtype() const;
	cdstring GetContentSubtypeText() const;

	void	SetContentParameter(const char* param1, const char* param2);
	void	RemoveContentParameter(const char* param);
	const cdstrpairvect& GetContentParameters() const
		{ return mContentParams; }
	short	CountContentParameters() const
				{ return mContentParams.size(); }
	cdstring	GetContentParameterText(short index) const;
	const cdstring&	GetContentParameter(const char* param) const;
	const cdstring&	GetMultipartBoundary() const;
	const cdstring& GetXMulberryFile() const;

	void	SetCharset(i18n::ECharsetCode charset)
				{ mCharset = charset; }
	i18n::ECharsetCode	GetCharset() const
					{ return mCharset; }

	void	SetTransferEncoding(EContentTransferEncoding encoding)
				{ mEncoding = encoding; }
	void	SetTransferEncoding(const char* encoding);
	EContentTransferEncoding GetTransferEncoding() const;
	cdstring GetTransferEncodingText() const;

	void	SetDontEncode()
		{ mDontEncode = true; }
	bool	GetDontEncode() const
		{ return mDontEncode; }

	void	SetContentId(const char* Id)
				{ mContentId = Id; }
	const cdstring&	GetContentId() const { return mContentId; }

	void	SetContentDescription(const char* description)
				{ mContentDescription = description; }
	const cdstring&	GetContentDescription() const
				{ return mContentDescription; }

	void	SetContentDisposition(EContentDisposition disposition)
				{ mContentDisposition = disposition; }
	void	SetContentDisposition(const char* disposition);
	EContentDisposition GetContentDisposition() const
				{ return mContentDisposition; }
	cdstring GetContentDispositionText() const;

	void	SetContentDispositionParameter(const char* param1, const char* param2);

	void	SetMappedName(const char* name)
				{ mMappedName = name; }
	void	SetMappedName(const unsigned char* name)
				{ mMappedName = name; }
	const cdstring&	GetMappedName() const
						{ return mMappedName; }

	void	SetMultiLevel(unsigned long level)
				{ mMultiLevel = level; }
	unsigned long	GetMultiLevel() const
				{ return mMultiLevel; }

	void	SetContentSize(unsigned long size)
				{ mContentSize = size; }
	void	SetContentSize(const char* size);
	unsigned long	GetContentSize() const
				{ return mContentSize; };

	bool	IsBinHexed() const;
	bool	IsUUed() const;
	bool	IsFlowed() const;
	bool	IsDelsp() const;
	bool	IsTextFlowed() const;

	void WriteCacheToStream(std::ostream& out) const;
	void ReadCacheFromStream(std::istream& in, unsigned long vers);

private:
	ETransferMode				mTransferMode;
	EContentType				mType;
	cdstring					mXtype;
	EContentSubType				mSubtype;
	cdstring					mXsubtype;
	cdstrpairvect				mContentParams;
	i18n::ECharsetCode			mCharset;
	EContentTransferEncoding	mEncoding;
	bool						mDontEncode;
	cdstring					mContentId;
	cdstring					mContentDescription;
	EContentDisposition			mContentDisposition;
	cdstrpairvect*				mContentDispositionParams;
	cdstring					mMappedName;
	unsigned long				mMultiLevel;
	unsigned long				mContentSize;

	void	InitMIMEContent();
	void	_copy(const CMIMEContent& copy);

	void	SetDefaultEncoding();
};

#endif
