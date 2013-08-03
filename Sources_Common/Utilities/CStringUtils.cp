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


// Source for common utilities

#include "CStringUtils.h"

#include <ctype.h>

// S T R I N G  F U N C T I O N S

#if !defined(__GNUC__) && !defined(__VCPP__)
// Duplicate string
char* strdup(const char* s1)
{
	if (s1 && *s1)
	{
		char* s2 = new char[::strlen(s1) + 1];
		::strcpy(s2, s1);
		return s2;
	}
	else
		return NULL;
}
#endif

#if !defined(__GNUC__) || __dest_os == __mac_os_x
// Duplicate string a fixed length
char* strndup(const char* s1, size_t len)
{
	if (s1 && *s1 && len)
	{
		char* s2 = new char[len + 1];
		::strncpy(s2, s1, len);
		s2[len] = 0;
		return s2;
	}
	else
		return NULL;
}
#endif

// Convert to lowercase
void strlower(char* s1)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
		{
			*s1 = tolower(*s1);
			s1++;
		}
#else
	if (s1--)
		while(*++s1)
			*s1 = tolower(*s1);
#endif
}

// Convert to uppercase
void strupper(char* s1)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
		{
			*s1 = toupper(*s1);
			s1++;
		}
#else
	if (s1--)
		while(*++s1)
			*s1 = toupper(*s1);
#endif
}

// Replace specific chars with another
void strreplace(char* s1, const char* s2, char c)
{
	char* p = s1;
	while(p && *p && ((p = strpbrk(p, s2)) != NULL))
		*p++ = c;
}

// Replace spaces
void strreplacespace(char* s1, char c)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
		{
			if (*s1 == ' ') *s1 = c;
			s1++;
		}
#else
	if (s1--)
		while(*++s1)
			if (*s1 == ' ') *s1 = c;
#endif
}

// Replace with space
void strplacespace(char* s1, char c)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
		{
			if (*s1 == c) *s1 = ' ';
			s1++;
		}
#else
	if (s1--)
		while(*++s1)
			if (*s1 == c) *s1 = ' ';
#endif
}

// Compare without case
int strcmpnocase(const char* s1,const char* s2)
{
	if (!s1 || !s2)
		return s1 - s2;

#if !__POWERPC__

	const unsigned char * p1 = (unsigned char *) s1;
	const unsigned char * p2 = (unsigned char *) s2;
	unsigned char c1, c2;

	while ((c1 = tolower(*p1++)) == (c2 = tolower(*p2++)))
		if (!c1)
			return 0;

#else

	const unsigned char * p1 = (unsigned char *) s1 - 1;
	const unsigned char * p2 = (unsigned char *) s2 - 1;
	unsigned long c1, c2;

	while ((c1 = tolower(*++p1)) == (c2 = tolower(*++p2)))
		if (!c1)
			return 0;

#endif

	return c1 - c2;
}

// Compare without case
int strncmpnocase(const char* s1,const char* s2, size_t n)
{
	if (!s1 || !s2)
		return s1 - s2;

#if !__POWERPC__

	const unsigned char * p1 = (unsigned char *) s1;
	const unsigned char * p2 = (unsigned char *) s2;
	unsigned char c1, c2;

	n++;

	while (--n)
		if ((c1 = tolower(*p1++)) != (c2 = tolower(*p2++)))
			return c1 - c2;
		else if (!c1)
			break;

#else

	const unsigned char * p1 = (unsigned char *) s1 - 1;
	const unsigned char * p2 = (unsigned char *) s2 - 1;
	unsigned long c1, c2;

	n++;

	while (--n)
		if ((c1 = tolower(*++p1)) != (c2 = tolower(*++p2)))
			return c1 - c2;
		else if (!c1)
			break;

#endif

	return 0;
}

// Get token with advance
char* stradvtok(char** s1, const char* s2)
{
	if (!s1 || !s2)
		return NULL;

	// If nothing left then return
	if (!**s1) return NULL;

	// Advance beyond any token separator
	char* s3 = *s1 + strspn(*s1, s2);

	// Find end of token
	char* s4 = strpbrk(s3, s2);

	// Found token separator at end
	if (s4)
	{
		// Tie off token
		*s4 = 0;

		// Advance past token
		*s1 = ++s4;
	}
	else
		// Advance to end of string
		*s1 = s3 + strlen(s3);

	return s3;
}

