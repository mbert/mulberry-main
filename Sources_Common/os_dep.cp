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


// Header for common OS specific definitions

#include "os_dep.h"

// Strings containing OS and Network line-end chars

static const unichar_t cr_uendl[] =  {'\r', 0};
static const unichar_t cr2_uendl[] =  {'\r', '\r', 0};
static const unichar_t crlf_uendl[] = {'\r', '\n', 0};
static const unichar_t crlf2_uendl[] = {'\r', '\n', '\r', '\n', 0};
static const unichar_t lf_uendl[] = {'\n', 0};
static const unichar_t lf2_uendl[] = {'\n', '\n', 0};

#if __line_end == __cr

const char* os_endl = "\r";
const unichar_t* os_uendl = cr_uendl;
unsigned long os_endl_len = 1;
const char* os_endl2 = "\r\r";
const unichar_t* os_uendl2 = cr2_uendl;

#elif __line_end == __crlf

const char* os_endl = "\r\n";
const unichar_t* os_uendl = crlf_uendl;
unsigned long os_endl_len = 2;
const char* os_endl2 = "\r\n\r\n";
const unichar_t* os_uendl2 = crlf2_uendl;

#elif __line_end == __lf

const char* os_endl = "\n";
const unichar_t* os_uendl = lf_uendl;
unsigned long os_endl_len = 1;
const char* os_endl2 = "\n\n";
const unichar_t* os_uendl2 = lf2_uendl;

#endif

const char* net_endl = "\r\n";
unsigned long net_endl_len = 2;
const char* net_endl2 = "\r\n\r\n";

const char* get_endl(EEndl endl)
{
	switch(endl)
	{
	case eEndl_Auto:
	case eEndl_Any:
	default:
		return os_endl;
	case eEndl_CR:
		return "\r";
	case eEndl_LF:
		return "\n";
	case eEndl_CRLF:
		return "\r\n";
	}
}

const unichar_t* get_uendl(EEndl endl)
{
	switch(endl)
	{
	case eEndl_Auto:
	case eEndl_Any:
	default:
		return os_uendl;
	case eEndl_CR:
		return cr_uendl;
	case eEndl_LF:
		return lf_uendl;
	case eEndl_CRLF:
		return crlf_uendl;
	}
}

unsigned long get_endl_len(EEndl endl)
{
	switch(endl)
	{
	case eEndl_Auto:
	case eEndl_Any:
	default:
		return os_endl_len;
	case eEndl_CR:
	case eEndl_LF:
		return 1;
	case eEndl_CRLF:
		return 2;
	}
}
