/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for cdstring class

#include "cdstring.h"

#include "char_stream.h"
#include "CCharsetManager.h"
#include "CCharSpecials.h"
#include "CURL.h"
#include "CStringUtils.h"
#include "CUTF8.h"
#ifdef __MULBERRY
#include "CXStringResources.h"
#endif

#include "cdustring.h"
#include "encrypt.h"
#include "md5.h"

#include <stdio.h>

#include <algorithm>

#include <strstream>

#if __dest_os == __linux_os
#include "HResourceMap.h"

#ifdef __MULBERRY
#include <JRect.h>
#endif

#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
#include "MyCFString.h"
#endif
#endif

extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

cdstring cdstring::null_str;
char cdstring::_empty = 0;

const char cSafemUTF7[] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
	  1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 48 - 63
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 80 - 95
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// 240 - 255

// Constants for modified-UTF7 base64
const signed char cmUTF7debase64[] =
	{	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 15
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 31
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, 63, -1, -1, -1,		// 47
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,		// 63
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,		// 79
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,		// 95
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,		// 111
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,		// 127
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 143
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 159
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 175
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 191
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 207
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 223
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// 239
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                      	 0         1         2         3         4         5         6
//                  	     0123456789012345678901234567890123456789012345678901234567890123
const char cmUTF7base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

struct TBase64
{
#ifdef big_endian
	unsigned b0 : 6;
	unsigned b1 : 6;
	unsigned b2 : 6;
	unsigned b3 : 6;
#else
	unsigned b3 : 6;
	unsigned b2 : 6;
	unsigned b1 : 6;
	unsigned b0 : 6;
#endif
};

union TAtom
{
	TBase64			base64;
	unsigned char	base256[3];
};


// Construct from number
cdstring::cdstring(const long num)
{
	_init();
	char buf[256];
	::snprintf(buf, 256, "%ld", num);
	_allocate(buf);
}

// Construct from number
cdstring::cdstring(const unsigned long num)
{
	_init();
	char buf[256];
	::snprintf(buf, 256, "%lu", num);
	_allocate(buf);
}

// Construct from number
cdstring::cdstring(const int32_t num)
{
	_init();
	char buf[256];
	::snprintf(buf, 256, "%ld", (long)num);
	_allocate(buf);
}

cdstring::cdstring(const uint32_t num)
{
	_init();
	char buf[256];
	::snprintf(buf, 256, "%lu", (unsigned long)num);
	_allocate(buf);
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
// Construct from CFString
cdstring::cdstring(const PPx::CFString& cfstr)
{
	_init();
	MyCFString temp(cfstr);
	temp.GetString(*this, kCFStringEncodingUTF8);
}
#endif

#endif

// Construct from rectangle
cdstring::cdstring(const Rect& rc)
{
	_init();
	char buf[256];
	::snprintf(buf, 256, "%d, %d, %d, %d", rc.left, rc.top, rc.right, rc.bottom);
	_allocate(buf);
}

#ifdef __MFC_STDAFX_H
#ifdef _UNICODE
// Construct from CString
cdstring::cdstring(const CString& str)
{
	_init();
	*this = cdustring(str).ToUTF8();
}

cdstring::cdstring(const TCHAR* str)
{
	_init();
	*this = cdustring(str).ToUTF8();
}

// Assignment with CString
cdstring& cdstring::operator=(const CString& str)
{
	*this = cdustring(str).ToUTF8();
	return *this;
}

// Assignment with CString
cdstring& cdstring::operator=(const TCHAR* str)
{
	*this = cdustring(str).ToUTF8();
	return *this;
}

// Append CString
cdstring& cdstring::operator+=(const CString& str)
{
	*this += cdustring(str).ToUTF8();
	return *this;
}

// Append CString
cdstring& cdstring::operator+=(const TCHAR* str)
{
	*this += cdustring(str).ToUTF8();
	return *this;
}

#endif

// Return Windows TCHAR string
CString cdstring::win_str() const
{
#ifdef _UNICODE
	cdustring temp(*this);
	return CString(temp.c_str());
#else
	return CString(_str ? _str : "");
#endif
}

#else
#ifdef _UNICODE

cdstring::cdstring(const TCHAR* str)
{
	_init();
	*this = cdustring(str).ToUTF8();
}

// Assignment with CString
cdstring& cdstring::operator=(const TCHAR* str)
{
	*this = cdustring(str).ToUTF8();
	return *this;
}

// Append CString
cdstring& cdstring::operator+=(const TCHAR* str)
{
	*this += cdustring(str).ToUTF8();
	return *this;
}

// Return Windows TCHAR string
cdustring cdstring::win_str() const
{
	return cdustring(*this);
}
#endif
#endif

// Convert number to string
cdstring& cdstring::operator=(const long num)
{
	char buf[256];
	::snprintf(buf, 256, "%ld", num);
	_allocate(buf);
	return *this;
}

// Convert number to string
cdstring& cdstring::operator=(const unsigned long num)
{
	char buf[256];
	::snprintf(buf, 256, "%lu", num);
	_allocate(buf);
	return *this;
}

// Convert number to string
cdstring& cdstring::operator=(const int32_t num)
{
	char buf[256];
	::snprintf(buf, 256, "%ld", (long)num);
	_allocate(buf);
	return *this;
}

cdstring& cdstring::operator=(const uint32_t num)
{
	char buf[256];
	::snprintf(buf, 256, "%lu", (unsigned long)num);
	_allocate(buf);
	return *this;
}

// Convert rectangle to string
cdstring& cdstring::operator=(const Rect& rc)
{
	char buf[256];
	::snprintf(buf, 256, "%d, %d, %d, %d", rc.left, rc.top, rc.right, rc.bottom);
	_allocate(buf);
	return *this;
}

// Compare with same
int cdstring::operator==(const cdstring& comp) const
{
	return operator==(comp.c_str());
}

// Compare with c-string
int cdstring::operator==(const char* cstr) const
{
	if (_str && cstr)
		return (::strcmp(_str, cstr) == 0);
	else if (_str && !cstr)
		return *_str ? 0 : 1;
	else if (!_str && cstr)
		return *cstr ? 0 : 1;
	else
		return 1;
}

// Compare with p-string
int cdstring::operator==(const unsigned char* pstr) const
{
	if (_str && pstr)
	{
		if (length() != *pstr)
			return 0;
		else
			return (::strncmp(_str, (const char*) &pstr[1], *pstr) == 0);
	}
	else if (_str && !pstr)
		return *_str ? 0 : 1;
	else if (!_str && pstr)
		return *pstr ? 0 : 1;
	else
		return 1;
}

// Compare with same
int cdstring::operator<(const cdstring& comp) const
{
	if (_str && comp._str)
		return (::strcmp(_str, comp._str) < 0);
	else if (_str && !comp._str)
		return *_str ? 0 : 1;
	else if (!_str && comp._str)
		return *comp._str ? 1 : 0;
	else
		return 0;
}

// Trim storage to actual size of string
void cdstring::trim()
{
	if (length())
		steal(::strdup(_str));
}

// Trim off leading or trailing space
void cdstring::trimspace()
{
	size_type len = length();
	if (len)
	{
		// Strip leading
		char* p = _str;
		while(*p == ' ')
			p++;

		// Strip trailing
		char* q = _str + len - 1;
		while((*q == ' ') && (q >= p))
			q--;

		// Copy middle
		len = q - p + 1;
		if (len)
			q = ::strndup(p, len);
		else
			q = NULL;
		steal(q);
	}
}

cdstring& cdstring::erase(size_type pos, size_type n)
{
	size_type len = length();
	if (pos >= len)
		return *this;

	if ((n == npos) || (pos + n > len))
		n = len - pos;
	
	if ((pos == 0) && (n == len))
	{
		clear();
		return *this;
	}
	
	// Determine length of chunk after the erased portion
	size_type len_after = len - (pos + n);
	
	// Allocate space for chunks either side of the one removed
	char* p = new char[pos + len_after + 1];
	
	// Copy over the chunk before the erase
	if (pos != 0)
		::memcpy(p, _str, pos);
	
	// Copy over the chunk after the erase
	if (len_after != 0)
		::memcpy(p + pos, _str + pos + n, len_after);
	p[pos + len_after] = 0;
	steal(p);
	
	return *this;
}

cdstring::size_type cdstring::find(const char* s, size_type pos, size_type n, bool casei) const
{
	if (s == NULL)
		return npos;
	
	const char* beg = _str;
	size_type sz = length();
	if (pos <= sz)
	{
		size_type d1 = sz - pos;
		const char* sn = s + n;
		const char* xpos = beg + pos;
		for (; d1 >= n; ++xpos, --d1)
		{
			const char* p1 = xpos;
			for (const char* s1 = s; s1 < sn; ++s1, ++p1)
			{
				if (casei)
				{
					if (!::tolower(*s1) != ::tolower(*p1))
						goto loop;
				}
				else
				{
					if (*s1 != *p1)
						goto loop;
				}
			}
			return static_cast<size_type>(xpos - beg);
		loop:;
		}
	}
	return npos;
}

cdstring::size_type cdstring::find(char c, size_type pos, bool casei) const
{
	const char* beg = _str;
	size_type sz = length();
	if (pos < sz)
	{
		const char* e = beg + sz;
		char cl = ::tolower(c);
		for (const char* xpos = beg + pos; xpos < e; ++xpos)
		{
			if (casei)
			{
				if (cl == ::tolower(*xpos))
					return static_cast<size_type>(xpos - beg);
			}
			else
			{
				if (c == *xpos)
					return static_cast<size_type>(xpos - beg);
			}
		}
	}
	return npos;
}

cdstring::size_type cdstring::rfind(const char* s, size_type pos, size_type n, bool casei) const
{
	if (s == NULL)
		return npos;
	
	const char* beg = _str;
	size_type sz = length();
	if (sz >= n)
	{
		if (pos > sz - n)
			pos = sz - n;
		const char* sn = s + n;
		const char* xpos = beg + pos;
		do
		{
			const char* p1 = xpos;
			for (const char* s1 = s; s1 < sn; ++s1, ++p1)
			{
				if (casei)
				{
					if (::tolower(*s1) != ::tolower(*p1))
						goto loop;
				}
				else
				{
					if (*s1 != *p1)
						goto loop;
				}
			}
			return static_cast<size_type>(xpos - beg);
		loop:;
		} while (xpos > beg && (--xpos, true));
	}
	return npos;
}

cdstring::size_type cdstring::rfind(char c, size_type pos, bool casei) const
{
	const char* beg = _str;
	size_type sz = length();
	if (sz > 0)
	{
		if (pos > sz - 1)
			pos = sz - 1;
		const char* xpos = beg + pos;
		char cl = ::tolower(c);
		do
		{
			if (casei)
			{
				if (cl == ::tolower(*xpos))
					return static_cast<size_type>(xpos - beg);
			}
			else
			{
				if (c == *xpos)
					return static_cast<size_type>(xpos - beg);
			}
		} while (xpos > beg && (--xpos, true));
	}
	return npos;
}

cdstring::size_type cdstring::find_first_of(const char* s, size_type pos, size_type n) const
{
	if (s == NULL)
		return npos;
	
	const char* beg = _str;
	size_type sz = length();

	if (pos < sz && n != 0)
	{
		const char* e = beg + sz;
		for (const char* xpos = beg + pos; xpos < e; ++xpos)
		{
			const char* j = s;
			for (size_type n1 = 0; n1 < n; ++n1, ++j)
				if (*xpos == *j)
					return size_type(xpos - beg);
		}
	}
	return npos;
}

cdstring::size_type cdstring::find_last_of(const char* s, size_type pos, size_type n) const
{
	if (s == NULL)
		return npos;
	
	const char* beg = _str;
	size_type sz = length();

	if ((sz != 0) && (n != 0))
	{
		if (pos > sz - 1)
			pos = sz - 1;
		for (const char* xpos = beg + pos + 1; xpos > beg;)
		{
			--xpos;
			const char* j = s;
			for (size_type n1 = 0; n1 < n; ++n1, ++j)
				if (*xpos == *j)
					return size_type(xpos - beg);
		}
	}
	return npos;
}

cdstring::size_type cdstring::find_first_not_of(const char* s, size_type pos, size_type n) const
{
	const char* beg = _str;
	size_type sz = length();

	if (pos < sz)
	{
		if (n == 0)
			return pos;
		const char* e = beg + sz;
		for (const char* xpos = beg + pos; xpos < e; ++xpos)
		{
			const char* j = s;
			for (size_type n1 = 0; n1 < n; ++n1, ++j)
				if (*xpos == *j)
					goto not_this_one;
			return size_type(xpos - beg);
		not_this_one:
			;
		}
	}
	return npos;
}

cdstring::size_type cdstring::find_first_not_of(char c, size_type pos) const
{
	const char* beg = _str;
	size_type sz = length();

	if (pos < sz)
	{
		const char* e = beg + sz;
		for (const char* xpos = beg + pos; xpos < e; ++xpos)
		{
			if (*xpos != c)
				return size_type(xpos - beg);
		}
	}
	return npos;
}

cdstring::size_type cdstring::find_last_not_of(const char* s, size_type pos, size_type n) const
{
	const char* beg = _str;
	size_type sz = length();

	if (sz != 0)
	{
		if (pos > sz - 1)
			pos = sz - 1;
		//const char* e = beg + sz;
		for (const char* xpos = beg + pos + 1; xpos > beg;)
		{
			--xpos;
			const char* j = s;
			for (size_type n1 = 0; n1 < n; ++n1, ++j)
				if (*xpos == *j)
					goto not_this_one;
			return size_type(xpos - beg);
		not_this_one:
			;
		}
	}
	return npos;
}

cdstring::size_type cdstring::find_last_not_of(char c, size_type pos) const
{
	const char* beg = _str;
	size_type sz = length();

	if (sz != 0)
	{
		if (pos > sz - 1)
			pos = sz - 1;
		for (const char* xpos = beg + pos + 1; xpos > beg;)
		{
			if (*--xpos != c)
				return size_type(xpos - beg);
		}
	}
	return npos;
}

int cdstring::compare(size_type pos1, size_type n1, const char* s, size_type n2, bool casei) const
{
	if (s == NULL)
		return 1;
	
	const char* p = _str;
	size_type sz = length();

	if (pos1 > sz)
		return 1;
	size_type len = std::min(sz - pos1 , n1);
	size_type rlen = std::min(len, n2);
	int result = casei ? ::strncmpnocase(p + pos1, s, rlen) : ::strncmp(p + pos1, s, rlen);
	if (result == 0)
	{
		if (len < n2)
			return -1;
		if (len == n2)
			return 0;
		return 1;
	}
	return result;
}

// Compare at start
bool cdstring::compare_start(const char* comp, bool casei) const
{
	// Length of this must be >= comp length
	size_type len1 = length();
	size_type len2 = ::strlen(comp);
	if ((len1 < len2) || !len1 || !len2)
		return 0;

	// Compare first chars
	return (casei ? ::strncmpnocase(_str, comp, len2) : ::strncmp(_str, comp, len2)) == 0;
}

// Compare at end
bool cdstring::compare_end(const char* comp, bool casei) const
{
	// Length of this must be >= comp length
	size_type len1 = length();
	size_type len2 = ::strlen(comp);
	if ((len1 < len2) || !len1 || !len2)
		return 0;

	// Compare last chars
	return (casei ? ::strcmpnocase(&_str[len1 - len2], comp) : ::strcmp(&_str[len1 - len2], comp)) == 0;
}

// Is string quoted
bool cdstring::isquoted() const
{
	size_type len = length();

	// Check for quotes at start and end
	if (len && (*_str == '\"') && (_str[len - 1] == '\"'))
		// Have quotes
		return true;

	return false;
}

// Add quotes if not already there
bool cdstring::quote(const char* specials, bool force, bool inet)
{
	if (specials == NULL)
		specials = cINETChar;

	if (_str && *_str)
	{
		// Look for special cases
		bool escape = true;
		if (!force)
		{
			char* p = _str;
			escape = false;
			bool quoted = false;
			while(*p && !escape)
			{
				switch(specials[(unsigned char) *p++])
				{
				case 0: // Atom
					break;
				case 1: // Quote
					quoted = true;
					break;
				case 2: // Escaped
					escape = true;
					break;
				case 3: // Literal	 - dubious case what to do? Probably go to UTF8
					escape = true;
					break;
				}
			}

			if (!quoted && !escape)
				return false;
		}

		// Enclose existing in quotes
		if (escape)
			FilterInEscapeChars(inet ? cINETChar : cCEscapeChar);
		char* p = new char[length() + 3];
		*p = 0;
		::strcat(p, "\"");
		::strcat(p, _str);
		::strcat(p, "\"");

		steal(p);
	}
	else
		// Use empty quoted string
		_allocate('\"', 2);

	return true;
}

// Remove quotes if there
bool cdstring::unquote()
{
	size_type len = length();
	if (len)
	{
		// Check for quotes at start and end
		if ((*_str == '\"') && (_str[len - 1] == '\"'))
		{
			// Strip quotes and store as new string
			if (len > 2)
			{
				steal(::strndup(_str + 1, ::strlen(_str) -2));
				FilterOutEscapeChars();
				return true;
			}
			else
				_tidy();
			return true;
		}
	}

	// No quotes
	return false;
}

// Split into tokens
void cdstring::split(const char* tokens, cdstrvect& results, bool trimit) const
{
	cdstring::size_type start = 0;
	cdstring::size_type end = cdstring::npos;
	end = find_first_of(tokens);
	while(end != cdstring::npos)
	{
		cdstring temp;
		temp.assign(*this, start, end - start);
		if (trimit)
			temp.trimspace();
		results.push_back(temp);
		start = find_first_not_of(tokens, end);
		if (start == cdstring::npos)
			break;
		end = find_first_of(tokens, start);
	}
	
	if (start != cdstring::npos)
	{
		cdstring temp;
		temp.assign(*this, start, cdstring::npos);
		if (trimit)
			temp.trimspace();
		results.push_back(temp);
	}
}

// Join list into single string
void cdstring::join(const cdstrvect& items, const char* delim)
{
	std::ostrstream sout;
	bool first = true;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		if (first)
			first = false;
		else
			sout << delim;
		sout << *iter;
	}
	sout << std::ends;
	
	// Grab the string
	steal(sout.str());
}

// Filter out C-style escape chars '\'
void cdstring::FilterOutEscapeChars()
{
	if (!_str) return;

	char* p = _str;
	char* q = _str;

	// Look at all chars
	unsigned long ctr = 0;
	while(*p)
	{
		// Is current char escape
		if (*p=='\\')
		{
			// Advance past escape
			ctr++;
			p++;

			// Test escaped char
			switch(*p++)
			{
			case 'a':			// alert
				*q++ = '\a';
				break;

			case '\\':			// backslash
				*q++ = '\\';
				break;

			case 'b':			// backspace
				*q++ = '\b';
				break;

			case 'r':			// carriage return
				*q++ = '\r';
				break;

			case '\"':			// double quote
				*q++ = '\"';
				break;

			case 'f':			// formfeed
				*q++ = '\f';
				break;

			case 't':			// horizontal tab
				*q++ = '\t';
				break;

			case 'n':			// newline
				*q++ = '\n';
				break;

			case '0':			// null character
				*q++ = 0;
				break;

			case '\'':			// single quote
				*q++ = '\'';
				break;

			case 'v':			// vertical tab
				*q++ = '\v';
				break;

			case 0:
				p--;
				break;

			default:
				*q = '?';
			}
		}
		else
			*q++ = *p++;
	}

	// add terminator
	*q = 0;

	// Duplicate and reassign if changed
	if (!ctr)
		steal(::strdup(_str));
}

// Filter in C-style escape chars '\'
void cdstring::FilterInEscapeChars(const char* charset)
{
	if (!_str) return;

	const char* p = _str;

	// count number of escapes
	unsigned long ctr = 0;
	unsigned long extra_ctr = 0;
	while(*p)
	{
		// Add extra char for escapes
		if (charset[static_cast<unsigned char>(*p)] == 2)	// Must escape
			extra_ctr++;

		// One char added
		ctr++;
		p++;
	}

	// Only bother if required
	if (!extra_ctr) return;

	// create new string
	char* r = new char[ctr + extra_ctr + 1];
	char* q = r;

	// Reset to start of string
	p = _str;
	while(*p)
	{
		// Test escaped char
		if (charset[static_cast<unsigned char>(*p)] == 2)	// Must escape
		{
			*q++ = '\\';
			*q++ = cCEscape[static_cast<unsigned char>(*p)];
		}
		else
			*q++ = *p;

		p++;
	}

	// Terminate
	*q = 0;

	// Reallocate
	steal(r);
}

// Encode to URL
void cdstring::EncodeURL(char ignore)
{
	if (!_str) return;

	const char* p = _str;

	// count number of escapes
	unsigned long total = 0;
	unsigned long ctr = 0;
	while(*p)
	{
		// Add two extra chars for escapes
		if ((cURLUnreserved[(unsigned char) *p] == 0) && (*p != ignore))
			total += 2;

		// One char added
		total++;
		ctr++;
		p++;
	}

	// Only bother if required
	if (total == ctr) return;

	// create new string
	char* r = new char[total + 1];
	char* q = r;

	// Reset to start of string
	p = _str;
	while(*p)
	{
		// Test escaped char
		if ((cURLUnreserved[(unsigned char) *p] == 0) && (*p != ignore))
		{
			*q++ = cURLEscape;

			// Do high nibble
			*q++ = cHexChar[(((unsigned char) *p) >> 4)];

			// Do low nibble
			*q++ = cHexChar[(((unsigned char) *p) & 0x0F)];
		}
		else
			*q++ = *p;

		p++;
	}

	// Terminate
	*q = 0;

	// Reallocate
	steal(r);
}

// Encode to URL
void cdstring::DecodeURL()
{
	if (!_str) return;

	char* p = _str;
	char* q = _str;

	// Look at all chars
	unsigned long ctr = 0;
	while(*p)
	{
		// Is current char escape
		if (*p == cURLEscape)
		{
			// Advance past escape
			ctr++;
			p++;

			// Copy nibbles of quoted char
			char c = (cFromHex[(unsigned char) *p++] << 4);
			c |= cFromHex[(unsigned char) *p++];
			*q++ = c;
		}
		else
			*q++ = *p++;
	}

	// add terminator
	*q = 0;

	// Duplicate and reassign if changed
	if (!ctr)
		steal(::strdup(_str));
}

// Load from resource
cdstring& cdstring::FromResource(long res_id, long index)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Convert to STR# + id
	short strx_id = index ? res_id : (res_id >> 16);
	short str_id = index ? index : (res_id & 0x0000FFFF);

	// Is it a STR#
	if (str_id)
	{
		Str255 temp;
		::GetIndString(temp, strx_id, str_id);
		*this = temp;
	}
	else
	{
		StringHandle hdl = ::GetString(strx_id);
		if (hdl)
		{
			*this = *hdl;
			::ReleaseResource((Handle) hdl);
		}
	}
#elif __dest_os == __win32_os
#ifdef __MFC_STDAFX_H
	CString s;
	s.LoadString(res_id);
	*this = s;
#endif
#elif __dest_os == __linux_os
	*this = stringFromResource(res_id);
#endif

	return *this;
}

