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


// filterbuf.cp

#include "mimefilters.h"

#include "CCharSpecials.h"
#include "CURL.h"

#pragma mark ____________________________mimefilterbuf

void mimefilterbuf::write_endl()
{
	// Always write out line end
	*mBufferNext++ = mime_endl()[0];
	mBufferCount++;
	
	// May be more than one char in line end
	if (mime_endl_len() == 2)
	{
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		*mBufferNext++ = mime_endl()[1];
		mBufferCount++;
	}
}

#pragma mark ____________________________mime_7bit_filterbuf

std::streamsize mime_7bit_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// Filter in LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;
	while(ntemp)
	{
		switch(*p)
		{
		case lendl1:
			write_endl();
			mGotLineEnd = true;
			break;
		
#if __line_end == __crlf
		case lendl2:
			// Always ignore
			p++;
			ntemp--;
			break;
#endif

		default:
			*mBufferNext++ = *p++;
			mBufferCount++;
			ntemp--;
			mGotLineEnd = false;
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;
}

std::streamsize mime_7bit_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Filter out LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;
	while(ntemp)
	{
		switch(*p)
		{
#if __line_end != __crlf
		case lendl3:
			// Always ignore
			p++;
			ntemp--;
			break;
#endif
		default:
			*mBufferNext++ = *p++;
			mBufferCount++;
			ntemp--;
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;
}


#pragma mark ____________________________mime_8bit_filterbuf

std::streamsize mime_8bit_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// Filter in LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;
	while(ntemp)
	{
		switch(*p)
		{
		case lendl1:
			// Always write out line end
			write_endl();
			mGotLineEnd = true;
			break;
		
#if __line_end == __crlf
		case lendl2:
			// Always ignore
			p++;
			ntemp--;
			break;
#endif

		default:
			*mBufferNext++ = *p++;
			mBufferCount++;
			ntemp--;
			mGotLineEnd = false;
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;
}

std::streamsize mime_8bit_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Filter out LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;
	while(ntemp)
	{
		switch(*p)
		{
#if __line_end != __crlf
		case lendl3:
			// Always ignore
			p++;
			ntemp--;
			break;
#endif
		default:
			*mBufferNext++ = *p++;
			mBufferCount++;
			ntemp--;
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;
}

#pragma mark ____________________________mime_qp_filterbuf

std::streamsize mime_qp_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// Filter in LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;
	while(ntemp)
	{
		switch(*p)
		{
		case lendl1:
			// Always write out line end
			write_endl();
			break;
		
#if __line_end == __crlf
		case lendl2:
			// Always ignore
			p++;
			ntemp--;
			break;
#endif

		default:
			// Check whether it needs QP'ing
			if (cQPChar[*p])
			{
				*mBufferNext++ = *p++;
				mBufferCount++;
				ntemp--;
			}
			else
			{
				*mBufferNext++ = '=';
				mBufferCount++;
				if (mBufferCount == filterbuf_size)
					CheckBuffer();

				// Map to charset
				unsigned char d = *p++;
				ntemp--;

				// Do high nibble
				*mBufferNext++ = cHexChar[(d >> 4)];
				mBufferCount++;
				if (mBufferCount == filterbuf_size)
					CheckBuffer();

				// Do low nibble
				*mBufferNext++ = cHexChar[(d & 0x0F)];
				mBufferCount++;
			}
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;
}

std::streamsize mime_qp_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Filter out CRLFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;

	while(ntemp)
	{
		switch(mDecodeState)
		{
		case eDecodeNormal:
		case eDecodeQuoteEndlLast:
			switch(*p)
			{
			case '=':
				// Advance past quote and change status
				p++;
				ntemp--;
				mDecodeState = eDecodeQuoteLast;
				break;

			case '\r':
			case '\n':
				if ((mDecodeState == eDecodeQuoteEndlLast) &&
					(*p != mEndlLast))
				{
					// Skip it - its part of a soft break
					p++;
					ntemp--;
					mDecodeState = eDecodeNormal;
					break;
				}
				
			// Fall through for default processing
			default:
#if __line_end != __crlf
				// Ignore the unwanted CR or LF
				if (*p == lendl3)
				{
					p++;
					ntemp--;
				}
				else
#endif
				{
					// Translate charset
					*mBufferNext++ = *p++;
					mBufferCount++;
					ntemp--;
				}
				mDecodeState = eDecodeNormal;
				break;
			}
			break;

		case eDecodeQuoteLast:
			switch(*p)
			{
			case '\r':
			case '\n':
				mEndlLast = *p++;
				ntemp--;
				mDecodeState = eDecodeQuoteEndlLast;
				break;

			default:
				// Get the high nibble character
				unsigned char c = *p++;
				ntemp--;
				
				// Just copy high nibble of quoted char
				mQuotedChar = 0;
				mQuotedChar = cFromHex[c] << 4;
				
				// Change state
				mDecodeState = eDecodeQuoteChar1Last;
				break;
			}
			break;
		
		case eDecodeQuoteChar1Last:
			// Get the low nibble character
			unsigned char c = *p++;
			ntemp--;
			
			// Just copy low nibble of quoted char
			mQuotedChar |= cFromHex[c];

#if __line_end != __crlf
			// Do not process hard LFs on Mac
			if (mQuotedChar != lendl3)
#endif
			{
				// Write completed char to buffer mapped from charset
				*mBufferNext++ = mQuotedChar;
				mBufferCount++;
			}

			// Change status
			mDecodeState = eDecodeNormal;
			break;
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;	
}

#pragma mark ____________________________mime_base64_filterbuf

// Constants for base64
const short cWrapLength = 76;

const signed char cdebase64[] =
						 {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 15
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 31
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,	// 47
						  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,	// 63
						  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,	// 79
						  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,	// 95
						  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	// 111
						  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,	// 127
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 143
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 159
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 175
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 191
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 207
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 223
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 239
						  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                      0         1         2         3         4         5         6
//                      0123456789012345678901234567890123456789012345678901234567890123
const char cbase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char cbase64pad = '=';

const long cEncodeLength = (3L*cWrapLength) >> 2;

std::streamsize mime_base64_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// Filter in LFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;

	while(ntemp)
	{
		unsigned char c = *p++;
		ntemp--;

		switch(mAtomPos)
		{
		case 0:
#ifdef big_endian
			mAtom.base256[0] = c;
#else
			mAtom.base256[2] = c;
#endif
			mAtomPos++;
			break;
		
		case 1:
#ifdef big_endian
			mAtom.base256[1] = c;
#else
			mAtom.base256[1] = c;
#endif
			mAtomPos++;
			break;

		case 2:
#ifdef big_endian
			mAtom.base256[2] = c;
#else
			mAtom.base256[0] = c;
#endif
			mAtomPos = 0;
			
			// Now write out complete atom
			WriteAtom();
			mEncodeLength += 3;

			// Check encoding line length
			if (mEncodeLength >= cEncodeLength)
			{
				// Always write out line end
				write_endl();

				if (mBufferCount == filterbuf_size)
					CheckBuffer();

				mEncodeLength = 0;
			}
			break;
		}
	}
	
	return n;
}

std::streamsize mime_base64_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Filter out CRLFs
	const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
	std::streamsize ntemp = n;

	while(ntemp)
	{
		unsigned char c = *p++;
		ntemp--;

		switch(c)
		{
		case lendl1:
#if __line_end == __crlf
		case lendl2:
#else
		case lendl3:
#endif
			// Do not decode crlfs
			break;
		default:
			switch(mAtomPos)
			{
			case 0:
				mAtom.base64.b0 = cdebase64[c];
				mAtomPos++;
				break;

			case 1:
				mAtom.base64.b1 = cdebase64[c];
				mAtomPos++;

				// Now write it out to buffer
#ifdef big_endian
				*mBufferNext++ = mAtom.base256[0];
#else
				*mBufferNext++ = mAtom.base256[2];
#endif
				mBufferCount++;
				break;

			case 2:
				{
					bool got_pad2 = (c == cbase64pad);
					mAtom.base64.b2 = cdebase64[c];
					mAtomPos++;
					
					// Now write it out to buffer if not pad
					if (!got_pad2)
						{
#ifdef big_endian
							*mBufferNext++ = mAtom.base256[1];
#else
							*mBufferNext++ = mAtom.base256[1];
#endif
							mBufferCount++;
						}
					break;
				}
			case 3:
				bool got_pad3 = (c == cbase64pad);
				mAtom.base64.b3 = cdebase64[c];
				mAtomPos = 0;

				// Now write it out to buffer if not pad
				if (!got_pad3)
				{
#ifdef big_endian
					*mBufferNext++ = mAtom.base256[2];
#else
					*mBufferNext++ = mAtom.base256[0];
#endif
					mBufferCount++;
				}
				break;
			}
		}

		// Must check buffer for overflow
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	return n;	
}

void mime_base64_filterbuf::WriteAtom()
{
	// Do padding if required
	switch(mAtomPos)
	{
	case 0:
		// Not required - atom complete
		*mBufferNext++ = cbase64[mAtom.base64.b0];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b1];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b2];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b3];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		break;
	case 1:
		// Fill remainder of atom with unencoded chars
#ifdef big_endian
		mAtom.base256[1] = 0;
		mAtom.base256[2] = 0;
#else
		mAtom.base256[1] = 0;
		mAtom.base256[0] = 0;
#endif

		// Fill destination with encoded chars and pad
		*mBufferNext++ = cbase64[mAtom.base64.b0];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b1];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64pad;
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64pad;
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		break;

	case 2:
		// Fill remainder of atom with unencoded chars
#ifdef big_endian
		mAtom.base256[2] = 0;
#else
		mAtom.base256[0] = 0;
#endif

		// Fill destination with encoded chars and pad
		*mBufferNext++ = cbase64[mAtom.base64.b0];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b1];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64[mAtom.base64.b2];
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		
		*mBufferNext++ = cbase64pad;
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
		break;
	}
	
	mAtomPos = 0;
}
