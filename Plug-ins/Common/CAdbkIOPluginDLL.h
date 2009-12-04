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

// CAdbkIOPluginDLL.h
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

#ifndef __ADBKIO_PLUGIN_MULBERRY__
#define __ADBKIO_PLUGIN_MULBERRY__

#include "CPluginDLL.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <Carbon/Carbon.h>
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long (*ImportCallbackProcPtr)(void* data, long group);

#ifdef __cplusplus
}
#endif

// Classes
class CAdbkIOPluginDLL : public CPluginDLL
{
public:

	enum EAdbkIOPluginCall
	{
		// Derived plug-in calls
		eAdbkIOSetCallback = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
		eAdbkIODoImport,
		eAdbkIOStartExport,
		eAdbkIOExportAddress,
		eAdbkIOExportGroup,
		eAdbkIOStopExport
	};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
typedef OSType                          SFTypeList[4];
#endif

	// Plug-in information class
	struct SAdbkIOPluginInfo
	{
		char mAdbkIOType[256];							// IO name for menu
		bool mDoesImport;
		bool mDoesExport;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		SFTypeList	mTypes;
		OSType		mCreator;
#else
		char mTypes[256];
#endif

	};

	struct SAdbkIOPluginAddress
	{
		long mNumFields;
		const char* mNickName;
		const char* mName;
		const char* mEmail;
		const char* mCompany;
		const char* mAddress;
		const char* mPhoneWork;
		const char* mPhoneHome;
		const char* mFax;
		const char* mURL;
		const char* mNotes;
	};

	struct SAdbkIOPluginGroup
	{
		long mNumFields;
		const char* mNickName;
		const char* mName;
		const char** mAddresses;
	};

	// Class to handle address information
	class CAdbkIOPluginAddress
	{
	public:
		SAdbkIOPluginAddress mAddress;
		
		CAdbkIOPluginAddress();
		~CAdbkIOPluginAddress();

		SAdbkIOPluginAddress* GetAddressData(void);
	};

	// Class to handle group information
	class CAdbkIOPluginGroup
	{
	public:
		SAdbkIOPluginGroup mGroup;
		
		CAdbkIOPluginGroup();
		~CAdbkIOPluginGroup();
		
		void AddAddress(const char* addr);
		SAdbkIOPluginGroup* GetGroupData(void);

	private:
		long mNumAddresses;
	};

	// Actual plug-in class

	CAdbkIOPluginDLL();
	virtual ~CAdbkIOPluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	virtual void	SetCallback(ImportCallbackProcPtr proc)		// Set callback into Mulberry
		{ mImportCallback = proc; }
	virtual long	DoImport(const char* fname);					// Do import
	virtual long	StartExport(const char* fname);					// Do import
	virtual long	ExportAddress(SAdbkIOPluginAddress* addr) = 0;	// Export an address
	virtual long	ExportGroup(SAdbkIOPluginGroup* grp) = 0;		// Export a group
	virtual long	StopExport(void);								// Stop export

protected:
	SAdbkIOPluginInfo mAdbkIOInfo;					// Information about plug-in
	ImportCallbackProcPtr mImportCallback;
	FILE* mExportFile;

	// These should be returned by specific sub-class
	virtual long GetDataLength(void) const					// Returns data size of plug-in
		{ return sizeof(SAdbkIOPluginInfo); }
	virtual const void* GetData(void) const					// Returns data of plug-in
		{ return &mAdbkIOInfo; }
	
	virtual long ImportAddresses(char* data) = 0;			// Do the actual import - pure virtual - must do in subclass
};

#endif
