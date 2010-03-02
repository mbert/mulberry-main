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


// Header for cdstring class

#ifndef __CDSTRING__MULBERRY__
#define __CDSTRING__MULBERRY__

#include <string.h>
#include <stdint.h>

#include <deque>
#include <queue>
#include <stack>
#include <map>
#if __GNUC__ == 4
//#include <multimap.h>
#endif
#include <ostream>
#include <set>
#if __GNUC__ == 4
//#include <multiset.h>
#endif
#include <string>
#include <vector>

#include "CCharsetCodes.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
#include <LString.h>
#include <SysCFString.h>
#endif
#include <Carbon/Carbon.h>
#endif

class cdstring;
class cdustring;

typedef std::vector<cdstring> cdstrvect;				// Use small vector
typedef std::deque<cdstring> cdstrdeque;
typedef std::queue<cdstring, cdstrdeque> cdstrqueue;
//typedef stack<cdstrvect, cdstrdeque> cdstrstack;
typedef std::pair<cdstring, cdstring> cdstrpair;
typedef std::vector<cdstrpair> cdstrpairvect;			// Use small vector
typedef std::pair<cdstring, bool> cdstrbool;
typedef std::vector<cdstrbool> cdstrboolvect;			// Use small vector
typedef std::pair<cdstring, long> cdstrlong;
typedef std::vector<cdstrlong> cdstrlongvect;			// Use small vector
typedef std::map<cdstring, cdstring> cdstrmap;
typedef std::map<cdstring, uint32_t> cdstruint32map;
typedef std::multimap<cdstring, cdstring> cdstrmultimap;
typedef std::set<cdstring> cdstrset;
typedef std::multiset<cdstring> cdstrmultiset;

class char_stream;

class cdstring
{
	friend std::istream& operator >> (std::istream&, cdstring&);
	friend std::istream& getline (std::istream&, cdstring& str, char);
	friend std::ostream& operator << (std::ostream&, cdstring&);

public:
	enum EStringEncrypt
	{
		eEncryptSimple,
		eEncryptSimplemUTF7,
		eEncryptCipher
	};

	typedef size_t       size_type;

	static cdstring null_str;
	static const size_type npos = static_cast<size_type>(-1);

	cdstring()												// Construct empty
		{ _init(); }
	cdstring(const cdstring& copy)							// Copy construct
		{ _init(); _allocate(copy._str); }
	cdstring(const cdstring& copy, size_type pos, size_type n = npos)
		{ _init(); _allocate(copy._str + pos, n); }
	cdstring(const std::string& copy)								// Construct from MSL string
		{ _init(); _allocate(copy.data(), copy.length()); }
	cdstring(const char* buf, size_type size)					// Construct from data
		{ _init(); _allocate(buf, size); }
	cdstring(const char* cstr)											// Construct from c-string
		{ _init(); _allocate(cstr); }
	cdstring(const unsigned char* pstr)						// Construct from p-string
		{ _init(); if (pstr) _allocate((const char*) &pstr[1], *pstr); }
	cdstring(const char c, size_type rep = 1)					// Construct from character
		{ _init(); _allocate(c, rep); }
	cdstring(const long num);									// Construct from number
	cdstring(const unsigned long num);							// Construct from number
	cdstring(const int32_t num);								// Construct from number
	cdstring(const uint32_t num);								// Construct from number
	cdstring(const Rect& rc);									// Construct from rectangle
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
	cdstring(const LString& pstr)								// Construct from LString
		{ _init(); _allocate(pstr.ConstTextPtr(), pstr.Length()); }
	cdstring(const PPx::CFString& cfstr);							// Construct from CFString
#endif
#endif

	cdstring& operator=(const cdstring& copy)							// Assignment with same type
		{ if (this != &copy) _allocate(copy._str); return *this; }
	cdstring& operator=(const std::string& copy)								// Assignment with MSL string
		{ _allocate(copy.data(), copy.length()); return *this; }
	cdstring& operator=(const char* cstr)								// Assignment with c-string
		{ _allocate(cstr); return *this; }
	cdstring& operator=(const unsigned char* pstr)						// Assignment with p-string
		{ if (pstr) _allocate((const char*) &pstr[1], *pstr); return *this; }

