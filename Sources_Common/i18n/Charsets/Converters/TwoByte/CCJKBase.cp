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


// Source for CCJKBase class

#include "CCJKBase.h"

using namespace i18n;

wchar_t CCJKBase::c_2_w(const unsigned char*& c)
{
	unsigned char c1 = *c++;
	if (c1 >= mOffset1 && c1 <= mMax1)
	{
		unsigned char c2 = *c;
		if (c2 >= mOffset2 && c2 <= mMax2)
		{
			c++;
			return tabledata(c1 - mOffset1, c2 - mOffset2);
		}
	}

	return undefined_wcharmap;
}

int CCJKBase::w_2_c(wchar_t wc, char* out)
{
	// Initialise tables first time through
	if (!get_page_map(0))
		init_tables();

	for(size_t i = 0; i < mNumPages; i++)
	{
		if ((wc >= get_page_bounds(i, 0)) && (wc < get_page_bounds(i, 1)))
		{
			unsigned long index = 2 * (wc - get_page_bounds(i, 0));
			*out++ = get_page_map(i)[index];
			*out = get_page_map(i)[index + 1];
			return 2;
		}
	}

	return -1;
}

void CCJKBase::init_tables()
{
	// Intitialise each table
	for(size_t i = 0; i < mNumPages; i++)
		set_page_map(i, new char[2 * (get_page_bounds(i, 1) - get_page_bounds(i, 0))]);

	// Loop over each jis->unicode map point and add to appropriate table
	for(unsigned long i = 0; i < mSize1; i++)
	{
		for(unsigned long j = 0; j < mSize2; j++)
		{
			// Detrmine the unicode char
			wchar_t wc = tabledata(i, j);
			
			// Get the jis char
			char char1 = i + mOffset1;
			char char2 = j + mOffset2;

			// Fill in unicode map entry
			for(size_t i = 0; i < mNumPages; i++)
			{
				if ((wc >= get_page_bounds(i, 0)) && (wc < get_page_bounds(i, 1)))
				{
					unsigned long index = 2 * (wc - get_page_bounds(i, 0));
					const_cast<char*>(get_page_map(i))[index] = char1;
					const_cast<char*>(get_page_map(i))[index + 1] = char2;
				}
			}
		}
	}
}
