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


// ustrfind : find text in a string using various options

#include "ustrfind.h"

#include <ctype.h>
#include <stddef.h>

const unichar_t* ustrmatch(const unichar_t* source, const unichar_t* end, const unichar_t* start1, const unichar_t* match1, EFindMode mode);

// strfind : finds a string in another string using different modes
// source : entire string to search
// len    : length of text to search
// sel_start : position in source at which to start
// match  : entire string to find
// mode   : mode of operation
const unichar_t* ustrfind(const unichar_t* source, unsigned long len, unsigned long sel_start,
							const unichar_t* match, EFindMode mode)
{
	const unichar_t* start = source + sel_start;
	const unichar_t* end = source + len - 1;

	// Adjust start if going backwards
	if (mode & eBackwards)
	{
		// Bump down one unichar_t
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

	const unichar_t* s1 = start;
	const unichar_t* p1 = match;
	unichar_t firstc, c1;
	bool back_to_start = false;

	if (!(firstc = *p1++))
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
			c1 = *s1--;
		else
			c1 = *s1++;

		// Do not allow wrap to restart search
		if ((mode & eWrap) && (s1 == start))
			back_to_start = true;

		// Check case sensitivity
		if (!(mode & eCaseSensitive))
			c1 = ::unitolower(c1);

		// Look for first unichar_t match
		if (c1 == firstc)
		{
			// Try to match remainder of string
			// Must adjust pointer for backwards direction
			const unichar_t* result = ::ustrmatch(source, end, (mode & eBackwards) ? s1 + 2 : s1, p1, mode);
			if (result)
				return result;
		}
	}

	return NULL;
}

// ustrmatch : matches one string with another using different modes
// source : start of entire find string
// end    : last character in find string
// start1 : position to start match (one character beyond first matching character)
// match1 : one character into matching string
// mode   : mode of operation
const unichar_t* ustrmatch(const unichar_t* source, const unichar_t* end, const unichar_t* start1, const unichar_t* match1, EFindMode mode)
{
	const unichar_t* p = start1;
	const unichar_t* q = match1;
	unichar_t c1;
	unichar_t c2;

	// Loop doing comparison of strings
	do
	{
		c1 = *p++;
		c2 = *q++;

		// Remove case if case insensitive
		if (!(mode & eCaseSensitive))
		{
			c1 = ::unitolower(c1);
			c2 = ::unitolower(c2);
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
				unichar_t cc = *(start1 - 2);
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