// Load from XML resource
cdstring& cdstring::FromResource(const char* rsrcid)
{
#ifdef __MULBERRY
	*this = rsrc::GetString(rsrcid);
#endif

	return *this;
}

// Append from resource
void cdstring::AppendResource(long res_id, long index)
{
	cdstring temp;
	*this += temp.FromResource(res_id, index);
}

// Append from resource
void cdstring::AppendResource(const char* rsrcid)
{
	cdstring temp;
	*this += temp.FromResource(rsrcid);
}

// snprintf %s substitute
void cdstring::Substitute(const cdstring& str)
{
	if (length() < 2)
		return;

	// Assume this string contains a single %s and do snprintf substitution
	cdstring buf;
	size_type buflen = length() + str.length() + 1;
	buf.reserve(buflen);
	::snprintf(buf.c_str_mod(), buflen, c_str(), str.c_str());
	
	// Grab the result into this one
	steal(buf.release());
}

// snprintf %ld substitute
void cdstring::Substitute(unsigned long ul)
{
	if (length() < 2)
		return;

	// Assume this string contains a single %ld and do snprintf substitution
	cdstring buf;
	size_type buflen = length() + 12;
	buf.reserve(buflen);
	::snprintf(buf.c_str_mod(), buflen, c_str(), ul);
	
	// Grab the result into this one
	steal(buf.release());
}

