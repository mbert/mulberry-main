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

// CPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 17-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the DLL being called by Mulberry.
//
// History:
// CD:	17-Nov-1997:	Created initial header and implementation.
// CD:	13-Dec-1997:	Turned into abstract base class
// CD:	14-Jan-1998:	Converted import/export into single type
// CD:	12-Feb-1998:	Added logging capability
//

#include "CPluginDLL.h"

#include <string.h>
#include <stdio.h>

#pragma mark ____________________________CPluginDLL

// Constructor
CPluginDLL::CPluginDLL()
{
	mLogging = NULL;

	mRefCon = reinterpret_cast<long>(this);		// refCon is pointer back to this!

	mInfo.mName = 0L;
	mInfo.mVersion = 0;
	mInfo.mType = ePluginUnknown;
	mInfo.mManufacturer = 0L;
	mInfo.mDescription = 0L;
	mInfo.mDataLength = 0;
	mInfo.mData = 0L;
}

// Destructor
CPluginDLL::~CPluginDLL()
{
	mRefCon = NULL;
}

// DLL entry point and dispatch
long CPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case ePluginConstruct:
		Construct();
		return GetRefCon();

	case ePluginDestroy:
		Destroy();
		return 1;

	case ePluginInitialise:
		Initialise();
		return 1;

	case ePluginTerminate:
		// This will delete the object - do nothing with instance variables after this
		Terminate();
		return 1;

	case ePluginCanRun:
		return (CanRun() ? 1 : 0);

	case ePluginStatus:
		return Status();

	case ePluginGetInformation:
		GetInformation((const CPluginDLL::SPluginInfo**) data);
		return 1;

	case ePluginUseRegistration:
		return (UseRegistration((unsigned long*) data) ? 1 : 0);

	case ePluginCanDemo:
		return (CanDemo() ? 1 : 0);

	case ePluginSetLoggingCallback:
		mLogging = (LoggingCallbackProcPtr) data;
		return 1;
	default:
		return 0;
	}
}

// Construct plug-in
void CPluginDLL::Construct(void)
{
	// Nothing to do - object already constructed
}

// Destroy plug-in
void CPluginDLL::Destroy(void)
{
	// Must delete it!
	delete this;
}

// Initialise plug-in
void CPluginDLL::Initialise(void)
{
	// Copy defaults to info object
	mInfo.mName = GetName();
	mInfo.mVersion = GetVersion();
	mInfo.mType = GetType();
	mInfo.mManufacturer = GetManufacturer();
	mInfo.mDescription = GetDescription();
	mInfo.mDataLength = GetDataLength();
	mInfo.mData = GetData();
}

// Terminate plug-in
void CPluginDLL::Terminate(void)
{
}

// Get plug-in status
long CPluginDLL::Status(void)
{
	return 0;
}

// Copy info
void CPluginDLL::GetInformation(const CPluginDLL::SPluginInfo** copy)
{
	*copy = &mInfo;
}

// Log some text
void CPluginDLL::LogEntry(const char* text) const
{
	// If log available, log it!
	if (mLogging)
		(*mLogging)(text);
}

void CPluginDLL::ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno)
{
	if (mLogging)
	{
		// Determine length of buffer needed to hold string
		unsigned long length = ::strlen(GetName()) + ::strlen(errtxt) + ::strlen(func) +::strlen(file) + 256;
		char* txt = new char[length];
		
		// Use name of plugin for log entry
		::strcpy(txt, GetName());
		::strcat(txt, " Error: ");
		::strcat(txt, errtxt);
		
		// Add function, file, line information
		::strcat(txt, os_endl);
		::strcat(txt, func);
		::strcat(txt, "  ");
		::strcat(txt, file);
		::strcat(txt, ",");
		char buf[32];
		::snprintf(buf, 32, "%ld", (unsigned long)lineno);
		::strcat(txt, buf);
		::strcat(txt, os_endl);
		LogEntry(txt);
		
		delete txt;
	}
}
