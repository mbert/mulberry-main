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


// Source for common utilities

#include "CUStringUtils.h"

#include <ctype.h>
#include <errno.h>

// S T R I N G  F U N C T I O N S

size_t unistrlen(const unichar_t* str)
{
	size_t	len = -1;
	
#if !__POWERPC__
	
	do
		len++;
	while (*str++);
	
#else
	
	unichar_t * p = (unichar_t *) str - 1;
	
	do
		len++;
	while (*++p);
	
#endif
	
	return(len);
}

int unistrcmp(const unichar_t * str1, const unichar_t * str2)
{
#if !__POWERPC__
	
	const	unichar_t * p1 = (unichar_t *) str1;
	const	unichar_t * p2 = (unichar_t *) str2;
				unichar_t		c1, c2;
	
	while ((c1 = *p1++) == (c2 = *p2++))
		if (!c1)
			return(0);

#else
	
	const	unichar_t * p1 = (unichar_t *) str1 - 1;
	const	unichar_t * p2 = (unichar_t *) str2 - 1;
				unichar_t		c1, c2;
		
	while ((c1 = *++p1) == (c2 = *++p2))
		if (!c1)
			return(0);

#endif
	
	return(c1 - c2);
}

int unistrncmp(const unichar_t * str1, const unichar_t * str2, size_t n)
{
#if !__POWERPC__
	
	const	unichar_t * p1 = (unichar_t *) str1;
	const	unichar_t * p2 = (unichar_t *) str2;
				unichar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *p1++) != (c2 = *p2++))
			return(c1 - c2);
		else if (!c1)
			break;
	
#else
	
	const	unichar_t * p1 = (unichar_t *) str1 - 1;
	const	unichar_t * p2 = (unichar_t *) str2 - 1;
				unichar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *++p1) != (c2 = *++p2))
			return(c1 - c2);
		else if (!c1)
			break;

#endif
	
	return(0);
}

int unistrcmp(const unichar_t * str1, const char * str2)
{
#if !__POWERPC__
	
	const	unichar_t * p1 = (unichar_t *) str1;
	const	char * p2 = (char *) str2;
				unichar_t		c1, c2;
	
	while ((c1 = *p1++) == (c2 = (unsigned char)*p2++))
		if (!c1)
			return(0);

#else
	
	const	unichar_t * p1 = (unichar_t *) str1 - 1;
	const	char * p2 = (char *) str2 - 1;
				unichar_t		c1, c2;
		
	while ((c1 = *++p1) == (c2 = (unsigned char)*++p2))
		if (!c1)
			return(0);

#endif
	
	return(c1 - c2);
}

int unistrncmp(const unichar_t * str1, const char * str2, size_t n)
{
#if !__POWERPC__
	
	const	unichar_t * p1 = (unichar_t *) str1;
	const	char * p2 = (char *) str2;
				unichar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *p1++) != (c2 = (unsigned char)*p2++))
			return(c1 - c2);
		else if (!c1)
			break;
	
#else
	
	const	unichar_t * p1 = (unichar_t *) str1 - 1;
	const	char * p2 = (char *) str2 - 1;
				unichar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *++p1) != (c2 = (unsigned char)*++p2))
			return(c1 - c2);
		else if (!c1)
			break;

#endif
	
	return(0);
}

// Compare without case
int unistrcmpnocase(const unichar_t* s1, const unichar_t* s2)
{
	if (!s1 || !s2)
		return s1 - s2;

#if !__POWERPC__

	const unichar_t * p1 = (unichar_t *) s1;
	const unichar_t * p2 = (unichar_t *) s2;
	unichar_t c1, c2;

	
	while ((c1 = ::unitolower(*p1++)) == (c2 = ::unitolower(*p2++)))
		if (!c1)
			return 0;

#else

	const unichar_t * p1 = (unichar_t *) s1 - 1;
	const unichar_t * p2 = (unichar_t *) s2 - 1;
	unichar_t c1, c2;

	while ((c1 = ::unitolower(*++p1)) == (c2 = ::unitolower(*++p2)))
		if (!c1)
			return 0;

#endif

	return c1 - c2;
}