// snprintf %ld substitute
void cdstring::Substitute(long ul)
{
	if (length() < 2)
		return;

	// Assume this string contains a single %ld and do snprintf substitution
	cdstring buf;
	size_type buflen = length() + 12;
	buf.reserve(buflen);
	::snprintf(buf.c_str_mod(), buflen, c_str(), ul);
	
	// Grab the result into this one
	steal(buf.release());
}

#pragma mark ____________________________S-Expressions

// Parse S-Expression into list
void cdstring::ParseSExpression(cdstrvect& list) const
{
	// Clear existing
	list.clear();

	// Check that this is not empty
	if (!_str)
		return;

	// Parse S-Expression list
	char* txt = _str;
	while(*txt == ' ') txt++;

	// Must start with '('
	if (*txt != '(') return;
	txt++;

	// Old style is double-bracketed
	if (*txt == '(')
	{
		while(*txt == '(')
		{
			char* p = ::strgetbrastr(&txt);
			if (p)
				list.push_back(p);
		}
	}

	// New style is IMAP style - use astrings
	else
	{
		while(*txt && (*txt != ')'))
		{
			char* p = ::strgettokenstr(&txt, " \t\r\n({})");
			if (p)
				list.push_back(p);
		}
	}
}

// Parse S-Expression into list
void cdstring::ParseSExpression(char_stream& txt, cdstrvect& list, bool convert)
{
	// Clear existing
	list.clear();

	// Check that this is not empty
	if (!*txt)
		return;

	// Must start with '('
	if (!txt.start_sexpression()) return;

	// Old style is double-bracketed
	if (*txt == '(')
	{
		while(txt.start_sexpression())
		{
			char* q = txt.get();
			if (q)
			{
				list.push_back(q);
				if (convert)
					list.back().ConvertToOS();
			}
			txt.end_sexpression();
		}
	}

	// New style is IMAP style - use astrings
	else
	{
		while(!txt.end_sexpression())
		{
			char* q = txt.get();
			if (q)
			{
				list.push_back(q);
				if (convert)
					list.back().ConvertToOS();
			}
		}
	}
}

