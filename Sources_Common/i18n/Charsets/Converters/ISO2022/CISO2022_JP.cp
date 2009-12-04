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


// Source for CISO2022_JP class

#include "CISO2022_JP.h"

using namespace i18n;

const char ESC = 0x1B;
const char ESC_ROMAN = '(';
const char ESC_ASCII = 'B';
const char ESC_JIS0201 = 'J';
const char ESC_JIS = '$';
const char ESC_JIS0208_1978 = '@';
const char ESC_JIS0208_1983 = 'B';

wchar_t CISO2022_JP::c_2_w(const unsigned char*& c)
{
	int count = 0;
	for (;;)
	{
		// Look for shift triggered by escape
		if (*c == ESC)
		{
			if (c[1] == ESC_ROMAN)
			{
				if (c[2] == ESC_ASCII)
				{
					mState = eASCII;
					c += 3;
					continue;
				}
				if (c[2] == ESC_JIS0201)
				{
					mState = eJIS_0201;
					c += 3;
					if (!*c)
						return undefined_wcharmap;
					continue;
				}
				
				// Always bump by at least one char to prevent inifinite loop
				c++;
				return undefined_wcharmap;
			}
			if (c[1] == ESC_JIS)
			{
				if (c[2] == ESC_JIS0208_1978 || c[2] == ESC_JIS0208_1983)
				{
					// Treat JIS X 0208-1978 and JIS X 0208-1983 the same
					mState = eJIS_0208;
					c += 3;
					if (!*c)
						return undefined_wcharmap;
					continue;
				}
				
				// Always bump by at least one char to prevent inifinite loop
				c++;
				return undefined_wcharmap;
			}
			
			// Always bump by at least one char to prevent inifinite loop
			c++;
			return undefined_wcharmap;
		}
		break;
	}

	switch(mState)
	{
	case eASCII:
		return mASCII_Converter.c_2_w(c);

	case eJIS_0201:
		return mJIS0201_Converter.c_2_w(c);

	case eJIS_0208:
		if (c[0] < 0x80 && c[1] < 0x80)
			return mJIS0208_Converter.c_2_w(c);
		break;
		
	default:;
	}

	// Always bump by at least one char to prevent inifinite loop
	c++;
	return undefined_wcharmap;
}

int CISO2022_JP::w_2_c(wchar_t wc, char* out)
{
	int result = 0;

	// Try ASCII first
	if (wc < 0x80)
	{
		// Check we are in the right shift mode
		if (mState != eASCII)
		{
			mState = eASCII;
			result += changestate(out);
		}
		
		// Write it out
		result += mASCII_Converter.w_2_c(wc, out);
		return result;
	}
	
	// Try JIS_0201 next
	char buf[2];
	int len = mJIS0201_Converter.w_2_c(wc, &buf[0]);
	if (len > 0)
	{
		// Check we are in the right shift mode
		if (mState != eJIS_0201)
		{
			mState = eJIS_0201;
			result += changestate(out);
		}
		
		// Write it out
		for(int i = 0; i < len; i++)
		{
			*out++ = buf[i];
			result++;
		}

		return result;
	}

	// Try JIS_0208 next
	len = mJIS0208_Converter.w_2_c(wc, &buf[0]);
	if (len > 0)
	{
		// Check we are in the right shift mode
		if (mState != eJIS_0208)
		{
			mState = eJIS_0208;
			result += changestate(out);
		}
		
		// Write it out
		for(int i = 0; i < len; i++)
		{
			*out++ = buf[i];
			result++;
		}

		return result;
	}

	return -1;
}

// Start state in default (ASCII)
void CISO2022_JP::init_w_2_c(std::ostream& out)
{
	mState = eASCII;
}

// Reset encoding state back to default (ASCII)
void CISO2022_JP::finish_w_2_c(std::ostream& out)
{
	if (mState != eASCII)
	{
		mState = eASCII;
		out.put(ESC);
		out.put(ESC_ROMAN);
		out.put(ESC_ASCII);
	}
}

// Change shift encoding to current state
int CISO2022_JP::changestate(char*& out)
{
	switch(mState)
	{
	case eASCII:
		*out++ = ESC;
		*out++ = ESC_ROMAN;
		*out++ = ESC_ASCII;
		break;
	case eJIS_0201:
		*out++ = ESC;
		*out++ = ESC_ROMAN;
		*out++ = ESC_JIS0201;
		break;
	case eJIS_0208:
		*out++ = ESC;
		*out++ = ESC_JIS;
		*out++ = ESC_JIS0208_1983;
		break;
	}
	
	return 3;
}