// Compare without case
int unistrncmpnocase(const unichar_t* s1, const unichar_t* s2, size_t n)
{
	if (!s1 || !s2)
		return s1 - s2;

#if !__POWERPC__

	const unichar_t * p1 = (unichar_t *) s1;
	const unichar_t * p2 = (unichar_t *) s2;
	unichar_t c1, c2;

	n++;

	while (--n)
		if ((c1 = ::unitolower(*p1++)) != (c2 = ::unitolower(*p2++)))
			return c1 - c2;
		else if (!c1)
			break;

#else

	const unichar_t * p1 = (unichar_t *) s1 - 1;
	const unichar_t * p2 = (unichar_t *) s2 - 1;
	unsigned long c1, c2;

	n++;

	while (--n)
		if ((c1 = ::unitolower(*++p1)) != (c2 = ::unitolower(*++p2)))
			return c1 - c2;
		else if (!c1)
			break;

#endif

	return 0;
}

// Compare without case
int unistrcmpnocase(const unichar_t* s1, const char* s2)
{
	if (!s1 || !s2)
		return (const char*)s1 - s2;

#if !__POWERPC__

	const unichar_t * p1 = (unichar_t *) s1;
	const char * p2 = (char *) s2;
	unichar_t c1, c2;

	
	while ((c1 = ::unitolower(*p1++)) == (c2 = (unsigned char)::tolower(*p2++)))
		if (!c1)
			return 0;

#else

	const unichar_t * p1 = (unichar_t *) s1 - 1;
	const char * p2 = (char *) s2 - 1;
	unichar_t c1, c2;

	while ((c1 = ::unitolower(*++p1)) == (c2 = (unsigned char)::tolower(*++p2)))
		if (!c1)
			return 0;

#endif

	return c1 - c2;
}

// Compare without case
int unistrncmpnocase(const unichar_t* s1, const char* s2, size_t n)
{
	if (!s1 || !s2)
		return (const char*)s1 - s2;

#if !__POWERPC__

	const unichar_t * p1 = (unichar_t *) s1;
	const char * p2 = (char *) s2;
	unichar_t c1, c2;

	n++;

	while (--n)
		if ((c1 = ::unitolower(*p1++)) != (c2 = (unsigned char)::tolower(*p2++)))
			return c1 - c2;
		else if (!c1)
			break;

#else

	const unichar_t * p1 = (unichar_t *) s1 - 1;
	const char * p2 = (char *) s2 - 1;
	unsigned long c1, c2;

	n++;

	while (--n)
		if ((c1 = ::unitolower(*++p1)) != (c2 = (unsigned char)::tolower(*++p2)))
			return c1 - c2;
		else if (!c1)
			break;

#endif

	return 0;
}

unichar_t * unistrcpy(unichar_t * dst, const unichar_t * src)
{
#if !__POWERPC__
	
	const	unichar_t * p = src;
				unichar_t * q = dst;
	
	while ((*q++ = *p++) != 0);
	
#else
	
	const	unichar_t * p = (unichar_t *) src - 1;
				unichar_t * q = (unichar_t *) dst - 1;
	
	while (*++q = *++p);

#endif
	
	return(dst);
}

unichar_t * unistrncpy(unichar_t * dst, const unichar_t * src, size_t n)
{
#if !__POWERPC__
	
	const	unichar_t * p = src;
				unichar_t * q = dst;
	
	n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			while (--n)
				*q++ = 0;
			break;
		}
	
#else
	
	const	unichar_t * p		= (const unichar_t *) src - 1;
				unichar_t * q		= (unichar_t *) dst - 1;
				unichar_t zero	= 0;
	
	n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			while (--n)
				*++q = 0;
			break;
		}

#endif
	
	return(dst);
}

unichar_t * unistrcat(unichar_t * dst, const unichar_t * src)
{
#if !__POWERPC__
	
	const	unichar_t * p = src;
				unichar_t * q = dst;
	
	while (*q++);
	
	q--;
	
	while ((*q++ = *p++) != 0);
	
#else
	
	const	unichar_t * p = (unichar_t *) src - 1;
			unichar_t * q = (unichar_t *) dst - 1;
	
	while (*++q);
	
	q--;
	
	while (*++q = *++p);

#endif
	
	return(dst);
}