// Create S_Expression from list
void cdstring::CreateSExpression(const cdstrvect& list, bool old_style)
{
	if (old_style)
	{
		*this += '(';
		for(cdstrvect::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			*this += '(';
			*this += *iter;
			*this += ')';
		}
		*this += ')';
	}
	else
	{
		*this = '(';
		bool first = true;
		for(cdstrvect::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			if (!first)
				*this += ' ';
			else
				first = false;

			cdstring temp(*iter);
			temp.quote(cINETChar);
			*this += temp;
		}
		*this += ')';
	}
}

// Parse S-Expression into list
void cdstring::ParseSExpression(cdstrpairvect& list) const
{
	// Clear existing
	list.clear();

	// Check that this is not empty
	if (!_str)
		return;

	// Parse S-Expression list
	char* txt = _str;
	char* p = ::strgetbrastr(&txt);
	txt = p;

	while(p && *txt)
	{
		p = ::strgetbrastr(&txt);
		if (p && *p)
		{
			char* s1 = ::strgettokenstr(&p, " \t\r\n({})");
			if (s1)
			{
				char* s2 = ::strgettokenstr(&p, " \t\r\n({})");
				if (s2)
					list.push_back(cdstrpair(s1, s2));
				else
					p = NULL;
			}
			else
				p = NULL;
		}
	}
}