// Compare a token with a string and advance token pointer
int stradvtokcmp(char** s1, const char* s2)
{
	if (!s1 || !s2)
		return -1;

	// Step over white space & CRLF
	while((**s1 == ' ') || (**s1 == '\r') || (**s1 == '\n')) (*s1)++;

	// If nothing left then return
	if (!**s1) return -1;

	size_t len2 = strlen(s2);

	// Look for break character at end of test string
	char c = *((*s1) + len2);
	if ((c == ' ') || (c == '\r') || (c == '\n') || (c == 0))
	{
		// Does token compare with test string
		int cmp = strncmpnocase(*s1, s2, len2);

		// If equal advance pointer
		if (cmp == 0)
			*s1 += len2 + (c ? 1 : 0);

		return cmp;
	}
	else
		return -1;
}

// Compare a test string with a string and advance string pointer if equal
int stradvstrcmp(char** s1, const char* s2)
{
	if (!s1 || !s2)
		return -1;

	// Does string compare with test string
	size_t len = strlen(s2);
	int cmp = strncmpnocase(*s1, s2, len);

	// If equal advance pointer
	if (cmp == 0)
		*s1 += len;

	return cmp;
}

// Match left & right brackets and terminate string
// Returns string inside bracket, advance original past bracketed text
// Returns NULL if it does not start with a left bracket (do not change original ptr)
// Returns NULL if there is no matching right bracket (do not change original ptr)
// NB could fail if text contains a quoted string with single left or right bracket
char* strmatchbra(char** s1)
{
	if (!s1)
		return NULL;

	char*	str =* s1;
	char*	start_rtn;
	char	l_bracket,r_bracket;
	unsigned long	level_ctr = 0;

	// Step over white space
	while(*str == ' ') str++;

	// Store bracket type
	l_bracket = *str;

	// Point to text after bracket
	start_rtn = str+1;

	// Determine right bracket
	r_bracket = ::getbracketmatch(l_bracket);

	// No match is error
	if (!r_bracket) return NULL;

	while(*str)
	{
		// Check current char
		if (*str == l_bracket)

			// Advance level counter if left bracket
			level_ctr++;

		else if (*str == r_bracket)

			// Decrease level counter if right bracket
			level_ctr--;

		// Check whether closure achieved
		if (level_ctr==0)
		{
			// Terminate string here and update original value
			*str = 0;
			*s1 = ++str;

			// Return ptr to next bit of string
			return start_rtn;
		}

		// Move to next char
		str++;
	}

	// String ends before level zero - its an error
	return NULL;
}

// Return char matching left bracket
char getbracketmatch(char bracket)
{
	switch(bracket)
	{
		case '(':
			return ')';

		case '{':
			return '}';

		case '[':
			return ']';

		case '<':
			return '>';

		// Any other character is an error
		default:
			return 0;
	}

}

// Get possibly bracketed string
char* strgetbrastr(char** s1)
{
	if (!s1)
		return NULL;

	char* start = *s1;

	// Remove leading space
	while(*start == ' ') start++;

	// Ignore null strs
	if (!*start) return NULL;

	// Get bracketed string if bracketed
	start = ::strmatchbra(s1);

	// If not bracketed just get atom
	if (!start)
	{
		start = *s1;

		// Find first space or end
		char* end = ::strpbrk(start,WHITE_SPACE);

		// Terminate string at space (advance past it) or point to end of atom
		if (end)
		{
			*end = 0;
			end++;
		}
		else
			end = start + ::strlen(start);
		*s1 = end;
	}

	return start;
}

// Get possibly quoted string
char* strgetquotestr(char** s1, bool unescape)
{
	return ::strgettokenstr(s1, WHITE_SPACE_BRACKETS, unescape);
}

// Get possibly quoted string
char* strgettokenstr(char** s1, const char* tokens, bool unescape)
{
	if (!s1 || !tokens)
		return NULL;

	char*	start = *s1;
	char*	end;

	// Remove leading space
	while(*start == ' ') start++;

	// Ignore null strs
	if (!*start) return NULL;

	// Handle quoted string
	if (*start=='"')
	{
		end = ++start;

		// Advance string ignoring quoted
		while(*end != '\"')
		{
			switch(*end)
			{
			case 0:
				// Nothing left so error
				return NULL;
			case '\\':
				if (unescape)
					end += 2;
				else
					end++;
				break;
			default:
				end++;
			}
		}

		// Terminate string at enclosing quote and adjust rtn ptr past it
		*end = 0;
		*s1 = ++end;

		// Must unescape always!
		if (unescape)
			::FilterOutEscapeChars(start);

		// Start past first quote
		return start;
	}

	// Handle unquoted atom
	else
	{
		end = start;

		// Find first space, bracket or end
		end = ::strpbrk(end, tokens);

		// Terminate string at space (advance past it) or point to end of atom
		if (end)
		{
			*end = 0;
			end++;
		}
		else
			end = start + ::strlen(start);
		*s1 = end;

		return start;
	}
}

