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

// CMachOFunctions.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 20-Apr-2003
// Author: Cyrus Daboo
// Platforms: Mac OS
//
// Description:
// This class implements a wrapper for MachO bundles that allow dynamic function loading.
//
// History:
// 20-Apr-2003: Created initial header and implementation.
//

#include "CMachOFunctions.h"

#include <LCFString.h>
#include <LCFURL.h>

#include <string.h>

CMachOLoader::~CMachOLoader()
{
	// Unload it if this object loaded it
	if (mBundle != NULL)
	{
		// Disconnect from bundle
		::CFRelease(mBundle);
		mBundle = NULL;
	}
}

int CMachOLoader::LoadFunction(const char* fn_name, void** fn_ptr)
{
	// Must be loaded
	if (mBundle == NULL)
		return 0;
		
	// Load function
	*fn_ptr = GetFrameworkProc(fn_name);
	
	// Throw if an error
	if (*fn_ptr == NULL)
		throw -1L;

	return (*fn_ptr != NULL) ? 1 : 0;
}

void* CMachOLoader::LoadSymbol(const char* name)
{
	// Must be loaded
	if (mBundle == NULL)
		return NULL;
		
	// Load function
	void* result = GetFrameworkSymbol(name);
	
	// Throw if an error
	if (result == NULL)
		throw -1L;

	return result;
}

int	CMachOLoader::LoadFramework(const char* bundle)
{
	// Create connection to library
	if (mBundle == NULL)
	{
		// Try /System/Library/Frameworks first
		LCFURL baseURL(kSystemDomain, kFrameworksFolderType, kCreateFolder);
		LCFURL bundleURL(baseURL, LCFString(bundle), false);
		
		mBundle = ::CFBundleCreate(NULL, bundleURL);
	}

	if (mBundle == NULL)
	{
		// Try /Library/Frameworks next
		LCFURL baseURL(kLocalDomain, kFrameworksFolderType, kCreateFolder);
		LCFURL bundleURL(baseURL, LCFString(bundle), false);
		
		mBundle = ::CFBundleCreate(NULL, bundleURL);
	}

	if (mBundle != NULL)
		::CFBundleLoadExecutable(mBundle);

	return (mBundle != NULL) ? 1 : 0;
}

void* CMachOLoader::GetFrameworkProc(const char* name)
{
	void* proc = NULL;

	// Now try to get ptr to plug-in code
	if (mBundle)
		proc = ::CFBundleGetFunctionPointerForName(mBundle, LCFString(name));

	return proc;
}

void* CMachOLoader::GetFrameworkSymbol(const char* name)
{
	void* proc = NULL;

	// Now try to get ptr to plug-in code
	if (mBundle)
		proc = ::CFBundleGetDataPointerForName(mBundle, LCFString(name));

	return proc;
}
