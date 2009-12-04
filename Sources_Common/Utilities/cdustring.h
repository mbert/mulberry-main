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


// Header for cdustring class

#ifndef __CDUSTRING__MULBERRY__
#define __CDUSTRING__MULBERRY__

#include "cdstring.h"
#include "CUStringUtils.h"

#include <vector>

class cdustring;

typedef std::vector<cdustring>	cdustrvect;

class cdustring
{
	friend std::istream& operator >> (std::istream&, cdustring&);
	friend std::istream& getline (std::istream&, cdustring& str, unichar_t);
	friend std::ostream& operator << (std::ostream&, cdustring&);

public:
	typedef size_t       size_type;

	static cdustring null_str;
	static const size_type npos = static_cast<size_type>(-1);

	cdustring()															// Construct empty
		{ _init(); }
	cdustring(const cdustring& copy)									// Copy construct
		{ _init(); _allocate(copy._str); }
	cdustring(const cdustring& copy, size_type pos, size_type n = npos)
		{ _init(); _allocate(copy._str + pos, n); }
	cdustring(const unichar_t* buf, size_type size = npos)				// Construct from data
		{ _init(); _allocate(buf, size); }
	cdustring(const unichar_t c, size_type rep = 1)						// Construct from character
		{ _init(); _allocate(c, rep); }

	explicit cdustring(const cdstring& utf8)										// Construct from utf8
		{ _init(); _allocate(utf8.c_str()); }
	explicit cdustring(const char* utf8buf, size_type size = npos)				// Construct from utf8
		{ _init(); _allocate(utf8buf, size); }

#if __dest_os == __win32_os
#ifdef __MFC_STDAFX_H
#ifdef _UNICODE
	cdustring(const CString& str)							// Construct from CString
		{ _init(); _allocate((const unichar_t*) str); }
	cdustring& operator=(const CString& str)								// Assignment with CString
		{ _allocate((const unichar_t*) str); return *this; }
	cdustring& operator+=(const CString& str)							// Append CString
		{ _append((const unichar_t*) str); return *this; }
#endif
#endif
#endif

	cdustring& operator=(const cdustring& copy)							// Assignment with same type
		{ if (this != &copy) _allocate(copy._str); return *this; }
	cdustring& operator=(const unichar_t* cstr)							// Assignment with c-string
		{ _allocate(cstr); return *this; }
	cdustring& operator=(const unichar_t c)								// Assignment with character
		{ _allocate(c, 1); return *this; }

	~cdustring()
		{ _tidy(); }

	cdustring& operator+=(const cdustring& copy)						// Append same type
		{ _append(copy._str); return *this; }
	cdustring& operator+=(const unichar_t* cstr)						// Append c-string
		{ _append(cstr); return *this; }
	cdustring& operator+=(const unichar_t chr)							// Append character
		{ _append(chr, 1); return *this; }

	cdustring operator+(const cdustring& copy) const					// Add with same type
		{ cdustring add(*this); add += copy; return add; }
	cdustring operator+(const unichar_t* cstr) const					// Add with c-string
		{ cdustring add(*this); add += cstr; return add; }
	cdustring operator+(const unichar_t chr) const
		{cdustring add(*this); add += chr; return add;}
	unichar_t operator[] (size_type pos) const								// Access a character
		{ return (pos < length() ? _str[pos] : '\0'); }
	unichar_t& operator[] (size_type pos)									// Access a character
		{ return (pos < length() ? _str[pos] : _empty); }

	operator unichar_t*()												// Access the whole string
		{ return (_str ? _str : &_empty); }
	operator const unichar_t*() const									// Access the whole string
		{ return (_str ? _str : &_empty); }
	operator const unichar_t*() //to shut up g++ warning
		{ return (_str ? _str : &_empty); } 

	int operator==(const cdustring& comp) const;					// Compare with same
	int operator!=(const cdustring& comp) const						// Compare with same
		{ return !(*this == comp); }

