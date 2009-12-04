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

	/*
 * Copyright (c) 2006 Cyrus Daboo.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are not permitted
 * without specific prior written permission from Cyrus Daboo.
 * This software is provided "as is" without express or implied warranty.
 */

// Source for CISO2022_CN class

#include "CISO2022_CN.h"

using namespace i18n;

const char SHIFT_SI = 0x0F;
const char SHIFT_SO = 0x0E;

const char ESC = 0x1B;
const char DESIGNATION = '$';
const char SO_SHIFT = 'N';

const char SO_DESIGNATE = ')';
const char SS2_DESIGNATE = '*';

const char SO_GB2312 = 'A';
const char SO_CNS11643_1 = 'G';
const char SO_CNS11643_2 = 'H';

wchar_t CISO2022_CN::c_2_w(const unsigned char*& c)
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
					if (c[3] == SO_GB2312)
						mState2 = eGB2312;
					else if (c[3] == SO_CNS11643_1)
						mState2 = eCNS11643_1;
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
				if (c[2] == SS2_DESIGNATE)
				{
					if (c[3] == SO_CNS11643_2)
						mState3 = eCNS11643_2;
					c += 4;
					if (!*c)
						return undefined_wcharmap;
					continue;
				}
				
				// Always bump by at least one char to prevent inifinite loop
				c++;
				return undefined_wcharmap;
			}
			else if (c[1] == SO_SHIFT)
			{
				switch(mState3)
				{
				case eNone3:
					// Always bump by at least one char to prevent inifinite loop
					c++;
					return undefined_wcharmap;
				case eCNS11643_2:
					c += 2;
					if (*c < 0x80 && c[1] < 0x80)
					{
						// We don't handle this right now
						//return mCNS11643_2_Converter.c_2_w(c);
						c += 2;
						return undefined_wcharmap;
					}
					else
						return undefined_wcharmap;
				default:;
				}
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
		{
			wchar_t ret = mASCII_Converter.c_2_w(c);
			
			// Test for line end and reset states
			if (ret == 0x000A || ret == 0x000D)
			{
				mState2 = eNone;
				mState3 = eNone3;
			}
			return ret;
		}

	case eTwoByte:
		if (*c < 0x80 && c[1] < 0x80)
		{
			switch(mState2)
			{
			case eNone:
				break;
			case eGB2312:
				return mGB2312_Converter.c_2_w(c);
			case eCNS11643_1:
				// We don't handle this right now
				//return mCNS11643_1_Converter.c_2_w(c);
				c += 2;
				return undefined_wcharmap;
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

int CISO2022_CN::w_2_c(wchar_t wc, char* out)
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
		
		// Test for line end and reset states
		if (wc == 0x000A || wc == 0x000D)
		{
			mState2 = eNone;
			mState3 = eNone3;
		}

		// Write it out
		result += mASCII_Converter.w_2_c(wc, out);
		return result;
	}
	
	// Try GB2312 next
	char buf[2];
	int len = mGB2312_Converter.w_2_c(wc, &buf[0]);
	if (len > 0)
	{
		// Check we are in the right designation/shift mode
		if (mState2 != eGB2312)
		{
			mState2 = eGB2312;
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
void CISO2022_CN::init_w_2_c(std::ostream& out)
{
	mState1 = eASCII;
	mState2 = eNone;
	mState3 = eNone3;
}

// Reset encoding state back to default (ASCII)
void CISO2022_CN::finish_w_2_c(std::ostream& out)
{
	if (mState1 != eASCII)
	{
		mState1 = eASCII;
		mState2 = eNone;
		mState3 = eNone3;
		out.put(SHIFT_SI);
	}
}

// Change shift encoding to current state
int CISO2022_CN::changestate1(char*& out)
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
int CISO2022_CN::changestate2(char*& out)
{
	switch(mState2)
	{
	case eNone:
		return 0;
	case eGB2312:
		*out++ = ESC;
		*out++ = DESIGNATION;
		*out++ = SO_DESIGNATE;
		*out++ = SO_GB2312;
		break;
	case eCNS11643_1:
		*out++ = ESC;
		*out++ = DESIGNATION;
		*out++ = SO_DESIGNATE;
		*out++ = SO_CNS11643_1;
		break;
	}
	
	return 4;
}

// Change shift encoding to current state
int CISO2022_CN::changestate3(char*& out)
{
	switch(mState3)
	{
	case eNone3:
		return 0;
	case eCNS11643_2:
		*out++ = ESC;
		*out++ = DESIGNATION;
		*out++ = SS2_DESIGNATE;
		*out++ = SO_CNS11643_2;
		break;
	}
	
	return 4;
}
