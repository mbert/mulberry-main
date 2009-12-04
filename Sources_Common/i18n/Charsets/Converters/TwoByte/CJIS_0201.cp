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


// Source for CJIS_0201 class

#include "CJIS_0201.h"

using namespace i18n;

wchar_t CJIS_0201::c_2_w(const unsigned char*& c)
{
	char cp = *c++;
	if (cp < 0x80)
	{
		if (cp == 0x5C)
			return 0x00A5;
		else if (cp == 0x7E)
			return 0x203E;
		else
			return cp;
	}
	else
	{
		if (cp >= 0xA1 && cp < 0xE0)
			return cp + 0xFEC0;
	}
	
	return undefined_wcharmap;
}

int CJIS_0201::w_2_c(wchar_t wc, char* out)
{
	if ((wc < 0x0080) && !(wc == 0x005C || wc == 0x007E))
		*out++ = wc;
	else if (wc == 0x00A5)
		*out++ = 0x5C;
	else if (wc == 0x203E)
		*out++ = 0x7E;
	else if (wc >= 0xFF61 && wc < 0xFFA0)
		*out++ = wc - 0xFEC0;
	else
		return -1;
	
	return 1;
}
