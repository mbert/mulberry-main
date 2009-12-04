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

// PluginDLLmain.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 17-Jun-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This is the header file for entry point into Mulberry plug-in DLLs.
// You should make a copy of this file for every plugin you create, and modify it
// by changing the default plug-in type from CPluginDLL to your own derived class.
//
// History:
// 17-Jun-1998: Modified to allow multiple copies of plugin for multithreaded oepration.
//

#ifndef __PLUGIN_DLL_MAIN_MULBERRY__
#define __PLUGIN_DLL_MAIN_MULBERRY__

#include "CCommAdbkIOPluginDLL.h"		// <-- Change this to your derived plug-in's header file

// Set this to the actual plugin class in use

typedef CCommAdbkIOPluginDLL CPluginType;

#endif
