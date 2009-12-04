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

#if __dest_os == __linux_os
#include "resources/RES/RegistrationBlankUnix.c"
#endif

#pragma mark ____________________________Entry Point

extern "C"
{
typedef long (*MulberryPluginEntryPtr)(long, void*, long);
}
CFBundleRef theBundle;
static MulberryPluginEntryPtr theFnPtr = NULL;
static LoggingCallbackProcPtr theLogPtr = NULL;

static OSErr LoadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr);
static OSErr UnloadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#pragma export on
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
		LoadFunction(&theBundle, &theFnPtr);
	
	// Double check validity of refCon
	if (!theBundle)
		return 0;

	// Now execute command
	unsigned long result = theFnPtr ? (*theFnPtr)(code, data, refCon) : 0;
	
	// Check for terminate code and unload
	if (code == CPluginDLL::ePluginDestroy)
		UnloadFunction(&theBundle, &theFnPtr);
	
	return result;
}
}
#pragma export off

#if __dest_os == __mac_os
static OSErr FindApplicationDirectory(short *theVRefNum, long *theParID);
static OSErr FindApplicationDirectory(short *theVRefNum, long *theParID)
{
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	FSSpec theSpec;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	/* Find the application FSSpec */
	OSErr theErr = GetProcessInformation(&thePSN, &theInfo);
	
	if (theErr == noErr)
	{
		/* Get directory id for Plug-ins */
		
		/* Return the folder which contains the application */
		*theVRefNum = theSpec.vRefNum;
		*theParID = theSpec.parID;
	}
	
	return theErr;
}

static OSErr CreateBundleFromFSSpec(FSSpec *theSpec, CFBundleRef *theBundle);
static OSErr CreateBundleFromFSSpec(FSSpec *theSpec, CFBundleRef *theBundle)
{
	/* Turn the FSSpec pointing to the Bundle into a FSRef */
	FSRef theRef;
	OSErr theErr = FSpMakeFSRef(theSpec, &theRef);
	
	/* Turn the FSRef into a CFURL */
	CFURLRef thePluginDirURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &theRef);
	if (thePluginDirURL == NULL)
		return -1;

	CFURLRef theBundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, thePluginDirURL, CFSTR("GPG.bundle"), false);
	CFRelease(thePluginDirURL);
	
	if (theBundleURL != NULL)
	{
		/* Turn the CFURL into a bundle reference */
		*theBundle = CFBundleCreate(kCFAllocatorSystemDefault, theBundleURL);
		
		CFRelease(theBundleURL);
	}
	
	return theErr;
}

static OSErr LoadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr)
{
	/* Start with no bundle */
	*theBundle = NULL;
	
	/* This returns the directory which contains the application */
	short theVRefNum;
	long theParID;
	OSErr theErr = FindApplicationDirectory(&theVRefNum, &theParID);
	
	/* Create the FSSpec pointing to the Bundle */
	FSSpec theSpec;
	if (theErr == noErr)
		theErr = FSMakeFSSpec(theVRefNum, theParID, "\pPlug-ins", &theSpec);
	
	/* Create a bundle reference based on a FSSpec */
	if (theErr == noErr)
		theErr = CreateBundleFromFSSpec(&theSpec, theBundle);
	
	if ((theErr == noErr) && (*theBundle != NULL))
	{
		Boolean isLoaded = CFBundleLoadExecutable(*theBundle);
		
		if (isLoaded)
		{
			/* Lookup the function in the bundle by name */
			*theFnPtr = (MulberryPluginEntryPtr) CFBundleGetFunctionPointerForName(*theBundle, CFSTR("MulberryPluginEntry"));
			
		}
	}
	
	return theErr;
}

static OSErr UnloadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr)
{
	/* Call the function if it was found */
	if (*theFnPtr != NULL)
	{
		/* Dispose of the function pointer to the bundled Mach-O routine */
		//DisposePtr((Ptr) *theFnPtr);
		*theFnPtr = NULL;
	}
			
	//CFBundleUnloadExecutable(*theBundle);
		
	//CFRelease(*theBundle);
	
	return noErr;
}
#endif
