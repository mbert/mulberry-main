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


// Source for CISO2022_KR class

#include "CISO2022_KR.h"

using namespace i18n;

const char SHIFT_SI = 0x0F;
const char SHIFT_SO = 0x0E;

const char ESC = 0x1B;
const char DESIGNATION = '$';

const char SO_DESIGNATE = ')';

const char SO_KSC5601_1987 = 'C';

wchar_t CISO2022_KR::c_2_w(const unsigned char*& c)
{
	int count = 0;
	for (;;)
	{
		// Look for shift triggered by escape
		if (*c == ESC)
		{
			if (c[1] == DESIGNATION)
			{
				if (c[2] == SO_DESIGNATE)
				{
					if (c[3] == SO_KSC5601_1987)
						mState2 = eKSC_5601_1987;
					else
					{
						// Always bump by at least one char to prevent inifinite loop
						c++;
						return undefined_wcharmap;
					}
					c += 4;
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
		else if (*c == SHIFT_SI)
		{
			mState1 = eASCII;
			c++;
			continue;
		}
		else if (*c == SHIFT_SO)
		{
			mState1 = eTwoByte;
			c++;
			if (!*c)
				return undefined_wcharmap;
			continue;
		}
		break;
	}

	switch(mState1)
	{
	case eASCII:
		return mASCII_Converter.c_2_w(c);

	case eTwoByte:
		if (*c < 0x80 && c[1] < 0x80)
		{
			switch(mState2)
			{
			case eNone:
				break;
			case eKSC_5601_1987:
			{
				wchar_t ret = mKSC5601_1987_Converter.c_2_w(c);
				
				// Test for line end and reset states
				if (ret == 0x000A || ret == 0x000D)
					mState2 = eNone;
				return ret;
			}
			default:;
			}
		}
		break;
	default:;
	}

	// Always bump by at least one char to prevent inifinite loop
	c++;
	return undefined_wcharmap;
}

int CISO2022_KR::w_2_c(wchar_t wc, char* out)
{
	int result = 0;

	// Try ASCII first
	if (wc < 0x80)
	{
		// Check we are in the right shift mode
		if (mState1 != eASCII)
		{
			mState1 = eASCII;
			result += changestate1(out);
		}
		
		// Reset state2 at line ends
		if ((wc == 0x000A) || (wc == 0x000D))
			mState2 = eNone;

		// Write it out
		result += mASCII_Converter.w_2_c(wc, out);
		return result;
	}
	
	// Try KSC_5601_1987 next
	char buf[2];
	int len = mKSC5601_1987_Converter.w_2_c(wc, &buf[0]);
	if (len > 0)
	{
		// Check we are in the right designation/shift mode
		if (mState2 != eKSC_5601_1987)
		{
			mState2 = eKSC_5601_1987;
			result += changestate2(out);
		}
		if (mState1 != eTwoByte)
		{
			mState1 = eTwoByte;
			result += changestate1(out);
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
void CISO2022_KR::init_w_2_c(std::ostream& out)
{
	mState1 = eASCII;
	mState2 = eNone;
}

// Reset encoding state back to default (ASCII)
void CISO2022_KR::finish_w_2_c(std::ostream& out)
{
	if (mState1 != eASCII)
	{
		mState1 = eASCII;
		mState2 = eNone;
		out.put(SHIFT_SI);
	}
}

// Change shift encoding to current state
int CISO2022_KR::changestate1(char*& out)
{
	switch(mState1)
	{
	case eASCII:
		*out++ = SHIFT_SI;
		break;
	case eTwoByte:
		*out++ = SHIFT_SO;
		break;
	}
	
	return 1;
}

// Change shift encoding to current state
int CISO2022_KR::changestate2(char*& out)
{
	switch(mState2)
	{
	case eNone:
		return 0;
	case eKSC_5601_1987:
		*out++ = ESC;
		*out++ = DESIGNATION;
		*out++ = SO_DESIGNATE;
		*out++ = SO_KSC5601_1987;
		break;
	}
	
	return 4;
}
