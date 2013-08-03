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


// Source for cdustring class

#include "cdustring.h"

#include "CCharsetManager.h"
#include "cdstring.h"

#include <algorithm>
#include <strstream>
#include <stdio.h>

cdustring cdustring::null_str;
unichar_t cdustring::_empty = 0;

// Compare with same
int cdustring::operator==(const cdustring& comp) const
{
	return operator==(comp.c_str());
}

// Compare with c-string
int cdustring::operator==(const unichar_t* cstr) const
{
	if (_str && cstr)
		return (::unistrcmp(_str, cstr) == 0);
	else if (_str && !cstr)
		return *_str ? 0 : 1;
	else if (!_str && cstr)
		return *cstr ? 0 : 1;
	else
		return 1;
}

// Compare with same
int cdustring::operator<(const cdustring& comp) const
{
	if (_str && comp._str)
		return (::unistrcmp(_str, comp._str) < 0);
	else if (_str && !comp._str)
		return *_str ? 0 : 1;
	else if (!_str && comp._str)
		return *comp._str ? 1 : 0;
	else
		return 0;
}

// Trim storage to actual size of string
void cdustring::trim()
{
	if (length())
		steal(::unistrdup(_str));
}

cdustring& cdustring::erase(size_type pos, size_type n)
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
	unichar_t* p = new unichar_t[pos + len_after + 1];
	
	// Copy over the chunk before the erase
	if (pos != 0)
		::memcpy(p, _str, pos * sizeof(unichar_t));
	
	// Copy over the chunk after the erase
	if (len_after != 0)
		::memcpy(p + pos * sizeof(unichar_t), _str + pos * sizeof(unichar_t) + n * sizeof(unichar_t), len_after * sizeof(unichar_t));
	p[pos + len_after] = 0;
	steal(p);
	
	return *this;
}

cdstring cdustring::ToUTF8() const
{
	std::ostrstream sout;
	i18n::CCharsetManager::sCharsetManager.ToUTF8(i18n::eUTF16, (const char*)_str, length() * 2, sout);
	sout << std::ends;
	cdstring result;
	result.steal(sout.str());
	return result;
}

// Convert a string shared by different OS's to another OS format
void cdustring::ConvertEndl(EEndl endl)
{
	// Shortcut if empty
	if (empty())
		return;

	// Now filter arbitrary line endings to local
	std::ostrstream out;
	const unichar_t* p = c_str();
	
	while(*p)
	{
		switch(*p)
		{
		case '\r':
			p++;
			if (*p == '\n') p++;
			out.write((const char*)get_uendl(endl), get_endl_len(endl) * sizeof(unichar_t));
			break;
		case '\n':
			p++;
			out.write((const char*)get_uendl(endl), get_endl_len(endl) * sizeof(unichar_t));
			break;
		default:
			out.write((const char*)p, sizeof(unichar_t));
			p++;
			break;
		}
	}
	out << std::ends << std::ends;
	
	// Grab the string
	steal((unichar_t*)out.str());
}

#pragma mark ____________________________Low level

// Make string
void cdustring::_allocate(const unichar_t* buf, size_type size)
{
	_tidy();
	if (buf != NULL)
	{
		if (size == npos)
			size = ::unistrlen(buf);
		if (size != 0)
		{
			_str = new unichar_t[size+1];
			::unistrncpy(_str, buf, size);
			_str[size] = 0;
		}
	}
}

// Make string
void cdustring::_allocate(unichar_t chr, size_type size)
{
	_tidy();
	if (size != 0)
	{
		_str = new unichar_t[size+1];
		unichar_t* p = _str;
		unichar_t* q = p + size;
		while(p < q)
			*p++ = chr;
		_str[size] = 0;
	}
}

// Make from utf8
void cdustring::_allocate(const char* utf8buf, size_type size)
{
	_tidy();
	if (utf8buf != NULL)
	{
		if (size == npos)
			size = ::strlen(utf8buf);
		if (size != 0)
		{
			std::ostrstream sout;
			i18n::CCharsetManager::sCharsetManager.FromUTF8(utf8buf, size, sout);
			sout << std::ends << std::ends;
			steal((unichar_t*)sout.str());
		}
	}
}


// Add buffer
void cdustring::_append(const unichar_t* buf, size_type size)
{
	if (buf != NULL)
	{
		if (size == npos)
			size = ::unistrlen(buf);
		if (size != 0)
		{
			unichar_t* more = new unichar_t[length() + size + 1];
			if (_str)
				::unistrcpy(more, _str);
			else
				*more = 0;
			::unistrncat(more, buf, size);
			steal(more);
		}
	}
}