	cdstring& operator=(const char c)									// Assignment with character
		{ _allocate(c, 1); return *this; }
	cdstring& operator=(const long num);								// Convert number to string
	cdstring& operator=(const unsigned long num);						// Convert number to string
	cdstring& operator=(const int32_t num);								// Convert number to string
	cdstring& operator=(const uint32_t num);							// Convert number to string
	cdstring& operator=(const Rect& rc);								// Convert rectangle to string
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
	cdstring& operator=(const LString& pstr)							// Assignment with LString
		{ _allocate(pstr.ConstTextPtr(), pstr.Length()); return *this; }
#endif
#endif

	~cdstring()
		{ _tidy(); }

	cdstring& operator+=(const cdstring& copy)						// Append same type
		{ _append(copy._str); return *this; }
	cdstring& operator+=(const char* cstr)							// Append c-string
		{ _append(cstr); return *this; }
	cdstring& operator+=(const unsigned char* pstr)					// Append p-string
		{ _append((const char*) &pstr[1], *pstr); return *this; }
	cdstring& operator+=(const char chr)								// Append character
		{ _append(chr, 1); return *this; }

	// Window CString handling
#if __dest_os == __win32_os
#ifdef __MFC_STDAFX_H

#ifdef _UNICODE
	cdstring(const CString& str);										// Construct from CString
	cdstring(const TCHAR* str);											// Construct from CString
	cdstring& operator=(const CString& str);							// Assignment with CString
	cdstring& operator=(const TCHAR* str);								// Assignment with CString
	cdstring& operator+=(const CString& str);							// Append CString
	cdstring& operator+=(const TCHAR* str);								// Append CString
#else
	cdstring(const CString& str)										// Construct from CString
		{ _init(); _allocate((const char*) str); }
	cdstring& operator=(const CString& str)								// Assignment with CString
		{ _allocate((const char*) str); return *this; }
	cdstring& operator+=(const CString& str)							// Append CString
		{ _append((const char*) str); return *this; }
#endif

	CString win_str() const;											// Access the whole string

#else

#ifdef _UNICODE
	cdstring(const TCHAR* str);											// Construct from CString
	cdstring& operator=(const TCHAR* str);								// Assignment with CString
	cdstring& operator+=(const TCHAR* str);								// Append CString

	cdustring win_str() const;											// Access the whole string
#endif
#endif
#endif

	cdstring operator+(const cdstring& copy) const						// Add with same type
		{ cdstring add(*this); add += copy; return add; }
	cdstring operator+(const char* cstr) const							// Add with c-string
		{ cdstring add(*this); add += cstr; return add; }
	cdstring operator+(const char chr) const
		{cdstring add(*this); add += chr; return add;}
	char operator[] (size_type pos) const								// Access a character
		{ return (pos < length() ? _str[pos] : '\0'); }
	char& operator[] (size_type pos)									// Access a character
		{ return (pos < length() ? _str[pos] : _empty); }

	operator char*()												// Access the whole string
		{ return (_str ? _str : &_empty); }
	operator const char*() const									// Access the whole string
		{ return (_str ? _str : &_empty); }
	operator const char*() //to shut up g++ warning
		{ return (_str ? _str : &_empty); } 

	int operator==(const cdstring& comp) const;						// Compare with same
	int operator!=(const cdstring& comp) const						// Compare with same
		{ return !(*this == comp); }

	int operator==(const char* cstr) const;							// Compare with c-string
	int operator!=(const char* cstr) const							// Compare with c-string
		{ return !(*this == cstr); }

	int operator==(const unsigned char* pstr) const;				// Compare with p-string
	int operator!=(const unsigned char* pstr) const					// Compare with p-string
		{ return !(*this == pstr); }

	int operator<(const cdstring& comp) const;						// Compare with same

