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


// Precomiled header for speed
#ifndef _MULPCH_H
#define _MULPCH_H

#ifdef MULMAC
#ifdef powerc
#pragma precompile_target "MulberryPPC.mch"
#else
#pragma precompile_target "Mulberry68K.mch"
#endif
#endif

#include "resource.h"

#include "Mulberry_Prefix.h"

#if 0
#include "CPreferences.h"
#endif

#endif
