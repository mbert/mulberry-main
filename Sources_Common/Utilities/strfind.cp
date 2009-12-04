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


// strfind : find text in a string using various options

#include "strfind.h"

#include <ctype.h>
#include <stddef.h>

const char* strmatch(const char* source, const char* end, const char* start1, const char* match1, EFindMode mode);

// strfind : finds a string in another string using different modes
// source : entire string to search
// len    : length of text to search
// sel_start : position in source at which to start
// match  : entire string to find
// mode   : mode of operation
const char* strfind(const char* source, unsigned long len, unsigned long sel_start,
					const char* match, EFindMode mode)
{
	const char* start = source + sel_start;
	const char* end = source + len - 1;

	// Adjust start if going backwards
	if (mode & eBackwards)
	{
		// Bump down one char
		start--;

		// Check not past beginning of entire text
		if (start < source)
		{
			// Wrap => go to last character of entire text
			if (mode & eWrap)
				start = end;
			else
				// No wrap - cannot match
				return NULL;
		}
	}

	const char* s1 = start;
	const char* p1 = match;
	unsigned char firstc, c1;
	bool back_to_start = false;

	if (!(firstc = static_cast<unsigned char>(*p1++)))
		return start;

	// Check case sensitivity
	if (!(mode & eCaseSensitive))
		firstc = ::tolower(firstc);

	while(!back_to_start)
	{
		// Look for wrap at end
		if (s1 > end)
		{
			if (mode & eWrap)
			{
				// Reset to start
				s1 = source;

				// Do not allow wrap to restart search
				if (s1 == start)
					break;
			}
			else
				break;
		}

		// Look for wrap at start of entire text
		else if (s1 < source)
		{
			if (mode & eWrap)
			{
				// Reset to end
				s1 = end;

				// Do not allow wrap to restart search
				if (s1 == start)
					break;
			}
			else
				break;
		}

		// Get next character to test (take direction into account
		if (mode & eBackwards)
			c1 = static_cast<unsigned char>(*s1--);
		else
			c1 = static_cast<unsigned char>(*s1++);

		// Do not allow wrap to restart search
		if ((mode & eWrap) && (s1 == start))
			back_to_start = true;

		// Check case sensitivity
		if (!(mode & eCaseSensitive))
			c1 = ::tolower(c1);

		// Look for first char match
		if (c1 == firstc)
		{
			// Try to match remainder of string
			// Must adjust pointer for backwards direction
			const char* result = ::strmatch(source, end, (mode & eBackwards) ? s1 + 2 : s1, p1, mode);
			if (result)
				return result;
		}
	}

	return NULL;
}

// strmatch : matches one string with another using different modes
// source : start of entire find string
// end    : last character in find string
// start1 : position to start match (one character beyond first matching character)
// match1 : one character into matching string
// mode   : mode of operation
const char* strmatch(const char* source, const char* end, const char* start1, const char* match1, EFindMode mode)
{
	const char* p = start1;
	const char* q = match1;
	unsigned char c1;
	unsigned char c2;

	// Loop doing comparison of strings
	do
	{
		c1 = static_cast<unsigned char>(*p++);
		c2 = static_cast<unsigned char>(*q++);

		// Remove case if case insensitive
		if (!(mode & eCaseSensitive))
		{
			c1 = ::tolower(c1);
			c2 = ::tolower(c2);
		}
	}
	while ((c1 == c2) && (p <= end));

	// If at end of match string then we have a match!
	// Or if end of find string and end of match string have a match!
	if (!c2 || ((p > end) && !*q))
	{
		// Check for entire word
		if (mode & eEntireWord)
		{
			// Check start of word
			bool word = false;
			if (start1 == source + 1)
				// Start of string => word
				word = true;
			else
			{
				// Check for word delimiter
				char cc = *(start1 - 2);
				word = isspace(cc) || ispunct(cc);
			}

			// Check end of word
			if (word)
			{
				if (p > end)
					// End of string => word
					word = true;
				else
					// Check for word delimiter
					word = isspace(c1) || ispunct(c1);

				// Return if entire word
				if (word)
					return start1 - 1;
			}
		}
		else
			return start1 - 1;
	}

	// No match
	return NULL;
}
