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


// ustrfind.h

#ifndef __USTRFIND__MULBERRY__
#define __USTRFIND__MULBERRY__

#include "strfind.h"

#include "CUStringUtils.h"

const unichar_t* ustrfind(const unichar_t* source, unsigned long len, unsigned long sel_start,
							const unichar_t* match, EFindMode mode);

#endif
