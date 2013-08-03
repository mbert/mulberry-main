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

// CQuickmailAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 15-Apr-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Quickmail address book IO DLL based plug-in for use in Mulberry.
//
// History:
// 15-Apr-1998: Created initial header and implementation.
//

#include "CQuickmailAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "CStringUtils.h"

#include <string.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Quickmail Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Quickmail address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "Quickmail";

const char* cQMTab = "\t";
const size_t cQMTabLen = 1;

#pragma mark ____________________________CQuickmailAdbkIOPluginDLL

// Constructor
CQuickmailAdbkIOPluginDLL::CQuickmailAdbkIOPluginDLL()
{
}

// Destructor
CQuickmailAdbkIOPluginDLL::~CQuickmailAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CQuickmailAdbkIOPluginDLL::Initialise(void)
{
	// Do default
	CAdbkIOPluginDLL::Initialise();
	
	// Assign UI name
	::strncpy(mAdbkIOInfo.mAdbkIOType, cPluginIOName, 255);
	mAdbkIOInfo.mAdbkIOType[255] = 0;
	
	// Type of opertaion allowed
	mAdbkIOInfo.mDoesImport = true;
	mAdbkIOInfo.mDoesExport = true;

	// Assigned file types
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mAdbkIOInfo.mTypes[0] = 0L;
	mAdbkIOInfo.mTypes[1] = 0L;
	mAdbkIOInfo.mTypes[2] = 0L;
	mAdbkIOInfo.mTypes[3] = 0L;
	
	mAdbkIOInfo.mCreator = '????';
#elif __dest_os == __win32_os
	::strncpy(mAdbkIOInfo.mTypes, "Quickmail Exported Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CQuickmailAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CQuickmailAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CQuickmailAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CQuickmailAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CQuickmailAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CQuickmailAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CQuickmailAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CQuickmailAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CQuickmailAdbkIOPluginDLL::ImportAddresses(char* data)
{
	char* p = data;
	char* last_name;
	char* first_name;
	char  whole_name[256];
	char* eaddr;

	while (*p)
	{
		// nil out pointers at start
		last_name = nil;
		first_name = nil;
		whole_name[0] = '\0';
		eaddr = nil;

		// Last name will be at start
		last_name = p;
		SkipTerm(&p);
		
		// First name will be next
		first_name = p;
		SkipTerm(&p);
		
		// Create whole name
		::strcpy(whole_name, first_name);
		::strcat(whole_name, " ");
		::strcat(whole_name, last_name);

		// Gateway will be next - ignore
		SkipTerm(&p);

		// Location will be next - ignore
		SkipTerm(&p);

		// Email will be next
		eaddr = p;
		char q = SkipTerm(&p);

		// Create empty address
		CAdbkIOPluginAddress addr;
		
		// Add email address
		addr.mAddress.mEmail = eaddr;
		
		// Add name
		addr.mAddress.mName = whole_name;

		// Give address to application
		(*mImportCallback)(addr.GetAddressData(), 0);

		// Step till line end or string end
		if (q != '\r')
		{
			while (*p && (*p != '\r')) p++;
			q = *p;
		}

		if (!q) break;

		while ((*p == '\r') || (*p == '\n')) p++;
		
	}
	
	return 1;
}

// Export an address
long CQuickmailAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	const char* write = nil;
	
	// Copy user name for modification
	char whole_name[256];
	::strcpy(whole_name, addr->mName);
	
	// Find last name
	char* last_name = ::strrchr(whole_name, ' ');
	if (last_name)
	{
		// Tie it off and advance to last name
		*last_name = '\0';
		last_name++;
		if (::strlen(last_name) > 0)
		{
			::fwrite(last_name, 1, ::strlen(last_name), mExportFile);
			::fwrite(cQMTab, 1, cQMTabLen, mExportFile);
		}
	}
	
	// Do first name
	::fwrite(whole_name, 1, ::strlen(whole_name), mExportFile);
	::fwrite(cQMTab, 1, cQMTabLen, mExportFile);

	// Do gateway (empty)
	::fwrite(cQMTab, 1, cQMTabLen, mExportFile);

	// Do lcoation (empty)
	::fwrite(cQMTab, 1, cQMTabLen, mExportFile);

	// Write email address mailbox@host
	write = addr->mEmail;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(os_endl, 1, os_endl_len, mExportFile);
	
	return 1;
}

// Export a group
long CQuickmailAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	// No group export

	return 1;
}

// Skip chars for termination
char CQuickmailAdbkIOPluginDLL::SkipTerm(char** txt)
{
	// Step up to terminator
	while ((**txt) && (**txt != '\t') && (**txt != '\r') && (**txt != '\n')) (*txt)++;
	
	char q = **txt;

	// null terminate and advance
	if ((**txt == '\r') && (*(*txt + 1) == '\n'))
	{
		**txt = '\0';
		(*txt) += 2;
	}
	else if ((**txt == '\t') || (**txt == '\r') || (**txt == '\n'))
	{
		**txt = '\0';
		(*txt)++;
	}
	
	return q;
}