// Parse S-Expression into list
void cdstring::ParseSExpression(char_stream& txt, cdstrpairvect& list, bool convert)
{
	// Clear existing
	list.clear();

	// Check that this is not empty
	if (!*txt)
		return;

	// Must start with '('
	if (!txt.start_sexpression()) return;

	while(txt.start_sexpression())
	{
		char* s1 = txt.get();
		if (s1)
		{
			char* s2 = txt.get();
			if (s2)
			{
				if (convert)
					list.push_back(cdstrpair(ConvertToOS(s1), ConvertToOS(s2)));
				else
					list.push_back(cdstrpair(s1, s2));
			}
		}
		txt.end_sexpression();
	}

	txt.end_sexpression();
}

// Create S_Expression from list
void cdstring::CreateSExpression(const cdstrpairvect& list)
{
	*this = '(';
	for(cdstrpairvect::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		*this += '(';

		cdstring temp = (*iter).first;
		temp.quote(cINETChar);
		*this += temp;

		*this += ' ';

		temp = (*iter).second;
		temp.quote(cINETChar);
		*this += temp;

		*this += ')';
	}
	*this += ')';
}

// Parse S-Expression into map
void cdstring::ParseSExpression(cdstrmap& map) const
{
	// Parse into cdstrvect first
	cdstrvect temp;
	ParseSExpression(temp);
	
	map.clear();
	
	// Add each vector pair to map
	for(cdstrvect::const_iterator iter = temp.begin(); iter != temp.end(); iter += 2)
		map.insert(cdstrmap::value_type(*iter, *(iter+1)));
}

// Parse S-Expression into list
void cdstring::ParseSExpression(char_stream& txt, cdstrmap& map, bool convert)
{
	// Parse into cdstrvect first
	cdstrvect temp;
	ParseSExpression(txt, temp, convert);
	
	map.clear();
	
	// Add each vector pair to map
	for(cdstrvect::const_iterator iter = temp.begin(); iter != temp.end(); iter += 2)
		map.insert(cdstrmap::value_type(*iter, *(iter+1)));
}

// Create S_Expression from map
void cdstring::CreateSExpression(const cdstrmap& map)
{
	// Convert map to cdstrvect
	cdstrvect temp;
	for(cdstrmap::const_iterator iter = map.begin(); iter != map.end(); iter++)
	{
		temp.push_back((*iter).first);
		temp.push_back((*iter).second);
	}
	
	// Now get SExpression for cdstrvect
	CreateSExpression(temp);
}

// Parse S-Expression into set
void cdstring::ParseSExpression(cdstrset& set) const
{
	// Parse into cdstrvect first
	cdstrvect temp;
	ParseSExpression(temp);
	
	set.clear();
	
	// Add each vector item to set
	for(cdstrvect::const_iterator iter = temp.begin(); iter != temp.end(); iter++)
		set.insert(*iter);
}

// Parse S-Expression into list
void cdstring::ParseSExpression(char_stream& txt, cdstrset& set, bool convert)
{
	// Parse into cdstrvect first
	cdstrvect temp;
	ParseSExpression(txt, temp, convert);
	
	set.clear();
	
	// Add each vector item to set
	for(cdstrvect::const_iterator iter = temp.begin(); iter != temp.end(); iter++)
		set.insert(*iter);
}

// Create S_Expression from set
void cdstring::CreateSExpression(const cdstrset& set)
{
	// Convert set to cdstrvect
	cdstrvect temp;
	for(cdstrset::const_iterator iter = set.begin(); iter != set.end(); iter++)
	{
		temp.push_back(*iter);
	}
	
	// Now get SExpression for cdstrvect
	CreateSExpression(temp);
}

#pragma mark ____________________________Arrays

// Convert array of pointers to list
void cdstring::FromArray(const char** txt, cdstrvect& list, bool unique)
{
	const char** p = txt;
	while(*p)
	{
		// Only add if not empty
		if (**p)
		{
			// test for uniqueness if required
			if (unique)
			{
				cdstrvect::const_iterator found = std::find(list.begin(), list.end(), *p);
				if (found == list.end())
					list.push_back(*p);
			}
			else
				list.push_back(*p);
		}
		
		// Next one
		p++;
	}
}

// Convert array of pointers to list
const char** cdstring::ToArray(const cdstrvect& list, bool copy)
{
	// Allocate size of array with extra for NULL terminator
	const char** result = new const char*[list.size() + 1];
	
	// Duplicate strings and add to list
	const char** p = result;
	for(cdstrvect::const_iterator iter = list.begin(); iter != list.end(); iter++, p++)
		*p = (copy ? ::strdup((*iter).c_str()) : (*iter).c_str());
		
	// NULL terminate
	*p = NULL;
	
	return result;
}

// Delete all items in array and array itself
void cdstring::FreeArray(const char** txt)
{
	// Delete each item in the array
	const char** p = txt;
	while(*p)
	{
		delete *p;
		p++;
	}
	delete txt;
}

#pragma mark ____________________________Searching

// Do pattern match
bool cdstring::PatternMatch(const cdstring& pattern) const
{
	return ::strpmatch(_str ? _str : &_empty, pattern);
}

// Match hierarchy pattern
bool cdstring::PatternDirMatch(const cdstring& pattern, char separator) const
{
	return ::strpdirmatch(_str ? _str : &_empty, pattern, separator);
}

#pragma mark ____________________________Transformations

// Encrypt
void cdstring::Encrypt(EStringEncrypt method, const char* key)
{
	// Only if something present
	if (!_str)
		return;

	switch(method)
	{
	case eEncryptSimple:
		::encrypt_simple(_str);
		break;
	case eEncryptSimplemUTF7:	// Do cipher encrypt with fixed key
		key = "Modified-UTF7";
	case eEncryptCipher:
		{
			char* encrypted = ::encrypt_cipher(_str, key);
			_tidy();
			_str = encrypted;
		}
		break;
	}
}

// Decrypt
void cdstring::Decrypt(EStringEncrypt method, const char* key)
{
	// Only if something present
	if (!_str)
		return;

	switch(method)
	{
	case eEncryptSimple:
		::decrypt_simple(_str);
		break;
	case eEncryptSimplemUTF7:	// Do cipher decrypt with fixed key
		key = "Modified-UTF7";
	case eEncryptCipher:
		{
			char* decrypted = ::decrypt_cipher(_str, key);
			_tidy();
			_str = decrypted;
		}
		break;
	}
}

