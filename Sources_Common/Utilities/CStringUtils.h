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


// Header for common utilities

#ifndef __CSTRINGUTILS__MULBERRY__
#define __CSTRINGUTILS__MULBERRY__

#include <string.h>

#define	LEFT_BRACE_SPACE			" ("
#define	LEFT_BRACKETS_SPACE			" ({["
#define	RIGHT_BRACKETS_SPACE		" )}]"
#define	RIGHT_BRACKETS_WHITE_SPACE	" \t\r\n)}]"
#define WHITE_SPACE					" \t\r\n"
#define WHITE_SPACE_BRACKETS		" \t\r\n({[]})"
#define CR							"\r"
#define CRLF						"\r\n"
#define SPACE						" "
#define TAB							"\t"
#define SPACE_TAB					" \t"

// String handling functions

int strcmpnocase(const char* s1,const char* s2);		// Compare without case
int strncmpnocase(const char* s1,const char* s2, size_t n);		// Compare without case

char*  stradvtok(char** s1,const char* s2);			// Get token with advance
int stradvtokcmp(char** s1,const char* s2);		// token compare and advance

int stradvstrcmp(char** s1,const char* s2);		// string compare and advance

char* strmatchbra(char** s1);					// Match brackets
char getbracketmatch(char bracket);				// Return char matching bracket

char* strgetbrastr(char** s1);										// Get possibly bracketed string
char* strgetquotestr(char** s1, bool unescape = true);				// Get possibly quoted string
char* strdupquotestr(char** s1);									// Duplicate possibly quoted string without destroying end
char* strgettokenstr(char** s1,
						const char* tokens,
						bool unescape = true);	// Get possibly quoted string
char* strduptokenstr(char** s1,
						const char* tokens);	// Duplicate possibly quoted string without destroying end

#if !defined(__GNUC__) && !defined(__VCPP__)
char* strdup(const char* s1);					// Duplicate a string
#endif

#if !defined(__GNUC__) || __dest_os == __mac_os_x
char* strndup(const char* s1, size_t len);		// Duplicate len chars of string
#endif

void strlower(char* s1);						// Convert to lowercase
void strupper(char* s1);						// Convert to uppercase

void strreplace(char* s1, const char* s2, char c);	// Replace specific chars with another
void strreplacespace(char* s1, char c);			// Replace spaces
void strplacespace(char* s1, char c);			// Replaces with spaces

char* strstrnocase(const char* s, const char* pat);	// Substring match without case
char* strnstrnocase(const char* s, const char* pat, size_t n);	// Substring match without case

bool strpmatch(const char* s, const char* pat);	// Wildcard pattern match
bool strpdirmatch(const char* s,
					const char* pat,
					char separator);			// Wildcard pattern match on hierarchies

unsigned long strindexfind(const char* s, const char** ss, unsigned long default_index = 0);
unsigned long strnindexfind(const char* s, const char** ss, unsigned long default_index = 0);

bool CheckStrAdv(char** s1,
						const char* s2);		// Compare strings and advance if matched
bool CopyStrNoMatch(char** str,
						const char* no_match,
						char* copy,
						unsigned long max_len);			// Copy string or empty string if match


void FilterEndls(char* txt);					// Filter out <LF>s
char* FilterInLFs(const char* txt,				// Add LFs after CRs in text - return new ptr
					bool dot_stuff = true);
void FilterOutLFs(char* txt);					// Remove LFs from CRLF pairs

void FilterOutEscapeChars(char* txt);			// Filter out C-style escape chars '\'
char* FilterInEscapeChars(const char* txt);		// Filter in C-style escape chars '\'

#endif
