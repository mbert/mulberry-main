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


// Source for CConverterBase class

#include "CConverterBase.h"

using namespace i18n;

char CConverterBase::undefined_charmap = '?';		// Undefined mapping character
wchar_t CConverterBase::undefined_wcharmap = '\0?';	// Undefined mapping character

void CConverterBase::ToUnicode(const char* str, size_t len, std::ostream& wout)
{
	// Must have valid input
	if (!str)
		return;

	// Convert each character
	const unsigned char* p = reinterpret_cast<const unsigned char*>(str);
	const unsigned char* q = p + len;
	while(p < q)
	{
		wchar_t wc = c_2_w(p);
#ifdef big_endian
		wout.put(wc >> 8);
		wout.put(wc & 0x00FF);
#else
		wout.put(wc & 0x00FF);
		wout.put(wc >> 8);
#endif
	}
}

void CConverterBase::FromUnicode(const wchar_t* wstr, size_t wlen, std::ostream& out)
{
	// Must have valid input
	if (!wstr)
		return;

	// Initialise the converter
	init_w_2_c(out);

	// Convert each character
	const wchar_t* p = wstr;
	const wchar_t* q = p + wlen;
	while(p < q)
	{
		char bout[32];
		int len = w_2_c(*p++, bout);
		for(int i = 0; i < len; i++)
			out.put(bout[i]);
	}

	// Finalise the converter
	finish_w_2_c(out);
}

void CConverterBase::ToUTF16(const char* str, size_t len, std::ostream& wout)
{
	// Must have valid input
	if (!str)
		return;

	// Convert each character
	const unsigned char* p = reinterpret_cast<const unsigned char*>(str);
	const unsigned char* q = p + len;
	while(p < q)
	{
		wchar_t wc = c_2_w(p);
		if (wc < 0x10000)
		{
#ifdef big_endian
			unsigned char c1 = (wc & 0xFF00) >> 8;
			unsigned char c2 = (wc & 0x00FF);
#else
			unsigned char c1 = (wc & 0x00FF);
			unsigned char c2 = (wc & 0xFF00) >> 8;
#endif
			wout.put(c1);
			wout.put(c2);
		}
		else if (wc > 0x10FFFF)
		{
#ifdef big_endian
			unsigned char c1 = 0;
			unsigned char c2 = '?';
#else
			unsigned char c1 = '?';
			unsigned char c2 = 0;
#endif
			wout.put(c1);
			wout.put(c2);
		}
		else
		{
			wc -= 0x10000;
			wchar_t wc1 = 0xD800 | ((wc & 0x000FFC00) >> 10);
			wchar_t wc2 = 0xDC00 | (wc & 0x000003FF);

#ifdef big_endian
			unsigned char c1 = (wc1 & 0xFF00) >> 8;
			unsigned char c2 = (wc1 & 0x00FF);
			unsigned char c3 = (wc2 & 0xFF00) >> 8;
			unsigned char c4 = (wc2 & 0x00FF);
#else
			unsigned char c1 = (wc1 & 0x00FF);
			unsigned char c2 = (wc1 & 0xFF00) >> 8;
			unsigned char c3 = (wc2 & 0x00FF);
			unsigned char c4 = (wc2 & 0xFF00) >> 8;
#endif
			wout.put(c1);
			wout.put(c2);
			wout.put(c3);
			wout.put(c4);
		}
	}
}

void CConverterBase::FromUTF16(const unsigned short* str, size_t ulen, std::ostream& out)
{
	// Must have valid input
	if (!str)
		return;

	// Convert each character
	const unsigned short* p = str;
	unsigned long charlen = 0;
	wchar_t wc = 0;
	while(*p)
	{
		if (charlen == 0)
		{
			// Look for mbcs ranges
			if ((*p < 0xD800) || (*p > 0xDFFF))
			{
				charlen = 1;
				wc = *p;
			}
			else if (*p < 0xDC00)
			{
				// Must be valid
				charlen = 2;
				wc = (*p & 0x03FF) << 10;
			}
			else
			{
				// Sequence error
				charlen = 1;
				wc = '?';
			}
		}
		else
		{
			// Must be valid
			if ((*p >= 0xDC00) && (*p <= 0xDFFF))
			{
				wc |= (*p & 0x03FF);
			}
			else
				wc = '?';
		}
		
		// Bump ptr
		p++;
		
		// Reduce byte remaining count and write it out if done
		if (!--charlen)
		{
			char bout[32];
			int len = w_2_c(wc, bout);
			for(int i = 0; i < len; i++)
				out.put(bout[i]);
		}
	}
}

void CConverterBase::ToUTF8(const char* str, size_t len, std::ostream& out)
{
	// Must have valid input
	if (!str)
		return;

	// Convert each character
	const unsigned char* p = reinterpret_cast<const unsigned char*>(str);
	const unsigned char* q = p + len;
	while(p < q)
	{
		wchar_t wp = c_2_w(p);
		if (wp < 0x80)
		{
			// Write 1 to buffer
			unsigned char c = wp;
			out.put(c);
		}
		else if (wp < 0x800)
		{
			// Write 2 to buffer
			unsigned char c = 0xc0 | (wp >> 6);
			out.put(c);

			c = 0x80 | (wp & 0x3f);
			out.put(c);
		}
		else // if (wp < 0x10000)
		{
			// Write 3 to buffer
			unsigned char c = 0xe0 | (wp >> 12);
			out.put(c);

			c = 0x80 | ((wp >> 6) & 0x3f);
			out.put(c);

			c = 0x80 | (wp & 0x3f);
			out.put(c);
		}
	}
}

void CConverterBase::FromUTF8(const char* str, size_t len, std::ostream& out)
{
	// Must have valid input
	if (!str)
		return;

	// Convert each character
	const char* p = str;
	unsigned long charlen = 0;
	wchar_t wc = 0;
	while(*p)
	{
		unsigned char mask = 0x3f;
		if (charlen == 0)
		{
			// Determine length of utf8 encoded wchar_t
			if ((*p & 0xf0 ) == 0xe0)
			{
				charlen = 3;
				mask = 0x0f;
			}
			else if ((*p & 0xe0 ) == 0xc0)
			{
				charlen = 2;
				mask = 0x1f;
			}
			else
			{
				charlen = 1;
				mask = 0x7f;
			}

			// Reset char
			wc = 0;
		}

		// Convert the byte
		wc <<= 6;
		wc |= (*p & mask);
		
		// Bump ptr
		p++;
		
		// Reduce byte remaining count and write it out if done
		if (!--charlen)
		{
			char bout[32];
			int len = w_2_c(wc, bout);
			for(int i = 0; i < len; i++)
				out.put(bout[i]);
		}
	}
}

void CConverterBase::init_w_2_c(std::ostream& out)
{
}

void CConverterBase::finish_w_2_c(std::ostream& out)
{
}