// Generate md5 hash
void cdstring::md5(unsigned long& digest) const
{
	unsigned char temp[16];
	md5(temp);
	
	digest = *reinterpret_cast<unsigned long*>(&temp[0]);
	digest += *reinterpret_cast<unsigned long*>(&temp[4]);
	digest += *reinterpret_cast<unsigned long*>(&temp[8]);
	digest += *reinterpret_cast<unsigned long*>(&temp[12]);
}

// Generate hexed md5 hash
void cdstring::md5(cdstring& digest) const
{
	unsigned char digest_num[16];
	md5(digest_num);

	digest.reserve(34);
	char* temp = digest.c_str_mod();
	for (int i = 0; i < 16; i++)
	{
		*temp++ = cHexChar[digest_num[i] >> 4];
		*temp++ = cHexChar[digest_num[i] & 0x0F];
	}
	*temp = 0;
}

// Generate md5 hash
void cdstring::md5(unsigned char digest[16]) const
{
	MD5_CTX theDigest;

	MD5Init(&theDigest);
	MD5Update (&theDigest, reinterpret_cast<const unsigned char*>(_str), length());
	MD5Final(digest, &theDigest);
}

// Basic string hash
unsigned long cdstring::hash() const
{
	return hash(_str);
}

// Basic string hash
unsigned long cdstring::hash(const char* str)
{
	unsigned long ret_val = 0;
	int i;

	const char* p = str;
	if (p)
	{
		while(*p)
		{
			i = (int) *p;
			ret_val ^= i;
			ret_val <<= 1;
			p++;
		}
	}
	return ret_val;
}

// Convert to modified-UTF7 (for IMAP4rev1)
void cdstring::ToModifiedUTF7(bool charset)
{
	// Only if something present
	if (!_str)
		return;

	// Look for characters to encode
	char* p = _str;
	bool unsafe = false;
	while(*p && !unsafe)
		unsafe = !cSafemUTF7[(unsigned char) *p++];

	// Encode it if required
	if (unsafe)
		steal(ToModifiedUTF7(_str, charset));
}

// Convert to modified-UTF7 (for IMAP4rev1)
char* cdstring::ToModifiedUTF7(const char* str, bool charset)
{
	// Only if something present
	if (!str)
		return NULL;

	// Count number chars after encoding
	const char* p = str;
	unsigned long total = 0;
	bool ampersand = false;
	bool unsafe = false;
	while(*p && !unsafe && ++total)
	{
		// For single '&' just bump
		if (*p == '&')
		{
			ampersand = true;
			total++;
		}
		else if (!cSafemUTF7[(unsigned char) *p])
			unsafe = true;
		p++;
	}

	char* q = NULL;

	// Must stream base64 if any unsafe chars
	if (unsafe)
	{
		// Stream to base64
		p = str;
		std::ostrstream sout;
		i18n::CUTF8 utf8;

		while(*p)
		{
			if (*p == '&')
			{
				sout.put(*p++);
				sout.put('-');
			}
			else if (!cSafemUTF7[(unsigned char) *p])
			{
				// Start escape sequence
				sout.put('&');

				int atom_pos = 0;
				TAtom atom;
				while(*p && !cSafemUTF7[(unsigned char) *p])
				{
					// Get converted wchar_t
					wchar_t wc = utf8.c_2_w((const unsigned char*&) p);
					
					// Encode two bytes from wchar_t
					for(int i = 0; i < 2; i++)
					{
						unsigned char c;
						if (i == 0)
							c = wc >> 8;	// Hi byte
						else
							c = wc & 0xFF;	// Lo byte

						// Determine encoding state
						switch(atom_pos)
						{
						case 0:
							// Zero atom at start
							atom.base256[0] = 0;
							atom.base256[1] = 0;
							atom.base256[2] = 0;

#ifdef big_endian
							atom.base256[0] = c;
#else
							atom.base256[2] = c;
#endif
							atom_pos++;
							break;
						case 1:
#ifdef big_endian
							atom.base256[1] = c;
#else
							atom.base256[1] = c;
#endif
							atom_pos++;
							break;
						case 2:
#ifdef big_endian
							atom.base256[2] = c;
#else
							atom.base256[0] = c;
#endif
							// Flush output
							sout.put(cmUTF7base64[atom.base64.b0]);
							sout.put(cmUTF7base64[atom.base64.b1]);
							sout.put(cmUTF7base64[atom.base64.b2]);
							sout.put(cmUTF7base64[atom.base64.b3]);

							atom_pos = 0;
							break;
						}
					}
				}

				// Flush remainder
				switch(atom_pos)
				{
				case 0:
					// Nothing to do
					break;
				case 1:
					// Two base64 chars to go
					sout.put(cmUTF7base64[atom.base64.b0]);
					sout.put(cmUTF7base64[atom.base64.b1]);
					break;
				case 2:
					// Three base64 chars to go
					sout.put(cmUTF7base64[atom.base64.b0]);
					sout.put(cmUTF7base64[atom.base64.b1]);
					sout.put(cmUTF7base64[atom.base64.b2]);
					break;
				}

				// Send escape sequence end
				sout.put('-');

			}
			else
				sout.put(*p++);
		}

		// Get data from stream
		sout << std::ends;
		q = sout.str();
	}

	// Must do special for ampersand
	else if (ampersand)
	{
		// Create new string and copy
		q = new char[total + 1];
		char* r = q;
		const char* s = str;
		while(*s)
		{
			if (*s == '&')
			{
				*r++ = *s++;
				*r++ = '-';
			}
			else
				*r++ = *s++;
		}
		*r = 0;
	}

	// Just duplicate
	else
		q = ::strdup(str);

	return q;
}

