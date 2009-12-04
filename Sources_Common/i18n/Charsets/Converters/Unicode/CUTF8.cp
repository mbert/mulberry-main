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


// Source for CUTF8 class

#include "CUTF8.h"

using namespace i18n;

wchar_t CUTF8::c_2_w(const unsigned char*& c)
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
	{
		c++;
		return undefined_wcharmap;
	}

	// Only handle 16-bit chars for now
	if (charlen <= 3)
	{
		// Reset char
		wchar_t wc = 0;

		while(charlen-- != 0)
		{
			// Convert the first byte
			cp = *c++;
			wc <<= 6;
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

int CUTF8::w_2_c(wchar_t wc, char* out)
{
	if (wc < 0x0080)
	{
		// Write 1 to buffer
		unsigned char c = wc;
		*out++ = c;

		return 1;
	}
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