// Duplicate possibly quoted string without destroying character after atom
char* strdupquotestr(char** s1)
{
	return ::strduptokenstr(s1, WHITE_SPACE_BRACKETS);
}

// Duplicate possibly quoted string without destroying character after atom
char* strduptokenstr(char** s1, const char* tokens)
{
	if (!s1 || !tokens)
		return NULL;

	char*	start = *s1;

	// Remove leading space
	while(*start == ' ') start++;

	// Handle quoted string
	if (*start=='"')
	{
		char* end = ++start;

		// NB ACAP work requires that this strip quotes from the string

#if 0
		// Advance string ignoring '\"'
		while((*end != '"') || (*(end-1) == '\\'))
		{
			if (!*end)
				// Nothing left so error
				return NULL;

			end++;
		}
#else
		char* copy = end;
		bool done = false;
		while(!done)
		{
			switch(*end)
			{
			case '\"':
				*copy = 0;
				done = true;
				break;
			case '\\':
				// Punt past quote
				end++;
				if (!*end)
					return NULL;
				
				// Copy the quoted char
				*copy++ = *end++;
				break;
			case 0:
				// Nothing left so error
				return NULL;
			default:
				*copy++ = *end++;
				break;
			}
		}
#endif

		// Terminate string at enclosing quote and adjust rtn ptr past it
		*end++ = 0;
		*s1 = end;

		// Start past first quote
		return ::strdup(start);
	}

	// Handle unquoted atom
	else
	{
		// Find first space, bracket or end
		char* end = ::strpbrk(start, tokens);

		// Terminate string at space (advance past it) or point to end of atom
		char* dup;
		if (end)
		{
			// Save end char and terminate
			char save = *end;
			*end = 0;

			// Duplicate then restore original
			dup = (*start ? ::strdup(start) : NULL);
			*end = save;
		}
		else
		{
			dup = (*start ? ::strdup(start) : NULL);
			end = start + ::strlen(start);
		}
		*s1 = end;

		return dup;
	}
}

char * strstrnocase(const char * str, const char * pat)
{
	if (!str || !pat)
		return NULL;

#if !__POWERPC__

	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc, c1, c2;
	
	if ((pat == NULL) || (!(firstc = tolower(*p1++))))
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while((c1 = tolower(*s1++)) != 0)
		if (c1 == firstc)
		{
			const unsigned char * s2 = s1;
			const unsigned char * p2 = p1;
			
			while ((c1 = tolower(*s2++)) == (c2 = tolower(*p2++)) && c1) {}
			
			if (!c2)
				return((char *) s1 - 1);
		}
	
	return(NULL);

#else

	unsigned char * s1 = (unsigned char *) str-1;
	unsigned char * p1 = (unsigned char *) pat-1;
	unsigned long firstc, c1, c2;
	
	if ((pat == NULL) || (!(firstc = tolower(*++p1))))    /* 980807  vss  PPC must be pre-increment */
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while((c1 = tolower(*++s1)) != 0)
		if (c1 == firstc)
		{
			const unsigned char * s2 = s1-1;
			const unsigned char * p2 = p1-1;
			
			while ((c1 = tolower(*++s2)) == (c2 = tolower(*++p2)) && c1) {}
			
			if (!c2)
				return((char *) s1);
		}
	
	return(NULL);

#endif
}

char * strnstrnocase(const char * str, const char * pat, size_t n)
{
	if (!str || !pat)
		return NULL;

#if !__POWERPC__

	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc = 0;
    unsigned char c1 = 0;
    unsigned char c2 = 0;
	
	if ((pat == NULL) || (!(firstc = tolower(*p1++))))
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while(n--)
	{
		c1 = tolower(*s1++);
		if (c1 == firstc)
		{
			size_t m = n;
			const unsigned char * s2 = s1;
			const unsigned char * p2 = p1;
			
			while (m-- && (c1 = tolower(*s2++)) == (c2 = tolower(*p2++))) {}
			
			if (!c2)
				return((char *) s1 - 1);
		}
	}

	return(NULL);

#else

	unsigned char * s1 = (unsigned char *) str-1;
	unsigned char * p1 = (unsigned char *) pat-1;
	unsigned char firstc = 0;
    unsigned char c1 = 0;
    unsigned char c2 = 0;
	
	if ((pat == NULL) || (!(firstc = tolower(*++p1))))    /* 980807  vss  PPC must be pre-increment */
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while(n--)
	{
		c1 = tolower(*++s1);
		if (c1 == firstc)
		{
			size_t m = n;
			const unsigned char * s2 = s1-1;
			const unsigned char * p2 = p1-1;
			
			while (m-- && (c1 = tolower(*++s2)) == (c2 = tolower(*++p2))) {}
			
			if (!c2)
				return((char *) s1);
		}
	}
	
	return(NULL);

#endif
}

