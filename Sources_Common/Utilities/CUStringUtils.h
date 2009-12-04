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


// Header for common utilities

#ifndef __CUSTRINGUTILS__MULBERRY__
#define __CUSTRINGUTILS__MULBERRY__

#include <string.h>

#if __dest_os == __mac_os || __dest_os == __mac_os_x
typedef unsigned short unichar_t;
#elif __dest_os == __win32_os
typedef wchar_t unichar_t;
#else
typedef unsigned short unichar_t;
#endif

// String handling functions
size_t unistrlen(const unichar_t * s);

int unistrcmp(const unichar_t * s1, const unichar_t * s2);
int unistrncmp(const unichar_t * s1, const unichar_t * s2, size_t n);

int unistrcmp(const unichar_t * s1, const char * s2);
int unistrncmp(const unichar_t * s1, const char * s2, size_t n);

int unistrcmpnocase(const unichar_t* s1, const unichar_t* s2);					// Compare without case
int unistrncmpnocase(const unichar_t* s1, const unichar_t* s2, size_t n);		// Compare without case

int unistrcmpnocase(const unichar_t* s1, const char* s2);					// Compare without case
int unistrncmpnocase(const unichar_t* s1, const char* s2, size_t n);		// Compare without case

unichar_t * unistrcpy(unichar_t * dst, const unichar_t * src);
unichar_t * unistrncpy(unichar_t * dst, const unichar_t * src, size_t n);

unichar_t * unistrcat(unichar_t * dst, const unichar_t * src);
unichar_t * unistrncat(unichar_t * dst, const unichar_t * src, size_t n);

unichar_t * unistrcat(unichar_t * dst, const char * src);
unichar_t * unistrncat(unichar_t * dst, const char * src, size_t n);

unichar_t * unistrdup(const unichar_t * s);
unichar_t * unistrndup(const unichar_t * s, size_t n);

unichar_t * unistrchr(const unichar_t * str, unichar_t chr);
unichar_t * unistrrchr(const unichar_t * str, const unichar_t chr);

unichar_t* unistrtok(unichar_t* str, const unichar_t* set, unichar_t** ptr);

unichar_t unitolower(unichar_t c);
unichar_t unitoupper(unichar_t c);

void unistrlower(unichar_t* s1);						// Convert to lowercase
void unistrupper(unichar_t* s1);						// Convert to uppercase

int isuspace(unichar_t c);
int isupunct(unichar_t c);
int isudigit(unichar_t c);
int isuxdigit(unichar_t c);

long uniatol(unichar_t* s);
unsigned long uniatoul(unichar_t* s);
unsigned long unihextoul(unichar_t* s);

long UTF8OffsetToUTF16Offset(const char* c, long offset);
long ISOOffsetToUTF8Offset(const char* c, long offset);

void FilterOutLFs(unichar_t* txt);					// Remove LFs from CRLF pairs

#endif
