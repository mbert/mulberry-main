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

// PluginDLLmain.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 17-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This is the entry point into Mulberry plug-in DLLs. You should modify this file
// by changing the default plug-in type from CPluginDLL to your own derived class.
//
// History:
// 17-Nov-1997: Created initial header and implementation.
// 17-Jun-1998: Modified to allow multiple copies of plugin for multithreaded oepration.
//

#if __dest_os == __win32_os
#undef Boolean
#endif

#include "PluginDLLmain.h"

#pragma mark ____________________________Entry Point

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#ifdef __GNUC__
#pragma GCC visibility push(default)
#else
#pragma export on
#endif
#endif
extern "C"
{

#if __dest_os == __win32_os
__declspec(dllexport)
#endif

long MulberryPluginEntry(long, void*, long);

long MulberryPluginEntry(long code, void* data, long refCon)
{
	// If refCon is nil => new plugin required
	if (!refCon)
		refCon = reinterpret_cast<long>(new CPluginType);
	
	// Double check validity of refCon
	if (!refCon || (refCon != reinterpret_cast<CPluginType*>(refCon)->GetRefCon()))
		return 0;

	// Now execute command
	return reinterpret_cast<CPluginType*>(refCon)->Entry(code, data, refCon);
}
}
#ifdef __GNUC__
#pragma GCC visibility pop
#else
#pragma export off
#endif