// Wildcard pattern match
bool strpmatch(const char* s, const char* pat)
{
	if (!s || !pat)
		return false;

	switch (*pat)
	{
	// Match wildcard zero or more times
	case '*':
		// Does pattern end in wildcard
		if (!pat[1])
		return true;
		/* if still more, hunt through rest of base */

		// Search through rest of string
		for (; *s; s++)
			// Compare remainder of string with remaining pattern
			if (::strpmatch(s, pat+1))
				return true;
		break;

	// Check pattern end
	case 0:
		// Matched if string also ended
		return (*s ? false : true);

	// General character
	default:
		// Do non-case match
		return ((isupper(*pat) ? *pat + 'a' - 'A' : *pat) ==
				(isupper((unsigned char)*s) ? *s + 'a' - 'A' : *s)) ? ::strpmatch (s+1, pat+1) : false;
	}

	return false;
}

// Wildcard pattern match on hierarchies
bool strpdirmatch(const char* s, const char* pat, char separator)
{
	if (!s || !pat)
		return false;

	switch (*pat)
	{
	// Match wildcard zero or more times
	case '*':
		// Does pattern end in wildcard
		if (!pat[1])
			return true;

		// Search through rest of string
		for (; *s; s++)
			if (::strpdirmatch(s, pat+1, separator))
				return true;
		break;

	// Match wildcard up to separator
	case '%':
		// Search through rest of string
		for (; *s; s++)
		{
			// Halt wildcard at separator and match remainder
			if (*s == separator)
				return ::strpdirmatch(s, pat+1, separator);
			
			// Compare remainder of string with remaining pattern
			else if (::strpdirmatch(s, pat+1, separator))
				return true;
		}
		
		// Must be at end of pattern and search string for success
		return !pat[1];

	// Check pattern end
	case 0:
		// Matched if string also ended
		return (*s ? false : true);

	// General character
	default:
		// Do non-case match
		return ((isupper(*pat) ? *pat + 'a' - 'A' : *pat) ==
				(isupper(*s) ? *s + 'a' - 'A' : *s)) ? ::strpdirmatch (s+1, pat+1, separator) : false;
	}

	return false;
}

// Find one string in array and return index
unsigned long strindexfind(const char* s, const char** ss, unsigned long default_index)
{
	if (s && ss)
	{
		const char** p = ss;
		unsigned long i = 0;
		while(*p)
		{
			if (!::strcmpnocase(s, *p))
				return i;
			i++;
			p++;
		}
	}
	
	return default_index;
}

// Find one string in array and return index
unsigned long strnindexfind(const char* s, const char** ss, unsigned long default_index)
{
	if (s && ss)
	{
		const char** p = ss;
		unsigned long i = 0;
		while(*p)
		{
			if (!::strncmpnocase(s, *p, ::strlen(*p)))
				return i;
			i++;
			p++;
		}
	}
	
	return default_index;
}

// Compare strings, advance original past matched string if its there
bool CheckStrAdv(char** s1, const char* s2)
{
	if (!s1 || !s2)
		return false;

	char*	s3 = *s1;
	size_t	len = ::strlen(s2);

	// Strip space
	while(*s3 == ' ') s3++;

	// Check for string
	if (::strncmpnocase(s3, s2, len)==0)
	{
		// Advance pointer past string if found
		*s1 = s3 + len;
		return true;
	}
	else

		return false;
}

// Copy original string if it does not match, if it matches copy emtpy string
// Always advance original
bool CopyStrNoMatch(char** str, const char* no_match, char* copy, unsigned long max_len)
{
	if (!str)
		return false;

	// If NULL copy emtpy string
	if (::CheckStrAdv(str, no_match))

		// Copy empty string because of match
		copy[0] = 0;

	else
	{
		// Get quoted string or atom
		char* p = ::strgetquotestr(str);
		if (!p) return false;

		// Copy it in only up to max chars
		if (::strlen(p) < max_len-1)
			::strcpy(copy, p);
		else
		{
			::strncpy(copy, p, max_len-1);
			copy[max_len-1] = 0;
		}
	}

	return true;

} // CopyStrNoMatch

#pragma mark -

