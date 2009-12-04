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


// char_stream.cp

// Handles parsing a stream of text typically found in preferences

#include "char_stream.h"

#include "CCharSpecials.h"
#include "cdstring.h"

#include <cstdlib>

#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE_V2_0)
extern const char* cValueBoolTrue;
#else
const char* cValueBoolTrue = "true";
#endif

#include "CStringUtils.h"

char* char_stream::get()
{
	// Handle putback
	putback();

	// Remove leading space
	while(*mTxt == ' ') mTxt++;

	// Ignore null strs
	if (!*mTxt) return nil;

	// Handle quoted string
	if (*mTxt=='"')
	{
		char* start = ++mTxt;
		char* end = start;

		// Advance string ignoring quoted
		while(*end != '\"')
		{
			switch(*end)
			{
			case 0:
				// Nothing left so error
				return nil;
			case '\\':
				end += 2;
				break;
			default:
				end++;
			}
		}

		// Terminate string at enclosing quote and adjust rtn ptr past it
		*end = 0;
		mTxt = ++end;

		// Must unescape always!
		::FilterOutEscapeChars(start);

		// Start past first quote
		return start;
	}

	// Handle unquoted atom
	else
	{
		char* start = mTxt;

		// Find first space, bracket or end
		char* end = ::strpbrk(start, cINETCharBreak);

		// Terminate string at space or point to end of atom
		if (end)
		{
			mPutback = *end;
			*end = 0;
			end++;
		}
		else
			end = start + ::strlen(start);
		mTxt = end;

		return start;
	}
}

void char_stream::get(cdstring& copy, bool convert)
{
	char* p = get();
	if (p)
	{
		copy = p;
		if (convert)
			copy.ConvertToOS();
	}
}

void char_stream::get(bool& copy)
{
	char* p = get();
	if (p)
		copy = (::strcmpnocase(p, cValueBoolTrue) == 0);
}

void char_stream::get(long& copy)
{
	char* p = get();
	if (p)
		copy = ::atol(p);
}

void char_stream::get(unsigned long& copy)
{
	char* p = get();
	if (p)
		copy = ::atol(p);
}

bool char_stream::start_sexpression()
{
	// Handle putback
	putback();

	// Punt white space
	while(*mTxt == ' ') mTxt++;

	// Must start with (
	if (*mTxt != '(') return false;

	mTxt++;

	// Check for empty
	if (*mTxt == ')')
	{
		// Punt over end
		mTxt++;
		return false;
	}

	return true;
}

bool char_stream::end_sexpression()
{
	// Handle putback
	putback();

	// Punt white space
	while(*mTxt == ' ') mTxt++;

	// Just punt over trailing ')'
	if (*mTxt == ')')
	{
		mTxt++;
		return true;
	}
	else
		return (!*mTxt);
}

bool char_stream::test_start_sexpression()
{
	// Handle putback
	putback();

	// Punt white space
	while(*mTxt == ' ') mTxt++;

	// Must start with (
	return (*mTxt == '(');
}

bool char_stream::test_end_sexpression()
{
	// Handle putback
	putback();

	// Punt white space
	while(*mTxt == ' ') mTxt++;

	// Look for trailing ')'
	if (*mTxt == ')')
		return true;
	else
		return (!*mTxt);
}

// Handle the putback character
void char_stream::putback()
{
	// If putback exists, put it back into the stream
	if (mPutback && (mPutback != ' '))
	{
		*--mTxt = mPutback;
		mPutback = 0;
	}
}
