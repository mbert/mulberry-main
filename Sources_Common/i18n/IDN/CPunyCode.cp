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


// Source for CPunyCode class

#include "CPunyCode.h"

#include "cdustring.h"

#include <strstream>

using namespace idn;

// base36 tables
static char basis_36[] =
   "abcdefghijklmnopqrstuvwxyz0123456789";
static signed char index_36[128] =
{
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    26,27,28,29, 30,31,32,33, 34,35,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1
};
#define CHAR36(c)  (((c) < 0 || (c) > 127) ? -1 : index_36[(c)])

// punycode constants from RFC3492

const int cBase			= 36;
const int cTmin			= 1;
const int cTmax			= 26;
const int cSkew			= 38;
const int cDamp			= 700;
const int cInitialBias 	= 72;
const unsigned long cInitialN 		= 128;

const char* cPrefix					= "xn--";
const char cDelimiter				= '-';

// Encode			:	punycode encode
//
// str				:	data to encode (input format is utf8)

void CPunyCode::Encode(cdstring& str)
{
	// See whether any non-ascii chars exist
	bool non_ascii = false;
	const char* p = str.c_str();
	while(*p)
	{
		if (*(unsigned char*)p++ > 0x7F)
		{
			non_ascii = true;
			break;
		}
	}
	
	// Just exit if nothing to do
	if (!non_ascii)
		return;
	
	// Get unicode version of string
	cdustring ustr(str);
	int ulen = ustr.length();

	// Write the prefix
	ostrstream sout;
	sout << cPrefix;
	
	unsigned long n = cInitialN;
	unsigned long delta = 0;
	int bias = cInitialBias;

	// Write out each basic char
	int b = 0;
	const unichar_t* up = ustr.c_str();
	while(*up)
	{
		if (*up < 0x0080)
		{
			b++;
			sout.put(*up);
		}
		up++;
	}
	
	// Write out delimiter if some basic chars found
	if (b > 0)
		sout.put(cDelimiter);
	
	int h = b;
	
	while(h < ulen)
	{
		// Find smallest code point >= n and its pos
		unsigned long m = 0xFFFF;
		int mpos = -1;
		for(unsigned long ctr = 0; ctr < ulen; ctr++)
		{
			if ((ustr[ctr] >= n) && (ustr[ctr] < m))
			{
				m = ustr[ctr];
				mpos = ctr;
			}
		}
		
		delta += (m - n) * (h + 1);
		n = m;
		
		// Encode all instances of m in the input string
		for(unsigned long ctr = 0; ctr < ulen; ctr++)
		{
			if (ustr[ctr] < n)
				delta++;
			else if (ustr[ctr] == n)
			{
				unsigned long q = delta;
				for(int k = cBase - bias; true; k += cBase)
				{
					int t = (k <= cTmin) ? cTmin : ((k >= cTmax) ? cTmax : k);
					if (q < t)
						break;
					unsigned long digit = t + ((q - t) % (cBase - t));
					if (digit < cBase)
						sout.put(basis_36[digit]);
					q = (q - t) / (cBase - t);
				}
				if (q < cBase)
					sout.put(basis_36[q]);
				bias = Adapt(delta, h + 1, h == b);
				delta = 0;
				h++;
			}
		}
		
		delta++;
		n++;
	}
	
	sout << ends;
	str.steal(sout.str());
}

// Decode			:	punycode decode
//
// str				:	c-str to decode (output is utf8)

void CPunyCode::Decode(cdstring& str)
{
	// Just exit if prefix is not present
	if (!str.compare_start(cPrefix))
		return;
	
	// Grab chars up to delimiter
	cdstring::size_type pos = str.rfind(cDelimiter);
	
	// Must have valid delimiter
	if (pos < 4)
		return;
	
	// Split string into basic and encoded parts
	cdstring basic;
	cdstring encoded;
	
	if (pos == cdstring::npos)
	{
		// Use entire string as encoded
		encoded.assign(str, 4);
	}
	else
	{
		// Split at pos
		basic.assign(str, 4, pos - 4);
		encoded.assign(str, pos + 1);
	}

	cdustring ubasic(basic);
	
	int n = cInitialN;
	int i = 0;
	int bias = cInitialBias;
	const char* input = encoded.c_str();
	while(*input)
	{
		int oldi = i;
		int w = 1;
		int len1 = ubasic.length() + 1;

		if (!ReadChar(&input, bias, i))
			return;
		
		bias = Adapt(i - oldi, len1, oldi == 0);
		
		// Get <n, i> value
		n += i / len1;
		i = i % len1;
		
		// Do char insert
		cdustring temp(ubasic, 0, i);
		temp.append((unichar_t) n);
		temp.append(ubasic, i, cdustring::npos);
		ubasic = temp;
		
		i++;
	}
	
	str = ubasic.ToUTF8();
}

bool CPunyCode::ReadChar(const char** input, int bias, int& output)
{
	int i = 0;
	int w = 1;
	for(int k = cBase - bias; true; k += cBase)
	{
		int digit = CHAR36(**input);
		(*input)++;
		if (digit == -1)
			return false;
		
		i += digit * w;

		int t = (k < cTmin) ? cTmin : ((k > cTmax) ? cTmax : k);
		
		if (digit < t)
			break;
		
		w *= (cBase - t);
	}
	
	output += i;
	return true;
}

int CPunyCode::Adapt(int delta, int numpoints, bool firsttime)
{
	delta /= firsttime ? cDamp : 2;
	
	delta += delta / numpoints;
	
	int k = 0;
	while(delta > ((cBase - cTmin) * cTmax) / 2)
	{
		delta /= (cBase - cTmin);
		k += cBase;
	}
	
	return k + ((cBase - cTmin + 1) * delta) / (delta + cSkew);
}