// Convert from modified-UTF7 (for IMAP4rev1) directly into utf8
char* cdstring::FromModifiedUTF7(char* str, bool charset)
{
	// Only if something present
	if (!str)
		return NULL;
	
	// Look for any decode
	if (::strchr(str, '&') == NULL)
		return NULL;

	{
		// Stream to store to if any decoding is needed
		std::ostrstream sout;
		i18n::CUTF8 utf8;	// The utf8 character converter

		// Catch exceptions
		try
		{
			// Bump over encoding end
			char* p = str;
			while(*p)
			{
				if ((*p == '&') && (*(p+1) == '-'))
				{
					sout.put(*p++);
					p++;
				}
				else if (*p == '&')
				{
					p++;
					int atom_pos = 0;
					TAtom atom;
					bool even = false;
					wchar_t wc = 0;
					char buf[16];
					while(*p && (*p != '-'))
					{
						switch(atom_pos)
						{
						case 0:
							// Zero atom at start
							atom.base256[0] = 0;
							atom.base256[1] = 0;
							atom.base256[2] = 0;

							atom.base64.b0 = cmUTF7debase64[(unsigned char) *p++];
							atom_pos++;
							break;
						case 1:
							atom.base64.b1 = cmUTF7debase64[(unsigned char) *p++];
							atom_pos++;
							break;
						case 2:
							atom.base64.b2 = cmUTF7debase64[(unsigned char) *p++];
							atom_pos++;
							break;
						case 3:
							atom.base64.b3 = cmUTF7debase64[(unsigned char) *p++];

							// Flush to output
#ifdef big_endian
							if (even)
							{
								// wchar_t bytes LO HI LO
								
								// Already have the HI part of the current wchar_t
								wc |= atom.base256[0];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;

								// Now do entire wchar_t
								wc = atom.base256[1];
								wc = (wc << 8) | atom.base256[2];
								len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;
							}
							else
							{
								// wchar_t bytes HI LO HI
								
								// Now do entire wchar_t
								wc = atom.base256[0];
								wc = (wc << 8) | atom.base256[1];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;

								// Cache remaining HI part
								wc = atom.base256[2];
								wc <<= 8;
							}
#else
							if (even)
							{
								// wchar_t bytes LO HI LO
								
								// Already have the HI part of the current wchar_t
								wc |= atom.base256[2];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);

								// Now do entire wchar_t
								wc = atom.base256[1];
								wc = (wc << 8) | atom.base256[0];
								len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;
							}
							else
							{
								// wchar_t bytes HI LO HI
								
								// Now do entire wchar_t
								wc = atom.base256[2];
								wc = (wc << 8) | atom.base256[1];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);

								// Cache remaining HI part
								wc = atom.base256[0];
								wc <<= 8;
							}
#endif
							// Reset atom pos and eveness
							even = ! even;
							atom_pos = 0;
							break;
						}
					}

					// Flush remainder
					switch(atom_pos)
					{
						case 0:
							// No more to process
							break;
						case 1:
							// Illegal parse
							throw -1L;
							break;
						case 2:
							// One left: even (LO), odd (HI) - error
							if (even)
							{
#ifdef big_endian
								wc |= atom.base256[0];
#else
								wc |= atom.base256[2];
#endif
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
							}
							else
								throw -1L;
							break;
						case 3:
							// Two left: even (LO HI) - error, odd (HI LO)
#ifdef big_endian
							if (even)
							{
								wc |= atom.base256[0];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;
							}
							else
							{
								// Now do entire wchar_t
								wc = atom.base256[0];
								wc = (wc << 8) | atom.base256[1];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
								wc = 0;
							}
#else
							if (even)
							{
								wc |= atom.base256[2];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
							}
							else
							{
								// Now do entire wchar_t
								wc = atom.base256[2];
								wc = (wc << 8) | atom.base256[1];
								int len = utf8.w_2_c(wc, buf);
								if (len)
									sout.write(buf, len);
							}
#endif
							break;
					}

					// Punt past mUTF7 terminator
					p++;
				}
				else
					sout.put(*p++);
			}
			sout << std::ends;
		}
		catch(long /*ex*/)
		{
			// Use unmodified string
			sout.clear();
			sout.write(str, ::strlen(str));
		}

		return sout.str();
	}
}

// Convert a string in either ISO-8859-15 or UTF-8 format into utf8 with line end translation
void cdstring::ConvertToOS()
{
	// Shortcut if empty
	if (empty())
		return;

	// See if valid UTF8
	if (!IsUTF8())
		FromISOToUTF8();
	
	// Now filter arbitrary line endings to local
	ConvertEndl();
}

// Convert a utf8 string to the external ISO-8859-15/UTF-8 format with line end translation
void cdstring::ConvertFromOS()
{
	// Shortcut if empty
	if (empty())
		return;
	
	// See if ISO-8859-15 subset
	if (IsISO_8859_15_Subset())
	{
		// Do conversion to iso-8859-15
		FromUTF8ToISO();
	}
	
	// No need to filter line endings, as the ConvertToOS can cope with any format
}

cdstring cdstring::ConvertToOS(const char* str)
{
	cdstring temp(str);
	temp.ConvertToOS();
	return temp;
}

cdstring cdstring::ConvertFromOS(const char* str)
{
	cdstring temp(str);
	temp.ConvertFromOS();
	return temp;
}

// Convert a string shared by different OS's to another OS format
void cdstring::ConvertEndl(EEndl endl)
{
	// Shortcut if empty
	if (empty())
		return;

	// Now filter arbitrary line endings to local
	std::ostrstream sout;
	const char* p = c_str();
	
	while(*p)
	{
		switch(*p)
		{
		case '\r':
			p++;
			if (*p == '\n') p++;
			sout.write(get_endl(endl), get_endl_len(endl));
			break;
		case '\n':
			p++;
			sout.write(get_endl(endl), get_endl_len(endl));
			break;
		default:
			sout.put(*p++);
			break;
		}
	}
	sout << std::ends;
	
	// Grab the string
	steal(sout.str());
}

// Convert to utf8 from iso-8859-15
void cdstring::FromISOToUTF8()
{
	// Shortcut if empty
	if (empty())
		return;

	// Convert each character
	std::ostrstream sout;
	const unsigned char* p = reinterpret_cast<const unsigned char*>(_str);
	const unsigned char* q = reinterpret_cast<const unsigned char*>(_str + length());
	while(p < q)
	{
		wchar_t wp = *p;
		if (wp < 0x80)
		{
			// Write 1 to buffer
			unsigned char c = wp;
			sout.put(c);
		}
		else
		{
			// Special for Euro
			if (wp == 0x00A4)
				wp = 0x20AC;

			if (wp < 0x800)
			{
				// Write 2 to buffer
				unsigned char c = 0xc0 | (wp >> 6);
				sout.put(c);

				c = 0x80 | (wp & 0x3f);
				sout.put(c);
			}
			else // if (wp < 0x10000)
			{
				// Write 3 to buffer
				unsigned char c = 0xe0 | (wp >> 12);
				sout.put(c);

				c = 0x80 | ((wp >> 6) & 0x3f);
				sout.put(c);

				c = 0x80 | (wp & 0x3f);
				sout.put(c);
			}

		}

		// Bump ptr
		p++;

	}
	sout << std::ends;
	
	// Grab the string
	steal(sout.str());
}

