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


// filterbuf.h

#ifndef __MIMEFILTERS__MULBERRY__
#define __MIMEFILTERS__MULBERRY__

#include "filterbuf.h"

class mimefilterbuf : public filterbuf
{
public:
	mimefilterbuf(bool encodeit, bool for_network)
		: filterbuf(encodeit)
		{}
	virtual ~mimefilterbuf()
		{}

protected:
	bool				mForNetwork;

	void write_endl();
	const char* mime_endl() const
		{ return mForNetwork ? net_endl : os_endl; }
	unsigned long mime_endl_len() const
		{ return mForNetwork ? net_endl_len : os_endl_len; }

};

class mime_7bit_filterbuf : public mimefilterbuf
{
public:
	mime_7bit_filterbuf(bool encodeit, bool for_network = false) : mimefilterbuf(encodeit, for_network)
		{ mGotLineEnd = true; }
	virtual ~mime_7bit_filterbuf() {}

protected:
	bool mGotLineEnd;

	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);
};

class mime_8bit_filterbuf : public mimefilterbuf
{
public:
	mime_8bit_filterbuf(bool encodeit, bool for_network = false) : mimefilterbuf(encodeit, for_network)
		{ mGotLineEnd = true; }
	virtual ~mime_8bit_filterbuf() {}

protected:
	bool mGotLineEnd;

	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);
};

class mime_qp_filterbuf : public mimefilterbuf
{
public:
	mime_qp_filterbuf(bool encodeit, bool for_network = false) : mimefilterbuf(encodeit, for_network)
		{ mDecodeState = eDecodeNormal; }
	virtual ~mime_qp_filterbuf() {}

protected:
	enum EQPDecodeState
	{
		eDecodeNormal = 0,
		eDecodeQuoteLast,
		eDecodeQuoteChar1Last,
		eDecodeQuoteEndlLast
	};
	
	EQPDecodeState mDecodeState;
	char mQuotedChar;
	char mEndlLast;
	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);
};

class mime_base64_filterbuf : public mimefilterbuf
{
public:
	mime_base64_filterbuf(bool encodeit, bool for_network = false) : mimefilterbuf(encodeit, for_network)
		{ mAtomPos = 0; mEncodeLength = 0; }
	virtual ~mime_base64_filterbuf() {}

    virtual int sync ()		// Must output any remaining partial atom if encoding
    	{ if (mEncode) WriteAtom(); CheckBuffer(true); return 0; }

protected:

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
	
	TAtom			mAtom;
	short			mAtomPos;
	short			mEncodeLength;
	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);

private:
	void WriteAtom();
};

#endif
