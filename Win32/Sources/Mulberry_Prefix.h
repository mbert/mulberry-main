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


	// Use PowerPlant-specific Precompiled header

#pragma once

#define __MFC_STDAFX_H

#include "StdAfx.h"

// The various OS's
#define __win32_os 95
#define __mac_os_x 3
#define __mac_os 2
#define __linux_os 1
#define __dest_os __win32_os

#define __MULBERRY
#define __MULBERRY_V2

#include "os_dep.h"

#include	"Mac2Win.h"
#include	"PP_Types.h"
#include	"PP2Win.h"

#include	"resource.h"       // main symbols
