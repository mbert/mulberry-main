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

	// Use PowerPlant-specific Precompiled header

#pragma once

#define UNICODE		1
#define _UNICODE	1

#define NOMINMAX	1

#define _USE_32BIT_TIME_T

#define snprintf _snprintf

#include "stdafx.h"

// The various OS's
#define __win32_os 95
#define __mac_os_x 3
#define __mac_os 2
#define __linux_os 1
#define __dest_os __win32_os

#include	"os_dep.h"
#include	"Mac2Win.h"
