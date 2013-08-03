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

// CVCardAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 19-Apr-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Communicator address book I/E DLL based plug-in for use in Mulberry.
//
// History:
// 19-Apr-1998: Created initial header and implementation.
// 30-Jun-1998: Created initial ldif import/export
//

#include "CVCardAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "cdstring.h"
#include "CStringUtils.h"

#include <stdlib.h>

#include <memory>
#include <strstream>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "vCard Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "vCard address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "vCard";

#pragma mark ____________________________CVCardAdbkIOPluginDLL

// Constructor
CVCardAdbkIOPluginDLL::CVCardAdbkIOPluginDLL()
{
}

// Destructor
CVCardAdbkIOPluginDLL::~CVCardAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CVCardAdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "vCard Exported Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CVCardAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CVCardAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CVCardAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CVCardAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CVCardAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CVCardAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CVCardAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CVCardAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Do the actual import
long CVCardAdbkIOPluginDLL::DoImport(const char* fname)
{
	long result = 0;

	// Open file
	FILE* file = NULL;
	unsigned char* data = NULL;

	try
	{
		// Open
		file = ::fopen(fname, "rb");
		if (!file)
			throw 1L;

		// Read in all data
		::fseek(file, 0, SEEK_END);
		long fileLength = ::ftell(file);

		data = (unsigned char*) ::malloc(fileLength + 1);
		if (!data)
			throw 1L;
		
		::rewind(file);
		
		::fread(data, 1, fileLength, file);

		// Close file
		::fclose(file);
		file = NULL;

		// Terminate
		data[fileLength] = 0;

		// Look for unicode input
		if ((fileLength > 2) && (data[0] == 0) && (data[1] != 0))
		{
			// Convert unicode to utf8 (latin1 subset only)
			int i = 0;
			int j = 0;
			for(; i < fileLength; i += 2)
			{
				if (data[i] == 0)
				{
					if (data[i + 1] < 0x80)
						data[j++] = data[i + 1];
					else
					{
						// Write 2 to buffer
						data[j++] = 0xc0 | (data[i + 1] >> 6);
						data[j++] = 0x80 | (data[i + 1] & 0x3f);
					}
				}
				else
					data[j++] = '?';
			}
			data[j++] = 0;
		}

		// Do import
		ImportAddresses((char*)data);
		
		// Finish with handle
		::free(data);
		data = NULL;
		
		result = 1;
	}
	catch(...)
	{
		if (data)
			::free(data);
		if (file)
			::fclose(file);
		result = 0;
	}

	return result;
}

long CVCardAdbkIOPluginDLL::ImportAddresses(char* data)
{
	// Turn data into stream
	std::istrstream in(data);
	SAdbkIOPluginAddress addr;
	addr.mName = NULL;
	addr.mEmail = NULL;
	addr.mNickName = NULL;
	addr.mCompany = NULL;
	addr.mAddress = NULL;
	addr.mPhoneWork = NULL;
	addr.mPhoneHome = NULL;
	addr.mFax = NULL;
	addr.mURL = NULL;
	addr.mNotes = NULL;
	while(mEngine.ReadOne(in, addr))
	{
		(*mImportCallback)(&addr, 0);
	
		// Clean out data
		delete addr.mName;
		delete addr.mEmail;
		delete addr.mNickName;
		delete addr.mCompany;
		delete addr.mAddress;
		delete addr.mPhoneWork;
		delete addr.mPhoneHome;
		delete addr.mFax;
		delete addr.mURL;
		delete addr.mNotes;

		addr.mName = NULL;
		addr.mEmail = NULL;
		addr.mNickName = NULL;
		addr.mCompany = NULL;
		addr.mAddress = NULL;
		addr.mPhoneWork = NULL;
		addr.mPhoneHome = NULL;
		addr.mFax = NULL;
		addr.mURL = NULL;
		addr.mNotes = NULL;
	}

	return 1;
}

// Export an address
long CVCardAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	std::ostrstream out;
	mEngine.WriteOne(out, *addr);
	out << std::ends;
	std::auto_ptr<char> data(out.str());
	::fwrite(data.get(), 1, ::strlen(data.get()), mExportFile);
	return 1;
}

// Export a group
long CVCardAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	return 1;
}