// Add chars
void cdustring::_append(const unichar_t chr, size_type size)
{
	if (size != 0)
	{
		unichar_t* more = new unichar_t[length() + size + 1];
		if (_str)
			::unistrcpy(more, _str);
		else
			*more = 0;
		unichar_t* p = more + length();
		unichar_t* q = p + size;
		while(p < q)
			*p++ = chr;
		more[length() + size] = 0;
		steal(more);
	}
}

// Add buffer
void cdustring::_append_ascii(const char* buf, size_type size)
{
	if (buf != NULL)
	{
		if (size == npos)
			size = ::strlen(buf);
		if (size != 0)
		{
			unichar_t* more = new unichar_t[length() + size + 1];
			if (_str)
				::unistrcpy(more, _str);
			else
				*more = 0;
			::unistrncat(more, buf, size);
			steal(more);
		}
	}
}

#pragma mark ____________________________Stream Helpers

std::istream& operator >> (std::istream& is, cdustring& str)
{
    std::istream::sentry s_ (is);
    if (s_)
    {
         int c1;
         int c2;
         const int BSIZE = 512;
         unichar_t buf[BSIZE];
         int bcnt = 0;
         str._tidy();

         while (true)
         {
              c1 = is.rdbuf ()->sbumpc();
              if (c1 == EOF)
              {
                  is.setstate (std::ios_base::eofbit);
                  break;
              }
              c2 = is.rdbuf ()->sbumpc();
              if (c2 == EOF)
              {
                  is.setstate (std::ios_base::eofbit);
                  break;
              }
			  unichar_t c = ((c1 & 0xFF) << 8) | (c2 & 0xFF);

              if (isuspace(c))
              {
            	  is.rdbuf()->sputbackc (c2);			// hh 980223 put whitespace back in stream
            	  is.rdbuf()->sputbackc (c1);			// hh 980223 put whitespace back in stream
                  break;
              } else
              {
                  if (bcnt == BSIZE)
                  {
                      str._append (buf, cdustring::size_type(bcnt));
                      bcnt=0;
                  }
                  buf[bcnt++] = c;
              }
         }
         if (bcnt != 0)
             str._append (buf, cdustring::size_type(bcnt));
    }

    return is;
}

// If delim is NULL then do arbitrary line-end lookup
std::istream& getline (std::istream& is, cdustring& str, unichar_t delim)
{
	int c1;
	int c2;

	std::ios_base::iostate flg = std::ios_base::goodbit;  // state of istream obj.
	// Don't skipws when starting as we might have an empty line or
	// leading spaces are significant
	std::istream::sentry s_ (is, true);
	if (s_)
	{
		const int BSIZE = 512;
		unichar_t buf[BSIZE];
		int bcnt = 0;
		str._tidy();
		bool have_c1 = false;

		while (true)
		{
			if (!have_c1)
				c1 = is.rdbuf ()->sbumpc();				// try to extract a character
			else
				have_c1 = false;
			if (c1 == EOF)
			{
				flg |= std::ios_base::eofbit;
				break;								// stop reading - eof was reached
			}
			c2 = is.rdbuf ()->sbumpc();				// try to extract a character
			if (c2 == EOF)
			{
				flg |= std::ios_base::eofbit;
				break;								// stop reading - eof was reached
			}
			unichar_t c = ((c1 & 0xFF) << 8) | (c2 & 0xFF);

			// Look for specific or general delim
			if ((delim && (c == delim)) || (c == '\n'))
				break;								// stop reading - delim reached
			else if (c == '\r')
			{
				// Ignore LF after CR
				c1 = is.rdbuf ()->sbumpc();
				if ((c1 == 0) && (is.rdbuf ()->sgetc() == '\n'))
					is.rdbuf ()->sbumpc ();
				break;
			}

			if (bcnt == BSIZE)
			{
				str._append (buf, cdustring::size_type(bcnt));	// buffer full, append to str.
				bcnt = 0;							// reset buffer pointer
			}
			buf[bcnt++] = c;
		}
		if (bcnt != 0)
			str._append (buf, cdustring::size_type(bcnt));		// empty the buffer
	}


	if (flg != std::ios_base::goodbit)				// setstate is called now to avoid
		is.setstate (flg);						// throwing eof exception even when no
												// char is extracted, in which case
												// failure should be thrown.

	return is;									// is.setcount () is removed in
												// November DWP.
}

std::ostream& operator << (std::ostream& os, cdustring& str)
{
     return os.write((const char*)str.c_str(), str.length() * sizeof(unichar_t));
}

std::ostream& operator << (std::ostream& os, const cdustring& str)
{
     return os.write((const char*)str.c_str(), str.length() * sizeof(unichar_t));
}
