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


// Source for CUCS4 class

#include "CUCS4.h"

using namespace i18n;

wchar_t CUCS4::c_2_w(const unsigned char*& c)
{
	// Just write out two bytes
	unsigned char c1 = *c++;
	unsigned char c2 = *c++;
	unsigned char c3 = *c++;
	unsigned char c4 = *c++;
	
	// Only handle 16-bit chars
	if ((mBigEndian && ((c1 != 0) || (c2 != 0))) || (!mBigEndian && ((c3 != 0) || (c4 != 0))))
		return undefined_wcharmap;
	
	wchar_t	wc = mBigEndian ? ((c3 << 8) | c4) : ((c2 << 8) | c1);

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
	
	return wc;
}

int CUCS4::w_2_c(wchar_t wc, char* out)
{
	// Just write out four bytes (always big endian)
	if (wc != 0xfffe)
	{
		*out++ = 0;
		*out++ = 0;
		*out++ = (wc & 0xFF00) >> 8;
		*out++ = (wc & 0x00FF);
		return 4;
	}
	else
		return -1;
}