unichar_t * unistrncat(unichar_t * dst, const unichar_t * src, size_t n)
{
#if !__POWERPC__
	
	const	unichar_t * p = src;
				unichar_t * q = dst;
	
	while (*q++);
	
	q--; n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			q--;
			break;
		}
	
	*q = 0;
	
#else
	
	const	unichar_t * p = (unichar_t *) src - 1;
				unichar_t * q = (unichar_t *) dst - 1;
	
	while (*++q);
	
	q--; n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			q--;
			break;
		}
	
	*++q = 0;

#endif
	
	return(dst);
}

unichar_t * unistrcat(unichar_t * dst, const char * src)
{
#if !__POWERPC__
	
	const	char * p = src;
				unichar_t * q = dst;
	
	while (*q++);
	
	q--;
	
	while ((*q++ = (unsigned char)*p++) != 0) ;
	
#else
	
	const	char * p = (char *) src - 1;
			unichar_t * q = (unichar_t *) dst - 1;
	
	while (*++q);
	
	q--;
	
	while (*++q = (unsigned char)*++p);

#endif
	
	return(dst);
}

unichar_t * unistrncat(unichar_t * dst, const char * src, size_t n)
{
#if !__POWERPC__
	
	const	char * p = src;
				unichar_t * q = dst;
	
	while (*q++);
	
	q--; n++;
	
	while (--n)
		if (!(*q++ = (unsigned char)*p++))
		{
			q--;
			break;
		}
	
	*q = 0;
	
#else
	
	const	char * p = (char *) src - 1;
				unichar_t * q = (unichar_t *) dst - 1;
	
	while (*++q);
	
	q--; n++;
	
	while (--n)
		if (!(*++q = (unsigned char)*++p))
		{
			q--;
			break;
		}
	
	*++q = 0;

#endif
	
	return(dst);
}

// Duplicate string
unichar_t* unistrdup(const unichar_t* s1)
{
	if (s1 && *s1)
	{
		unichar_t* s2 = new unichar_t[::unistrlen(s1) + 1];
		::unistrcpy(s2, s1);
		return s2;
	}
	else
		return NULL;
}

// Duplicate string a fixed length
unichar_t* unistrndup(const unichar_t* s1, size_t len)
{
	if (s1 && *s1 && len)
	{
		unichar_t* s2 = new unichar_t[len + 1];
		::unistrncpy(s2, s1, len);
		s2[len] = 0;
		return s2;
	}
	else
		return NULL;
}

unichar_t * unistrchr(const unichar_t * str, unichar_t chr)
{
#if !__POWERPC__

	const unichar_t * p = str;
	      unichar_t   c = chr;
	      unichar_t   ch;
	
	while((ch = *p++) != 0)
		if (ch == c)
			return((unichar_t *) (p - 1));
	
	return(c ? 0 : (unichar_t *) (p - 1));

#else

	const unichar_t * p = (unichar_t *) str - 1;
	      unichar_t   c = chr;
	      unichar_t   ch;
	
	while(ch = *++p)
		if (ch == c)
			return((unichar_t *) p);
	
	return(c ? 0 : (unichar_t *) p);

#endif
}

unichar_t * unistrrchr(const unichar_t * str, const unichar_t chr)
{
#if !__POWERPC__

	const unichar_t * p = str;
	const unichar_t * q = 0;
	      unichar_t   c = chr;
	      unichar_t   ch;
	
	while((ch = *p++) != 0)
		if (ch == c)
			q = p - 1;
	
	if (q)
		return((unichar_t *) q);
	
	return(c ? 0 : (unichar_t *) (p - 1));

#else

	const unichar_t * p = (unichar_t *) str - 1;
	const unichar_t * q = 0;
	      unichar_t   c = chr;
	      unichar_t   ch;
	
	while(ch = *++p)
		if (ch == c)
			q = p;
	
	if (q)
		return((unichar_t *) q);
	
	return(c ? 0 : (unichar_t *) p);

#endif
}

/* Code deleted here mm 010316  The version of wcstok for Windows made use of the thread-local
   data belonging to strtok, which breaks the requirement that strtok behave as if no other library
   function calls it.  Further, the Standards committe learnt from its experience with strtok that
   requiring a state that the library had to look after was a bad thing and so they gave wcstok an
   extra parameter so that thread-local storage is no longer needed for it.                         */

