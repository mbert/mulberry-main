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


// strfind.h

#ifndef __STRFIND__MULBERRY__
#define __STRFIND__MULBERRY__

enum EFindMode
{
	eFind =				0,
	eWrap =				1L << 0,
	eBackwards =		1L << 1,
	eCaseSensitive =	1L << 2,
	eEntireWord =		1L << 3
};

const char* strfind(const char* source, unsigned long len, unsigned long sel_start,
					const char* match, EFindMode mode);

#endif
