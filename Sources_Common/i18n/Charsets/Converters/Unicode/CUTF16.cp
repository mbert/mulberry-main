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


// Source for CUTF16 class

#include "CUTF16.h"

using namespace i18n;

wchar_t CUTF16::c_2_w(const unsigned char*& c)
{
	unsigned char c1 = *c++;
	unsigned char c2 = *c++;
	wchar_t	wc = mBigEndian ? ((c1 << 8) | c2) : ((c2 << 8) | c1);

	// Look for endian switch
	if (wc == 0xfeff)
	{
		// Switch endianess
		mBigEndian = true;
	}
	else if (wc == 0xfffe)
	{
		// Switch endianess
		mBigEndian = false;
	}
	else if ((wc >= 0xd800) && (wc < 0xdc00))
	{
		// Have lead word - we do not handle these as they are more than 16-bits

		// Ignore second word
		c++;
		c++;

		return undefined_wcharmap;
	}
	else if ((wc >= 0xdc00) && (wc < 0xe000))
	{
		// Have illegal second word
		return undefined_wcharmap;
	}
	
	return wc;
}

int CUTF16::w_2_c(wchar_t wc, char* out)
{
	// Just write out two bytes (always big endian)
	if ((wc != 0xfffe) && !((wc >= 0xd800) && (wc < 0xe000)))
	{
		*out++ = (wc & 0xFF00) >> 8;
		*out++ = (wc & 0x00FF);
		return 2;
	}
	else
		return -1;
}