typedef char unichar_map[8192];									/*- mm 990914 -*/
#define set_unichar_map(map, ch)  map[ch>>3] |= (1 << (ch&7))
#define tst_unichar_map(map, ch) (map[ch>>3] &  (1 << (ch&7)))

unichar_t* unistrtok(unichar_t* str, const unichar_t* set, unichar_t** ptr)
{
	unichar_t 		* p, * q;
	static unichar_t 	* n	= (unichar_t *) L"";
	unichar_t 		* s;
	unichar_t			c;
	unichar_map		map;
	
	::memset(&map, 0, sizeof(unichar_map));
	
	if (str)
		s = (unichar_t *) str;
	else
		if (*ptr)
			s = (unichar_t *) *ptr;
		else
			return(NULL);
	
	
#if !__POWERPC__
	
	p = (unichar_t *) set;

	while ((c = *p++) != 0)
		set_unichar_map(map, c);
	
	p = s;
	
	while ((c = *p++) != 0)
		if (!tst_unichar_map(map, c))
			break;
	
	if (!c)
	{
		/*s = n;*/
		*ptr = NULL;
		return(NULL);
	}
	
	q = p - 1;
	
	while ((c = *p++) != 0)
		if (tst_unichar_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s    = p;
		*--p = 0;
	}
	
	if (q == NULL)												/*- mm 000422 -*/
		*ptr = NULL;											/*- mm 000422 -*/
	else														/*- mm 000422 -*/
		*ptr = s;												/*- mm 000422 -*/
	return(( unichar_t *) q);

#else   /* __POWERPC__ */
	
	p = ( unichar_t *) set - 1;

	while (c = *++p)
		set_unichar_map(map, c);
	
	p = s - 1;
	
	while (c = *++p)
		if (!tst_unichar_map(map, c))
			break;
	
	if (!c)
	{
		/*s = n;*/											/*- ejs 020219 -*/
		*ptr = NULL;										/*- mm 000422 -*/
		return(NULL);
	}
	
	q = p;
	
	while (c = *++p)
		if (tst_unichar_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s  = p + 1;
		*p = L'\0';
	}
	
	if (q == NULL)												/*- mm 000422 -*/
		*ptr = NULL;											/*- mm 000422 -*/
	else														/*- mm 000422 -*/
		*ptr = s;												/*- mm 000422 -*/
	return(( unichar_t *) q);

#endif   /* __POWERPC__ */
}

unichar_t unitolower(unichar_t c)
{
	if (c < 0x7F)
		return tolower(c);
	else
		return c;
}

unichar_t unitoupper(unichar_t c)
{
	if (c < 0x7F)
		return toupper(c);
	else
		return c;
}

// Convert to lowercase
void unistrlower(unichar_t* s1)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
        {
            unichar_t u = *s1;
			*s1++ = unitolower(u);
        }
#else
	if (s1--)
		while(*++s1)
        {
            unichar_t u = *s1;
			*s1 = unitolower(u);
        }
#endif
}

// Convert to uppercase
void unistrupper(unichar_t* s1)
{
#if !__POWERPC__

	if (s1)
		while(*s1)
        {
            unichar_t u = *s1;
			*s1++ = unitoupper(u);
        }
#else
	if (s1--)
		while(*++s1)
        {
            unichar_t u = *s1;
			*s1 = unitoupper(u);
        }
#endif
}

int isuspace(unichar_t c)
{
	if (c < 0x7F)
		return isspace(c);
	else
		return 0;
}

int isupunct(unichar_t c)
{
	if (c < 0x7F)
		return ispunct(c);
	else
		return 0;
}

int isudigit(unichar_t c)
{
	if (c < 0x7F)
		return isdigit(c);
	else
		return 0;
}

int isuxdigit(unichar_t c)
{
	if (c < 0x7F)
		return isxdigit(c);
	else
		return 0;
}

