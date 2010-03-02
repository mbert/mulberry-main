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

// CDLLFunctions.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 08-Mar-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a wrapper for DLLs that allow dynamic function loading.
//
// History:
// 08-Mar-2003: Created initial header and implementation.
//

#include "CDLLFunctions.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <TextUtils.h>
#endif

#if __dest_os == __mac_os_x
#include "MoreFilesX.h"
#endif

#include <string.h>

#if __dest_os == __win32_os
#include "cdstring.h"
#endif

#if __dest_os == __linux_os
#include <dlfcn.h>
#endif

#include "cdstring.h"
#include "cdustring.h"

CDLLLoader::~CDLLLoader()
{
	// Unload it if this object loaded it
	if (mUnload && (mInstance != NULL))
	{
		// Disconnect from dll
#if __dest_os == __mac_os

		::CloseConnection(&mInstance);

#elif __dest_os == __mac_os_x

		::CFRelease(mInstance);

#elif __dest_os == __win32_os

		::FreeLibrary((HMODULE) mInstance);

#elif __dest_os == __linux_os

		::dlclose(mInstance);

#else
#error __dest_os
#endif
		mInstance = NULL;
	}
}

int CDLLLoader::LoadFunction(const char* fn_name, void** fn_ptr)
{
	// Must be loaded
	if (mInstance == NULL)
		return 0;
		
	// Load function
	*fn_ptr = GetDLLSymbol(fn_name);
	
	// Throw if an error
	if (*fn_ptr == NULL)
		throw -1L;

	return (*fn_ptr != NULL) ? 1 : 0;
}

int	CDLLLoader::LoadDLL(const char* path)
{
	// Create connection to library
#if __dest_os == __mac_os

	Str255 name;
	::c2pstrcpy(name, path);
	
	Ptr main_addr;
	Str255 errName;
	OSErr err = ::GetSharedLibrary(name, kPowerPCCFragArch, kLoadCFrag, &mInstance, &main_addr, errName);
	if (err != noErr)
		return 0;
	
#elif __dest_os == __mac_os_x

	// Convert path to FSRef
	FSSpec fspec;
	::FSPathMakeFSSpec((const unsigned char*) path, &fspec, NULL);
	FSRef fref;
	::FSpMakeFSRef(&fspec, &fref);

	// Get CFURL for item
	CFURLRef bundleURL = ::CFURLCreateFromFSRef(NULL, &fref);
	if (bundleURL)
	{
		mInstance = ::CFBundleCreate(NULL, bundleURL);
		::CFRelease(bundleURL);	
	}

#elif __dest_os == __win32_os

#ifdef _UNICODE
	mInstance = ::LoadLibrary(cdstring(path).win_str());
#else
	mInstance = ::LoadLibrary(path);
#endif

#elif __dest_os == __linux_os

	mInstance = ::dlopen(path, RTLD_NOW);

#else
#error __dest_os
#endif
	return (mInstance != NULL) ? 1 : 0;
}

void* CDLLLoader::GetDLLSymbol(const char* name)
{
	void* proc = NULL;

	// Now try to get ptr to plug-in code
#if __dest_os == __mac_os

	Str255 pname;
	::c2pstrcpy(pname, name);

	// Create CFM proc
	CFragSymbolClass symClass;
	OSErr err = ::FindSymbol(mInstance, pname, (Ptr*) &proc, &symClass);
	if (err != noErr)
		proc = NULL;
	
#elif __dest_os == __mac_os_x

	// Load function
	CFStringRef	ref = ::CFStringCreateWithCString(NULL, name, kCFStringEncodingMacRoman);
	if (ref)
	{
		proc = ::CFBundleGetFunctionPointerForName(mInstance, ref);
		::CFRelease(ref);
	}
	
#elif __dest_os == __win32_os

	proc = ::GetProcAddress((HMODULE) mInstance, name);

#elif __dest_os == __linux_os

	proc = ::dlsym(mInstance, name);

#else
#error __dest_os
#endif

	return proc;
}
