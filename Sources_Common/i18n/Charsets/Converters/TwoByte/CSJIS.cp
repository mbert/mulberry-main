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


// Source for CSJIS class

#include "CSJIS.h"

using namespace i18n;

wchar_t CSJIS::c_2_w(const unsigned char*& c)
{
	// Check for ASCII/JIS_0201
	if ((*c < 0x80) || (*c >= 0xA1 && *c <= 0xDF))
		return mJIS0201_Converter.c_2_w(c);
	else
	{
		unsigned char s1 = *c++;
		if (((s1 >= 0x81) && (s1 <= 0x9F)) || ((s1 >= 0xE0) && (s1 <= 0xEA)))
		{
			// Check for JIS_0208
			if (!*c)
				return undefined_wcharmap;
			unsigned char s2 = *c++;
			if (((s2 >= 0x40) && (s2 <= 0x7E)) || ((s2 >= 0x80) && (s2 <= 0xFC)))
			{
				unsigned char t1 = (s1 < 0xE0 ? s1 - 0x81 : s1 - 0xC1);
				unsigned char t2 = (s2 < 0x80 ? s2 - 0x40 : s2 - 0x41);
				unsigned char buf[3];
				buf[0] = 2 * t1 + (t2 < 0x5E ? 0 : 1) + 0x21;
				buf[1] = (t2 < 0x5E ? t2 : t2 - 0x5E) + 0x21;
				buf[2] = 0;
				const unsigned char* bufp = &buf[0];
				return mJIS0208_Converter.c_2_w(bufp);
			}
		}
		else if ((s1 >= 0xF0) && (s1 <= 0xF9))
		{
			// User-defined range
			if (!*c)
				return undefined_wcharmap;
			unsigned char s2 = *c++;
			if (((s2 >= 0x40) && (s2 <= 0x7E)) || ((s2 >= 0x80) && (s2 <= 0xFC)))
				return 0xE000 + 188 * (s1 - 0xF0) + (s2 < 0x80 ? s2 - 0x40 : s2 - 0x41);
		}
	}

	return undefined_wcharmap;
}

int CSJIS::w_2_c(wchar_t wc, char* out)
{
	// Try JIS_0201 next
	int len = mJIS0201_Converter.w_2_c(wc, out);
	if (len >= 0)
		return len;

	// Try JIS_0208 next
	char cbuf[2];
	if (mJIS0208_Converter.w_2_c(wc, cbuf) == 2)
	{
		if ((cbuf[0] >= 0x21 && cbuf[0] <= 0x74) && (cbuf[1] >= 0x21 && cbuf[1] <= 0x7E))
		{
			unsigned char t1 = (cbuf[0] - 0x21) >> 1;
			unsigned char t2 = (((cbuf[0] - 0x21) & 1) ? 0x5E : 0) + (cbuf[1] - 0x21);
			*out++ = (t1 < 0x1F ? t1 + 0x81 : t1 + 0xC1);
			*out = (t2 < 0x3F ? t2 + 0x40 : t2 + 0x41);
			return 2;
		}
	}

	return -1;
}
