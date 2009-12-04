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


// Source for CIDNA class

#include "CIDNA.h"

#include "CPunyCode.h"

using namespace idn;

cdstrmap CIDNA::sCache;

// Encode			:	IDNA encode
//
// str				:	data to encode (input format is utf8)

void CIDNA::Encode(cdstring& str)
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
	
	cdstring result;

	// Tokenise on . characters
	const char* start = str.c_str();
	const char* stop = start;
	non_ascii = false;
	while(*stop)
	{
		if (*stop == '.')
		{
			// See if encoding is required
			if (non_ascii)
			{
				cdstring temp(start, stop - start);
				CPunyCode::Encode(temp);
				result += temp;
				result += ".";
			}
			else
				result.append(start, stop + 1 - start);
			
			// Reset state
			non_ascii = false;
			start = ++stop;
		}
		else
		{
			if (*(unsigned char*)stop > 0x7F)
				non_ascii = true;
			stop++;
		}
	}
	
	// Do whatever is left over

	// See if encoding is required
	if (non_ascii)
	{
		cdstring temp(start);
		CPunyCode::Encode(temp);
		result += temp;
	}
	else
		result.append(start);
	
	str = result;
}

// Decode			:	IDNS decode
//
// str				:	c-str to decode (output is utf8)

const cdstring& CIDNA::Decode(const cdstring& str)
{
	// Check for prefix
	if (str.find("xn--") == cdstring::npos)
		return str;
	
	// Look in cache
	cdstrmap::const_iterator found = sCache.find(str);
	if (found != sCache.end())
		return (*found).second;
	
	// Do decode and add to cache
	cdstring result;

	// Tokenise on . characters
	const char* start = str.c_str();
	const char* stop = start;
	while(*stop)
	{
		if (*stop == '.')
		{
			// See if encoding is required
			if (::strncmp(start, "xn--", 4) == 0)
			{
				cdstring temp(start, stop - start);
				CPunyCode::Decode(temp);
				result += temp;
				result += ".";
			}
			else
				result.append(start, stop + 1 - start);
			
			// Reset state
			start = ++stop;
		}
		else
			stop++;
	}
	
	// Do whatever is left over

	// See if encoding is required
	if (::strncmp(start, "xn--", 4) == 0)
	{
		cdstring temp(start);
		CPunyCode::Decode(temp);
		result += temp;
	}
	else
		result.append(start);
	
	// Add to cache
	sCache.insert(cdstrmap::value_type(str, result));
	
	// Return value from cache
	return (*sCache.find(str)).second;
}
