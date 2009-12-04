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


// Source for CIMAPUTF7 class

#include "CIMAPUTF7.h"

using namespace i18n;

const char cSafemUTF7[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
	  1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 48 - 63
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 80 - 95
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// 240 - 255

// Constants for modified-UTF7 base64
const signed char cmUTF7debase64[] =
	{	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 15
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 31
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, 63, -1, -1, -1,		// 47
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,		// 63
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,		// 79
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,		// 95
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,		// 111
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,		// 127
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 143
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 159
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 175
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 191
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 207
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 223
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 239
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                      	 0         1         2         3         4         5         6
//                  	     0123456789012345678901234567890123456789012345678901234567890123
const char cmUTF7base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

wchar_t CIMAPUTF7::c_2_w(const unsigned char*& c)
{
	// Determine length of utf8 encoded wchar_t and mask for first byte bits
	unsigned char cp = *c;
	unsigned char mask = 0x3f;
	unsigned long charlen = 0;
	if (cp < 0x80)
	{
		charlen = 1;
		mask = 0x7f;
	}
	else if ((cp & 0xe0 ) == 0xc0)
	{
		charlen = 2;
		mask = 0x1f;
	}
	else if ((cp & 0xf0 ) == 0xe0)
	{
		charlen = 3;
		mask = 0x0f;
	}
	else if ((cp & 0xf8 ) == 0xf0)
	{
		charlen = 4;
		mask = 0x07;
	}
	else if ((cp & 0xfc ) == 0xf8)
	{
		charlen = 5;
		mask = 0x03;
	}
	else if ((cp & 0xfe ) == 0xfc)
	{
		charlen = 6;
		mask = 0x01;
	}
	else
		return undefined_wcharmap;

	// Only handle 16-bit chars for now
	if (charlen <= 3)
	{
		// Reset char
		wchar_t wc = 0;

		while(charlen-- != 0)
		{
			// Convert the first byte
			cp = *c++;
			wc << 6;
			wc |= (cp & mask);

			// Reset mask for remaining bytes
			mask = 0x3f;
		}
		
		return wc;
	}
	else
	{
		while(charlen-- != 0)
			c++;
	}

	return undefined_wcharmap;
}

int CIMAPUTF7::w_2_c(wchar_t wc, char* out)
{
	int ctr = 0;

	if (wc < 0x0080)
	{
		if (wc == '&')
		{
			// Turn off encoding run
			ctr += DoneEncoding(out);

			// Output encoded &-
			*out++ = '&';
			*out++ = '-';
			ctr += 2;

			return ctr;
		}
		else if (cSafemUTF7[wc & 0x00FF])
		{
			// Turn off encoding run
			ctr += DoneEncoding(out);

			// Output unencoded character
			unsigned char c = wc;
			*out++ = c;
			ctr++;

			return ctr;
		}
	}
	
	// If we get here we have an unsafe character that must be encoded
	
	// Turn on encoding if not already on
	else if (wc < 0x0800)
	{
		// Write 2 to buffer
		unsigned char c = 0xc0 | (wc >> 6);
		*out++ = c;

		c = 0x80 | (wc & 0x3f);
		*out++ = c;

		return 2;
	}
	else // if (wc < 0x10000)
	{
		// Write 3 to buffer
		unsigned char c = 0xe0 | (wc >> 12);
		*out++ = c;

		c = 0x80 | ((wc >> 6) & 0x3f);
		*out++ = c;

		c = 0x80 | (wc & 0x3f);
		*out++ = c;

		return 3;
	}
	return -1;
}

int CIMAPUTF7::DoneEncoding(char*& out)
{
	if (mEncoding)
	{
		mEncoding = false;
		*out++ = '-';
		return 1;
	}
	else
		return 0;
}


// Start state in default (ASCII)
void CIMAPUTF7::init_w_2_c(std::ostream& out)
{
	mEncoding = false;
}

// Reset encoding state back to default (ASCII)
void CIMAPUTF7::finish_w_2_c(std::ostream& out)
{
	if (mEncoding)
	{
		mEncoding = false;
		out.put('-');
	}
}

