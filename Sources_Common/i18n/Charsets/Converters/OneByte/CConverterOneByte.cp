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


// Source for CConverterOneByte class

#include "CConverterOneByte.h"

#include <string.h>

using namespace i18n;

static char undefined_charmap = '?';		// Undefined mapping character
static wchar_t undefined_wcharmap = '\0?';	// Undefined mapping character

CConverterOneByte::CConverterOneByte(const wchar_t* map)
{
	m_c_2_w_map = map;
	m_w_2_c_map = NULL;
	max_w_2_c = 0x007F;
	generate_map();
}

CConverterOneByte::~CConverterOneByte()
{
	// This is the one we allocated
	delete m_w_2_c_map;
	m_w_2_c_map = NULL;
}

void CConverterOneByte::generate_map()
{
	// Now fill 7bit map and find largest wchar_t used
	for(int i = 0; i < 128; i++)
	{
		wchar_t wc = m_c_2_w_map[i];
		if ((wc > max_w_2_c) && (wc != 0xFFFD))
			max_w_2_c = wc;
	}

	// Create empty map
	unsigned char* bigmap = new unsigned char[max_w_2_c + 1];
	m_w_2_c_map = bigmap;
	::memset((void*) bigmap, 0, max_w_2_c);
	
	// Now fill 7bit map and inverse map
	for(int i = 0; i < 128; i++)
	{
		bigmap[i] = i;
		wchar_t wc = m_c_2_w_map[i];
		if (wc && (wc != 0xFFFD))
			bigmap[wc] = i + 128;
	}
}

wchar_t CConverterOneByte::c_2_w(const unsigned char*& c)
{
	unsigned char cp = *c++;
	if (cp < 0x80)
		return cp;
	else
		return m_c_2_w_map[cp & 0x7f];
}

int CConverterOneByte::w_2_c(wchar_t wc, char* out)
{
	// Look up in reverse table up to maximum reverse index
	if (wc <= max_w_2_c)
	{
		char c = m_w_2_c_map[wc];
		if ((c != 0) && wc)
		{
			*out++ = c;
			return 1;
		}
	}

	return -1;
}
