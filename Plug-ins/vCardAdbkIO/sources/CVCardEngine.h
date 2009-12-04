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

// CVCardEngine.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 01-Aug-2002
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a generic vCard I/O engine.
//
// History:
// 01-Aug-2002: Created initial header and implementation.
//

#ifndef __VCARDENGINE_PLUGIN_MULBERRY__
#define __VCARDENGINE_PLUGIN_MULBERRY__

#include "cdstring.h"
#include "CAdbkIOPluginDLL.h"

class CVCardEngine
{
public:
	CVCardEngine() {}
	~CVCardEngine() {}
	
	bool ReadOne(std::istream& in, CAdbkIOPluginDLL::SAdbkIOPluginAddress& addr);
	void WriteOne(std::ostream& out, const CAdbkIOPluginDLL::SAdbkIOPluginAddress& addr);
};

#endif
