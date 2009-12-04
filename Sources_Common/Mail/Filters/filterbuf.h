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

#ifndef __FILTERBUF__MULBERRY__
#define __FILTERBUF__MULBERRY__

#include "cdstring.h"

#include <streambuf>
#include <iostream>

const long filterbuf_size = 8192;

class CProgress;
class LStream;

class filterbuf : public std::streambuf
{
public:
	filterbuf(bool encode, CProgress* progress = NULL);
	virtual ~filterbuf();

	virtual void	SetStream(std::ostream* out)
		{ mOut = out; }
	virtual std::ostream*	GetOStream()
		{ return mOut; }
	virtual void	SetStream(LStream* out)
		{ mLOut = out; }
	virtual LStream*	GetLStream()
		{ return mLOut; }
	virtual void	SetBuffer(char* out, unsigned long buf_size)
		{ mBOut = out; mBOutSize = buf_size; }

	virtual void	SetProgress(CProgress* progress)
		{ mProgress = progress; }

	unsigned long	GetWritten() const
		{ return mBOutWritten; }

protected:
	std::ostream* 	mOut;
	LStream*		mLOut;
	char*			mBOut;
	unsigned long	mBOutSize;
	unsigned long	mBOutWritten;
	CProgress*		mProgress;
	bool	 		mEncode;
	char*			mBuffer;
	char*			mBufferNext;
	unsigned long	mBufferCount;

	void CreateBuffer();
	void WriteBuffer(const char_type* s, std::streamsize n);
	bool CheckBuffer(bool force = false);

    virtual int sync ()
    	{ CheckBuffer(true); return 0; }

	virtual void	write(const char_type* s, std::streamsize n);

	virtual std::streamsize xsputn (const char_type* s, std::streamsize n)
		{ return (mEncode ? encode(s, n) : decode(s, n)); }
	virtual int_type overflow (int_type c = traits_type::eof () )
		{ char ch = c; xsputn(&ch, 1); return c; }

	virtual std::streamsize encode (const char_type* s, std::streamsize n) = 0;
	virtual std::streamsize decode (const char_type* s, std::streamsize n) = 0;
};

// Class to do line end conversions on a stream
class crlf_filterbuf : public filterbuf
{
public:
	crlf_filterbuf(EEndl out_endl) : filterbuf(true)
		{ mOutEndl = out_endl; mCRLast = false; }
	virtual ~crlf_filterbuf() {}

protected:
	EEndl	mOutEndl;
	bool	mCRLast;

	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);
};

// Class to dot-stuff a stream (used for SMTP send DATA command)
class dotstuff_filterbuf : public filterbuf
{
public:
	dotstuff_filterbuf(bool encode_it) : filterbuf(encode_it) { got_crlf = true; }
	virtual ~dotstuff_filterbuf() {}

protected:
	bool got_crlf;
	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);
};

// Class to from-stuff a stream (used to prevent unix mailbox format delimiters
// appearing in stream and forcing an invalid message break)
class fromstuff_filterbuf : public filterbuf
{
public:
	fromstuff_filterbuf(bool encode_it) : filterbuf(encode_it) { mCRLast = false; got_crlf = true; }
	virtual ~fromstuff_filterbuf() {}

protected:
	bool		mCRLast;
	bool		got_crlf;
	cdstring	mPartialLine;

	virtual std::streamsize encode (const char_type* s, std::streamsize n);
	virtual std::streamsize decode (const char_type* s, std::streamsize n);

    virtual int sync ()		// Must output any remaining partial atom if encoding
    	{ if (mEncode) FinishLine(); CheckBuffer(true); return 0; }

private:
	void WriteLineWithStuffing(const char_type* s, std::streamsize n);
	void FinishLine();
};

#endif