long uniatol(unichar_t* s)
{
	long result = 0;
	bool negative = false;

	// Punt space
	while(isuspace(*s)) s++;
	
	// Look for '+'/'-'
	if (*s == '+')
		s++;
	else if (*s == '-')
	{
		negative = true;
		s++;
	}
	else if (!isudigit(*s))
		return result;
	
	// Count digits
	unichar_t* start = s;
	
	while(isudigit(*s)) s++;
	
	// Check range error
	if (s - start > 10)
	{
		errno = ERANGE;
		return result;
	}
	
	// Need to check range error if we have exactly ten digits being converted
	bool range_check = (s - start == 10);
	
	while(start < s)
	{
		long add_char = (*start - '0');

		// Check for overflow
		if (range_check && ((result > 0x0ccccccc) || ((result == 0x0ccccccc) && (add_char > 7))))
		{
			errno = ERANGE;
			return 0;
		}

		result = result * 10 + add_char;
		start++;
	}
	
	if (negative)
		result = -result;
	
	return result;
}

unsigned long uniatoul(unichar_t* s)
{
	unsigned long result = 0;

	// Punt space
	while(isuspace(*s)) s++;
	
	// Look for '+' which we ignore
	if (*s == '+')
		s++;
	else if (!isudigit(*s))
		return result;
	
	// Count digits
	unichar_t* start = s;
	
	while(isudigit(*s)) s++;
	
	// Check range error
	if (s - start > 10)
	{
		errno = ERANGE;
		return result;
	}
	
	// Need to check range error if we have exactly ten digits being converted
	bool range_check = (s - start == 10);
	
	while(start < s)
	{
		unsigned long add_char = (*start - '0');

		// Check for overflow
		if (range_check && ((result > 0x19999999) || ((result == 0x19999999) && (add_char > 5))))
		{
			errno = ERANGE;
			return 0;
		}

		result = result * 10 + add_char;
		start++;
	}
	
	return result;
}

const unsigned long ul_fromhex[] =
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0 - 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 16 - 31
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 32 - 47
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 48 - 63
    0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 64 - 79
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 80 - 95
    0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 96 - 111
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 112 - 127
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 128 - 143
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 144 - 159
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 160 - 175
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 176 - 191
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 192 - 207
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 208 - 223
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 224 - 239
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned long unihextoul(unichar_t* s)
{
	long result = 0;

	// Punt space
	while(isuspace(*s)) s++;
	
	if (!isuxdigit(*s))
		return result;
	
	// Count digits
	unichar_t* start = s;
	
	while(isudigit(*s)) s++;
	
	// Check range error
	if (s - start > 8)
	{
		errno = ERANGE;
		return result;
	}
	
	while(start < s)
	{
		unsigned long add_char = ul_fromhex[*start & 0x00FF];

		result = (result << 4) | add_char;
		start++;
	}
	
	return result;
}

int cUTF8CharOffset[256] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x00
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x10
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x20
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x30
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x40
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x50
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x60	
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x70
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x80 - all illegal
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x90 - all illegal
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xA0 - all illegal
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xB0 - all illegal
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xC0
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xD0
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 0xE0
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1		// 0xF0 - 0xFE, 0xFF are illegal
};

long UTF8OffsetToUTF16Offset(const char* c, long offset)
{
	long result = 0;
	while(offset > 0)
	{
		result++;
		int offset_char = cUTF8CharOffset[(unsigned char) *c];
		offset -= offset_char;
		c += offset_char;
	}
	
	return result;
}

int cISOtoUTF8CharOffset[256] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x00
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x10
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x20
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x30
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x40
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x50
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x60	
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x70
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0x80
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0x90
	2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xA0 (0xA4 = Euro)
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xB0
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xC0
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xD0
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// 0xE0
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2		// 0xF0
};

long ISOOffsetToUTF8Offset(const char* c, long offset)
{
	long result = 0;
	while(offset > 0)
	{
		int offset_char = cISOtoUTF8CharOffset[(unsigned char) *c];
		result += offset_char;
		offset--;
		c++;
	}
	
	return result;
}

// Remove LFs from CRLF pairs
void FilterOutLFs(unichar_t* txt)
{
	// Must have text
	if (!txt)
		return;

	const unichar_t* p = txt;
	unichar_t* q = txt;
	while(*p)
	{
		if (*p == '\n')
			p++;
		else
			*q++ = *p++;
	}
	*q = 0;
}