	int operator==(const unichar_t* cstr) const;							// Compare with c-string
	int operator!=(const unichar_t* cstr) const							// Compare with c-string
		{ return !(*this == cstr); }

	int operator<(const cdustring& comp) const;						// Compare with same

	size_type length() const										// Return length
		{ return (_str ? ::unistrlen(_str) : 0); }
	void reserve(size_type res)										// Reserve space ready for ptr insert
		{ _allocate(unichar_t(0), res); }
	void clear()													// Empty the string
		{ _tidy(); }
	bool empty() const												// Is it empty
		{ return (_str ? !*_str : true); }

	const unichar_t* c_str() const										// Return string
		{ return (_str ? _str : &_empty); }
	unichar_t* c_str_mod()												// Return string for direct modification
		{ return (_str ? _str : &_empty); }
	unichar_t* grab_c_str()												// Grab ptr from this string
	{
		return release();
	}
	unichar_t* release()												// Grab ptr from this string
	{
		unichar_t* p = _str;

		// Forget old str
		_init();

		return p;
	}
	void steal(unichar_t* cstr)											// Take control of an existing string
		{ _tidy(); _str = cstr; }

	cdustring& append(const cdustring& str)
	{
		_append(str._str);
		return *this;
	}
	cdustring& append(const cdustring& str, size_type pos, size_type n)
	{
		if (pos >= str.length())
			return *this;
		_append(str._str + pos, n);
		return *this;
	}
	cdustring& append(const unichar_t* s, size_type n)
	{
		if (s)
			_append(s, n);
		return *this;
	}
	cdustring& append(const unichar_t* s)
	{
		if (s)
			_append(s, ::unistrlen(s));
		return *this;
	}
	cdustring& append(size_type n, unichar_t c)
	{
		_append(c, n);
		return *this;
	}
	cdustring& append_ascii(const char* s)
	{
		if (s)
			_append_ascii(s, ::strlen(s));
		return *this;
	}

	void push_back(unichar_t c)
	{
		append(1, c);
	}
	void pop_back()
	{
		if (_str && *_str)
		{
			_str[length() - 1] = 0;
		}
	}

	cdustring& assign(const cdustring& str)
	{
		_allocate(str._str);
		return *this;
	}
	cdustring& assign(const cdustring& str, size_type pos, size_type n)
	{
		if (pos >= str.length())
			return *this;
		_allocate(str._str + pos, n);
		return *this;
	}
	cdustring& assign(const unichar_t* s, size_type n)
	{
		if (s)
			_allocate(s, n);
		return *this;
	}
	cdustring& assign(const unichar_t* s)
	{
		if (s)
			_allocate(s, ::unistrlen(s));
		return *this;
	}
	cdustring& assign(size_type n, unichar_t c)
	{
		_allocate(c, n);
		return *this;
	}

	void trim();												// Trim storage to actual size of string

	cdustring& erase(size_type pos = 0, size_type n = npos);

	cdstring ToUTF8() const;

	void ConvertEndl(EEndl endl = eEndl_Auto);					// Convert a string shared by different OS's to another OS endl

protected:
	unichar_t*	_str;

private:
	static unichar_t _empty;

	void	_init()												// Init class
		{ _str = NULL; }
	void	_tidy()												// Delete memory
		{ delete[] _str; _str = NULL; }

	void	_allocate(const unichar_t* buf, size_type size = npos);			// Make from buffer
	void	_allocate(const unichar_t chr, size_type size);					// Make from chars
	void	_allocate(const char* buf, size_type size = npos);				// Make from utf8 buffer

	void	_append(const unichar_t* buf, size_type size = npos);			// Add buffer
	void	_append(const unichar_t chr, size_type size);					// Add chars
	void	_append_ascii(const char* buf, size_type size = npos);			// Add buffer

};


// stream helpers

std::istream& operator >> (std::istream& is, cdustring& str);
std::istream& getline (std::istream& is, cdustring& str, unichar_t delim = '\n');
std::ostream& operator << (std::ostream& os, cdustring& str);
std::ostream& operator << (std::ostream& os, const cdustring& str);

#endif