// Convert from utf8 to local iso-8859-15 equivalent charset
void cdstring::FromUTF8ToISO()
{
	// Shortcut if empty
	if (empty())
		return;

	// Convert each character
	std::ostrstream sout;
	const char* p = _str;
	const char* q = _str + length();
	unsigned long charlen = 0;
	wchar_t wc = 0;
	while(p < q)
	{
		unsigned char mask = 0x3f;
		if (charlen == 0)
		{
			// Determine length of utf8 encoded wchar_t
			if ((*p & 0xf0 ) == 0xe0)
			{
				charlen = 3;
				mask = 0x0f;
			}
			else if ((*p & 0xe0 ) == 0xc0)
			{
				charlen = 2;
				mask = 0x1f;
			}
			else
			{
				charlen = 1;
				mask = 0x7f;
			}

			// Reset char
			wc = 0;
		}

		// Convert the byte
		wc <<= 6;
		wc |= (*p & mask);

		// Bump ptr
		p++;

		// Reduce byte remaining count and write it out if done
		if (!--charlen)
		{
			// Special for Euro
			if (wc == 0x20AC)
			{
				// Use iso-8859-15 code for Euro
				sout.put((char)0xA4);
				found_euro = true;
			}
			else if (wc > 0x00FF)
				sout.put('?');
			else
				sout.put(wc & 0x00FF);
		}
	}
	sout << std::ends;
	
	// Grab the string
	steal(sout.str());
}

// Check for valid UTF8
bool cdstring::IsUTF8() const
{
	const unsigned char* p = reinterpret_cast<const unsigned char*>(_str);
	while(*p)
	{
		// ascii chars are alsways utf8
		if (*p < 0x80)
		{
			p++;
		}
		else
		{
			// Check for valid lead char
			if (*p < 0xC0)
				return false;
			
			// Determine length of utf8 encoded wchar_t
			unsigned long charlen = 0;
			if ((*p & 0xf0 ) == 0xe0)
			{
				charlen = 3;
			}
			else if ((*p & 0xe0 ) == 0xc0)
			{
				charlen = 2;
			}
			p++;
			
			// Now make sure trail bytes are valid
			while(--charlen)
			{
				if ((*p++ & 0xc0) != 0x80)
					return false;
			}
		}
	}
	
	return true;
}

// Check for iso-8859-15 subset in utf8
// return true if the string contains non-ascii and they are all iso-8859-15 subset
// return false if string is all ascii or non iso-8859-15 characters
bool cdstring::IsISO_8859_15_Subset() const
{
	bool non_ascii = false;
	const unsigned char* p = reinterpret_cast<const unsigned char*>(_str);
	const unsigned char* q = p + length();
	unsigned long charlen = 0;
	wchar_t wc = 0;
	while(p < q)
	{
		unsigned char mask = 0x3f;
		if (charlen == 0)
		{
			// Determine length of utf8 encoded wchar_t
			if ((*p & 0xf0 ) == 0xe0)
			{
				charlen = 3;
				mask = 0x0f;
			}
			else if ((*p & 0xe0 ) == 0xc0)
			{
				charlen = 2;
				mask = 0x1f;
			}
			else
			{
				charlen = 1;
				mask = 0x7f;
			}

			// Reset char
			wc = 0;
		}

		// Convert the byte
		wc <<= 6;
		wc |= (*p & mask);

		// Bump ptr
		p++;

		// Reduce byte remaining count and test if done
		if (!--charlen)
		{
			if (wc > 0x007F)
			{
				// Look for non-ISO-8859-15 character
				if ((wc > 0x00FF) && (wc != 0x20AC))
					return false;
					
				// Have some valid non-ascii ISO-8859-15
				non_ascii = true;
			}
		}
	}
	
	// Ensure ascii only returns false
	return non_ascii;
}

#pragma mark ____________________________Low level

// Make string
void cdstring::_allocate(const char* buf, size_type size)
{
	_tidy();
	if (buf != NULL)
	{
		if (size == npos)
			size = ::strlen(buf);
		if (size != 0)
		{
			_str = new char[size+1];
			memcpy(_str, buf, size);
			_str[size] = 0;
		}
	}
}

// Make string
void cdstring::_allocate(char chr, size_type size)
{
	_tidy();
	if (size != 0)
	{
		_str = new char[size+1];
		::memset(_str, chr, size);
		_str[size] = 0;
	}
}

// Add buffer
void cdstring::_append(const char* buf, size_type size)
{
	if (buf != NULL)
	{
		if (size == npos)
			size = ::strlen(buf);
		if (size != 0)
		{
			char* more = new char[length() + size + 1];
			if (_str)
				strcpy(more, _str);
			else
				*more = 0;
			strncat(more, buf, size);
			steal(more);
		}
	}
}

// Add chars
void cdstring::_append(const char chr, size_type size)
{
	if (size != 0)
	{
		char* more = new char[length() + size + 1];
		if (_str)
			::strcpy(more, _str);
		else
			*more = 0;
		::memset(more + length(), chr, size);
		more[length() + size] = 0;
		steal(more);
	}
}

#pragma mark ____________________________Stream Helpers

std::istream& operator >> (std::istream& is, cdstring& str)
{
    std::istream::sentry s_ (is);
    if (s_)
    {
         int c;
         const int BSIZE = 512;
         char buf[BSIZE];
         int bcnt = 0;
         str._tidy();

         while (true)
         {
              c = is.rdbuf ()->sbumpc();

              if (c == EOF)
              {
                  is.setstate (std::ios_base::eofbit);
                  break;
              }
              else if (isspace(c))
              {
            	  is.rdbuf()->sputbackc (c);			// hh 980223 put whitespace back in stream
                  break;
              } else
              {
                  if (bcnt == BSIZE)
                  {
                      str._append (buf, cdstring::size_type(bcnt));
                      bcnt=0;
                  }
                  buf[bcnt++] = (char) c;
              }
         }
         if (bcnt != 0)
             str._append (buf, cdstring::size_type(bcnt));
    }

    return is;
}

// If delim is NULL then do arbitrary line-end lookup
std::istream& getline (std::istream& is, cdstring& str, char delim)
{
	int c;

	std::ios_base::iostate flg = std::ios_base::goodbit;  // state of istream obj.
	// Don't skipws when starting as we might have an empty line or
	// leading spaces are significant
	std::istream::sentry s_ (is, true);
	if (s_)
	{
		const int BSIZE = 512;
		char buf[BSIZE];
		int bcnt = 0;
		str._tidy();

		while (true)
		{
			c = is.rdbuf ()->sbumpc();				// try to extract a character
			if (c == EOF)
			{
				flg |= std::ios_base::eofbit;
				break;								// stop reading - eof was reached
			}

			// Look for specific or general delim
			if ((delim && (c == delim)) || (c == '\n'))
				break;								// stop reading - delim reached
			else if (c == '\r')
			{
				// Ignore LF after CR
				if (is.rdbuf ()->sgetc() == '\n')
					is.rdbuf ()->sbumpc ();
				break;
			}

			if (bcnt == BSIZE)
			{
				str._append (buf, cdstring::size_type(bcnt));	// buffer full, append to str.
				bcnt = 0;							// reset buffer pointer
			}
			buf[bcnt++] = (char) c;
		}
		if (bcnt != 0)
			str._append (buf, cdstring::size_type(bcnt));		// empty the buffer
	}


	if (flg != std::ios_base::goodbit)				// setstate is called now to avoid
		is.setstate (flg);						// throwing eof exception even when no
												// char is extracted, in which case
												// failure should be thrown.

	return is;									// is.setcount () is removed in
												// November DWP.
}