// Convert from network endl to local endl
void FilterEndls(char* txt)
{
	// Must have text
	if (!txt)
		return;

	char*	s1=txt;
	char*	s2=txt;

	// Loop until null
	while(*s2)
	{

#if __line_end != __crlf
		// Skip the unwanted one
		if (*s2 == lendl3)
			s2++;
		else
#endif
		// Copy from advanced ptr to current ptr
		*s1++ = *s2++;
	}

	// Tie off string
	*s1 = 0;

}

// Add LFs after CRs in text - return new ptr
char* FilterInLFs(const char* txt, bool dot_stuff)
{
	// Must have text
	if (!txt)
		return NULL;

	// First count lonely CRs or LFs
	unsigned long count = 0;
	const char* p = txt;

	while (*p)
	{
		// CR or LF but not CRLF
		if (((*p == '\r') && (*(p+1) != '\n')) ||
			((*(p-1) != '\r') && (*p == '\n')))
			count++;

		// Double period at start of line
		if (dot_stuff && ((*p == '.') && ((*(p-1) == '\r') || (*(p-1) == '\n'))))
			count++;
		p++;
	}

	// Create new ptr of the right size
	char* filtered = new char[::strlen(txt) + count + 1];

	// Now do filter
	if (filtered)
	{
		p = txt;
		char* q = filtered;
		while(*p) 
		{
			// Copy char
			if ((*p != '\r') && (*p != '\n'))
				*q++ = *p++;
			else
			{
				// Check existing CRLF and copy
				if ((*p == '\r') && (*(p+1) == '\n'))
				{
					*q++ = *p++;
					*q++ = *p++;
				}
				
				// Must be lonely CR or LF
				else
				{
					// Add CRLF to filter and update ptrs
					p++;
					*q++ = '\r';
					*q++ = '\n';
				}

				// Check for period at start of line and double it
				if (dot_stuff && (*p == '.')) *q++ = '.';
			}

		}

		// Tie it off
		*q = 0;
	}

	return filtered;
}

// Remove LFs from CRLF pairs
void FilterOutLFs(char* txt)
{
	// Must have text
	if (!txt)
		return;

	const char* p = txt;
	char* q = txt;
	while(*p)
	{
		if (*p == '\n')
			p++;
		else
			*q++ = *p++;
	}
	*q = 0;
}

// Filter out C-style escape chars '\'
void FilterOutEscapeChars(char* txt)
{
	// Must have text
	if (!txt)
		return;

	char* p = txt;
	char* q = txt;

	// Look at all chars
	while(*p) {

		// Is current char escape
		if (*p=='\\') {

			// Advance past escape
			p++;

			// Test escaped char
			switch(*p++) {

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
}

// Filter in C-style escape chars '\'
char* FilterInEscapeChars(const char* txt)
{
	// Must have text
	if (!txt)
		return NULL;

	const char* p = txt;

	// count number of escapes
	unsigned long ctr=0;
	while(*p) {
		// Add extra char for escapes
		if ((*p=='\a') ||
			(*p=='\\') ||
			(*p=='\b') ||
			(*p=='\r') ||
			(*p=='\"') ||
			(*p=='\f') ||
			(*p=='\t') ||
			(*p=='\n') ||
			(*p==0) ||
			(*p=='\'') ||
			(*p=='\v'))
			ctr++;

		// One char added
		ctr++;
		p++;
	}

	// create new string
	char* r = new char[ctr+1];
	char* q = r;

	// Reset to start of string
	p = txt;
	while(*p) {

		// Test escaped char
		switch(*p) {

			case '\a':			// alert
				*q++ = '\\';
				*q++ = 'a';
				break;

			case '\\':			// backslash
				*q++ = '\\';
				*q++ = '\\';
				break;

			case '\b':			// backspace
				*q++ = '\\';
				*q++ = 'b';
				break;

			case '\r':			// carriage return
				*q++ = '\\';
				*q++ = 'r';
				break;

			case '\"':			// double quote
				*q++ = '\\';
				*q++ = '"';
				break;

			case '\f':			// formfeed
				*q++ = '\\';
				*q++ = 'f';
				break;

			case '\t':			// horizontal tab
				*q++ = '\\';
				*q++ = 't';
				break;

			case '\n':			// newline
				*q++ = '\\';
				*q++ = 'n';
				break;

			case '\'':			// single quote
				*q++ = '\\';
				*q++ = '\'';
				break;

			case '\v':			// vertical tab
				*q++ = '\\';
				*q++ = 'v';
				break;

			case 0:
				p--;
				break;

			default:
				*q++ = *p;
		}

		p++;
	}

	// Terminate
	*q = 0;

	return r;
}