	size_type length() const										// Return length
		{ return (_str ? ::strlen(_str) : 0); }
	void reserve(size_type res)										// Reserve space ready for ptr insert
		{ _allocate('\0', res); }
	void clear()													// Empty the string
		{ _tidy(); }
	bool empty() const												// Is it empty
		{ return (_str ? !*_str : true); }

	const char* c_str() const										// Return string
		{ return (_str ? _str : &_empty); }
	char* c_str_mod()												// Return string for direct modification
		{ return (_str ? _str : &_empty); }
	char* grab_c_str()												// Grab ptr from this string
	{
		return release();
	}
	char* release()													// Grab ptr from this string
	{
		char* p = _str;

		// Forget old str
		_init();

		return p;
	}
	void steal(char* cstr)											// Take control of an existing string
		{ _tidy(); _str = cstr; }

	cdstring& append(const cdstring& str)
	{
		_append(str._str);
		return *this;
	}
	cdstring& append(const cdstring& str, size_type pos, size_type n)
	{
		if (pos >= str.length())
			return *this;
		_append(str._str + pos, n);
		return *this;
	}
	cdstring& append(const char* s, size_type n)
	{
		if (s)
			_append(s, n);
		return *this;
	}
	cdstring& append(const char* s)
	{
		if (s)
			_append(s, npos);
		return *this;
	}
	cdstring& append(size_type n, char c)
	{
		_append(c, n);
		return *this;
	}

	void push_back(char c)
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

	cdstring& assign(const cdstring& str)
	{
		_allocate(str._str);
		return *this;
	}
	cdstring& assign(const cdstring& str, size_type pos, size_type n)
	{
		if (pos >= str.length())
			return *this;
		_allocate(str._str + pos, n);
		return *this;
	}
	cdstring& assign(const char* s, size_type n)
	{
		if (s)
			_allocate(s, n);
		return *this;
	}
	cdstring& assign(const char* s)
	{
		if (s)
			_allocate(s, npos);
		return *this;
	}
	cdstring& assign(size_type n, char c)
	{
		_allocate(c, n);
		return *this;
	}

	void trim();												// Trim storage to actual size of string
	void trimspace();											// Trim off leading or trailing space

	cdstring& erase(size_type pos = 0, size_type n = npos);

	// find
	size_type find(const cdstring& str, size_type pos = 0, bool casei = false) const
	{
		return find(str._str, pos, str.length(), casei);
	}
	size_type find(const char* s, size_type pos, size_type n, bool casei = false) const;
	size_type find(const char* s, size_type pos = 0, bool casei = false) const
	{
		return find(s, pos, ::strlen(s), casei);
	}
	size_type find(char c, size_type pos = 0, bool casei = false) const;

	// rfind
	size_type rfind(const cdstring& str, size_type pos = npos, bool casei = false) const
	{
		return rfind(str._str, pos, str.length(), casei);
	}
	size_type rfind(const char* s, size_type pos, size_type n, bool casei = false) const;
	size_type rfind(const char* s, size_type pos = npos, bool casei = false) const
	{
		return rfind(s, pos, ::strlen(s), casei);
	}
	size_type rfind(char c, size_type pos = npos, bool casei = false) const;

	// find_first_of
	size_type find_first_of(const cdstring& str, size_type pos = 0) const
	{
		return find_first_of(str._str, pos, str.length());
	}
	size_type find_first_of(const char* s, size_type pos, size_type n) const;
	size_type find_first_of(const char* s, size_type pos = 0) const
	{
		return find_first_of(s, pos, ::strlen(s));
	}
	size_type find_first_of(char c, size_type pos = 0) const
	{
		return find(c, pos);
	}

	// find_last_of
	size_type find_last_of(const cdstring& str, size_type pos = npos) const
	{
		return find_last_of(str._str, pos, str.length());
	}
	size_type find_last_of(const char* s, size_type pos, size_type n) const;
	size_type find_last_of(const char* s, size_type pos = npos) const
	{
		return find_last_of(s, pos, ::strlen(s));
	}
	size_type find_last_of(char c, size_type pos = npos) const
	{
		return rfind(c, pos);
	}

