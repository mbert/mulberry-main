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

// CAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based addressbook import for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the addressbook import DLL being called by Mulberry.
//
// History:
// CD:	13-Jan-1998:	Created initial header and implementation.
//

#include "CAdbkIOPluginDLL.h"

#include <stdlib.h>
#include <string.h>

#pragma mark ____________________________CAdbkIOPluginDLL

// Constructor
CAdbkIOPluginDLL::CAdbkIOPluginDLL()
{
	mImportCallback = NULL;
	mAdbkIOInfo.mAdbkIOType[0] = 0;
	mAdbkIOInfo.mDoesImport = false;
	mAdbkIOInfo.mDoesExport = false;
	
	mExportFile = NULL;
}

// Destructor
CAdbkIOPluginDLL::~CAdbkIOPluginDLL()
{
}

// DLL entry point and dispatch
long CAdbkIOPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CAdbkIOPluginDLL::eAdbkIOSetCallback:
		SetCallback((ImportCallbackProcPtr) data);
		return 1;

	case CAdbkIOPluginDLL::eAdbkIODoImport:
		// Only if import callback setup
		if (mImportCallback)
		{
			long result = DoImport((const char*) data);
			mImportCallback = NULL;
			return result;
		}
		else
			return 0;

	case CAdbkIOPluginDLL::eAdbkIOStartExport:
		return StartExport((const char*) data);

	case CAdbkIOPluginDLL::eAdbkIOExportAddress:
		return ExportAddress((SAdbkIOPluginAddress*) data);

	case CAdbkIOPluginDLL::eAdbkIOExportGroup:
		return ExportGroup((SAdbkIOPluginGroup*) data);

	case CAdbkIOPluginDLL::eAdbkIOStopExport:
		return StopExport();

	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}

// Do the actual import
long CAdbkIOPluginDLL::DoImport(const char* fname)
{
	long result = 0;

	// Open file
	FILE* file = NULL;
	char* data = NULL;

	try
	{
		// Open
		file = ::fopen(fname, "rb");
		if (!file)
			throw 1L;

		// Read in all data
		::fseek(file, 0, SEEK_END);
		long fileLength = ::ftell(file);

		data = (char*) ::malloc(fileLength + 1);
		if (!data)
			throw 1L;
		
		::rewind(file);
		
		::fread(data, 1, fileLength, file);

		// Close file
		::fclose(file);
		file = NULL;

		// Terminate
		data[fileLength] = 0;

		// Do import
		ImportAddresses(data);
		
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

// Start export
long CAdbkIOPluginDLL::StartExport(const char* fname)
{
	// Open
	mExportFile = ::fopen(fname, "wb");

	return mExportFile != NULL;
}

// Stop export
long CAdbkIOPluginDLL::StopExport(void)
{
	// Close
	if (mExportFile)
		::fclose(mExportFile);
	mExportFile = NULL;

	return 1;
}

#pragma mark ____________________________CAdbkIOPluginAddress

CAdbkIOPluginDLL::CAdbkIOPluginAddress::CAdbkIOPluginAddress()
{
	mAddress.mNumFields = 10;
	mAddress.mNickName = NULL;
	mAddress.mName = NULL;
	mAddress.mEmail = NULL;
	mAddress.mCompany = NULL;
	mAddress.mAddress = NULL;
	mAddress.mPhoneWork = NULL;
	mAddress.mPhoneHome = NULL;
	mAddress.mFax = NULL;
	mAddress.mURL = NULL;
	mAddress.mNotes = NULL;
}

CAdbkIOPluginDLL::CAdbkIOPluginAddress::~CAdbkIOPluginAddress()
{
}

CAdbkIOPluginDLL::SAdbkIOPluginAddress* CAdbkIOPluginDLL::CAdbkIOPluginAddress::GetAddressData(void)
{
	return &mAddress;
}

#pragma mark ____________________________CAdbkIOPluginGroup

CAdbkIOPluginDLL::CAdbkIOPluginGroup::CAdbkIOPluginGroup()
{
	mGroup.mNumFields = 3;
	mGroup.mNickName = NULL;
	mGroup.mName = NULL;
	mGroup.mAddresses = NULL;
	
	mNumAddresses = 0;
}

CAdbkIOPluginDLL::CAdbkIOPluginGroup::~CAdbkIOPluginGroup()
{
	if (mGroup.mAddresses)
		::free(mGroup.mAddresses);
}

void CAdbkIOPluginDLL::CAdbkIOPluginGroup::AddAddress(const char* addr)
{
	mNumAddresses++;

	if (!mGroup.mAddresses)
		mGroup.mAddresses = (const char**) ::malloc(sizeof(char*));
	else
		mGroup.mAddresses = (const char**) ::realloc(mGroup.mAddresses, mNumAddresses * sizeof(char*));
	
	mGroup.mAddresses[mNumAddresses - 1] = addr;
}

CAdbkIOPluginDLL::SAdbkIOPluginGroup* CAdbkIOPluginDLL::CAdbkIOPluginGroup::GetGroupData(void)
{
	// Must add empty address into list for terminator if some exist
	if (mNumAddresses)
		AddAddress(NULL);
	
	return &mGroup;
}