	// find_first_not_of
	size_type find_first_not_of(const cdstring& str, size_type pos) const
	{
		return find_first_not_of(str._str, pos, str.length());
	}
	size_type find_first_not_of(const char* s, size_type pos, size_type n) const;
	size_type find_first_not_of(const char* s, size_type pos = 0) const
	{
		return find_first_not_of(s, pos, ::strlen(s));
	}
	size_type find_first_not_of(char c, size_type pos) const;

	// find_last_not_of
	size_type find_last_not_of(const cdstring& str, size_type pos = npos) const
	{
		return find_last_not_of(str._str, pos, str.length());
	}
	size_type find_last_not_of(const char* s, size_type pos, size_type n) const;
	size_type find_last_not_of(const char* s, size_type pos = npos) const
	{
		return find_last_not_of(s, pos, ::strlen(s));
	}
	size_type find_last_not_of(char c, size_type pos = npos) const;

	// compare

	int compare(const cdstring& str, bool casei = false) const
	{
		return compare(0, length(), str._str, str.length(), casei);
	}
	int compare(size_type pos1, size_type n1, const cdstring& str, bool casei = false) const
	{
		return compare(pos1, n1, str._str, str.length(), casei);
	}
	int compare(size_type pos1, size_type n1, const cdstring& str, size_type pos2, size_type n2, bool casei = false) const
	{
		return compare(pos1, n1, str._str + pos2, n2, casei);
	}
	int compare(const char* s, bool casei = false) const
	{
		return compare(0, length(), s, ::strlen(s), casei);
	}
	int compare(size_type pos1, size_type n1, const char* s, bool casei = false) const
	{
		return compare(pos1, n1, s, ::strlen(s), casei);
	}
	int compare(size_type pos1, size_type n1, const char* s, size_type n2, bool casei = false) const;

	bool compare_start(const char* comp, bool casei = false) const;			// Compare at start
	bool compare_start(const cdstring& comp, bool casei = false) const		// Compare at start
		{ return compare_start(comp._str, casei); }
	bool compare_end(const char* comp, bool casei = false) const;			// Compare at end
	bool compare_end(const cdstring& comp, bool casei = false) const		// Compare at end
		{ return compare_end(comp._str, casei); }

	// other bits

	bool isquoted() const;										// Is string quoted
	bool quote(bool force = false, bool inet = false)			// Add quotes if not already there
		{
			return quote(NULL, force, inet);
		}
	bool quote(const char* specials, bool force = false, bool inet = false);	// Add quotes if not already there
	bool unquote();												// Remove quotes if there

	void split(const char* tokens, cdstrvect& results, bool trim = true) const;			// Split into tokens
	void join(const cdstrvect& items, const char* delim);		// Join list into single string

	void FilterOutEscapeChars();								// Filter out C-style escape characters
	void FilterInEscapeChars(const char* charset);				// Filter in C-style escape characters

	void EncodeURL(char ignore = 0);							// Encode to URL
	void DecodeURL();											// Decode from URL

	cdstring& FromResource(long res_id, long index = 0);		// Load from resource
	cdstring& FromResource(const char* rsrcid);					// Load from XML resource
	void AppendResource(long res_id, long index = 0);			// Append from resource
	void AppendResource(const char* rsrcid);					// Append from XML resource

	void Substitute(const cdstring& str);						// snprintf %s substitute
	void Substitute(unsigned long ul);							// snprintf %ld substitute
	void Substitute(long ul);									// snprintf %ld substitute

	void ParseSExpression(cdstrvect& list) const;					// Parse S-Expression into vector
	static void ParseSExpression(char_stream& txt, cdstrvect& list, bool convert);		// Parse S-Expression into vector
	void CreateSExpression(const cdstrvect& list,
							bool old_style = false);				// Create S_Expression from vector
	void ParseSExpression(cdstrpairvect& list) const;				// Parse S-Expression into vector
	static void ParseSExpression(char_stream& txt, cdstrpairvect& list, bool convert);				// Parse S-Expression into vector
	void CreateSExpression(const cdstrpairvect& list);				// Create S_Expression from vector

	void ParseSExpression(cdstrmap& map) const;						// Parse S-Expression into map
	static void ParseSExpression(char_stream& txt, cdstrmap& map, bool convert);					// Parse S-Expression into map
	void CreateSExpression(const cdstrmap& map);					// Create S_Expression from map

	void ParseSExpression(cdstrset& set) const;						// Parse S-Expression into set
	static void ParseSExpression(char_stream& txt, cdstrset& set, bool convert);					// Parse S-Expression into set
	void CreateSExpression(const cdstrset& set);					// Create S_Expression from set

	static void FromArray(const char** txt, cdstrvect& list, bool unique = false);	// Convert array of pointers to list
	static const char** ToArray(const cdstrvect& list, bool copy = true);			// Convert list to NULL terminated array of pointers
	static void FreeArray(const char** txt);										// Delete all items in array and array itself

	bool PatternMatch(const cdstring& pattern) const;				// Match pattern
	bool PatternDirMatch(const cdstring& pattern,
								char separator) const;				// Match hierarchy pattern

	void Encrypt(EStringEncrypt method, const char* key = NULL);		// Encrypt
	void Decrypt(EStringEncrypt method, const char* key = NULL);		// Decrypt
	void md5(unsigned long& digest) const;							// Generate md5 hash
	void md5(cdstring& digesthex) const;							// Generate md5 hash
	void md5(unsigned char digest[16]) const;						// Generate md5 hash
	unsigned long hash() const;										// Basic string hash
	static unsigned long hash(const char* str);						// Basic string hash

	void ToModifiedUTF7(bool charset);							// Convert to modified-UTF7 (for IMAP4rev1)
	void FromModifiedUTF7(bool charset)							// Convert from modified-UTF7 (for IMAP4rev1)
		{ if (_str) steal(FromModifiedUTF7(_str, charset)); }

	static char* ToModifiedUTF7(const char* str, bool charset);	// Convert to modified-UTF7 (for IMAP4rev1)
	static char* FromModifiedUTF7(char* str, bool charset);		// Convert from modified-UTF7 (for IMAP4rev1)

	void ConvertToOS();											// Convert a string in either ISO-8859-15 or UTF-8 format into utf8 with line end translation
	void ConvertFromOS();										// Convert a utf8 string to the external ISO-8859-15/UTF-8 format with line end translation

	void ConvertEndl(EEndl endl = eEndl_Auto);					// Convert a string shared by different OS's to another OS endl

	static cdstring ConvertToOS(const char* str);
	static cdstring ConvertFromOS(const char* str);

	void FromISOToUTF8();										// Convert iso-8859-15 to utf8
	void FromUTF8ToISO();										// Convert from utf8 to local iso-8859-15 equivalent charset
	bool IsUTF8() const;										// Check for valid UTF8
	bool IsISO_8859_15_Subset() const;							// Check for iso-8859-15 subset in utf8

protected:
	char*	_str;

private:
	static char _empty;

	void	_init()												// Init class
		{ _str = NULL; }
	void	_tidy()												// Delete memory
		{ delete[] _str; _str = NULL; }

	void	_allocate(const char* buf, size_type size = npos);			// Make from buffer
	void	_allocate(char chr, size_type size);						// Make from chars

	void	_append(const char* buf, size_type size = npos);			// Add buffer
	void	_append(const char chr, size_type size);					// Add chars

};

// stream helpers

std::istream& operator >> (std::istream& is, cdstring& str);
std::istream& getline (std::istream& is, cdstring& str, char delim = '\n');
inline std::ostream& operator << (std::ostream& os, cdstring& str)
{
     return os.write(str.c_str(), str.length());
}
inline std::ostream& operator << (std::ostream& os, const cdstring& str)
{
     return os.write(str.c_str(), str.length());
}

// Case insensitive maps
struct case_insensitive_cdstring
{
	bool operator() (const cdstring& s1, const cdstring& s2) const
	{
		return s1.compare(s2, true) < 0;
	}
};
typedef std::multimap<cdstring, cdstring, case_insensitive_cdstring>	cdstrmultimapcasei;

#endif
