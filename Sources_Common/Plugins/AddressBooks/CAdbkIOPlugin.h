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


// CAdbkIOPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based address IO plug-ins in Mulberry.
//
// History:
// CD:	13-Jan-1998:	Created initial header and implementation.
//

#ifndef __CADBKIOPLUGIN__MULBERRY__
#define __CADBKIOPLUGIN__MULBERRY__

#include "CPlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long (*ImportCallbackProcPtr)(void* data, long group);

long ImportCallback(void* data, long group);

#if __dest_os == __mac_os
enum
{
     kAdbkImportCallbackProcInfo = kCStackBased
   | RESULT_SIZE(SIZE_CODE(sizeof(long)))
   | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void*)))
   | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
};
#endif

#ifdef __cplusplus
}
#endif

// Classes
class CAddressList;
class CGroupList;

class CAdbkIOPlugin : public CPlugin
{
	friend long ImportCallback(void* data, long group);

public:

	// Actual plug-in class

	CAdbkIOPlugin(fspec file_spec) :
		CPlugin(file_spec) {}
	virtual ~CAdbkIOPlugin() {}

	// Initialisation
	virtual void ProcessInfo(SPluginInfo* info);			// Process information from plugin

	// Get information
	virtual const char* GetUIName() const				// Return name to use in UI
		{ return mAdbkIOInfo.mAdbkIOType; }
	virtual bool	DoesImport() const					// Does plugin do import
		{ return mAdbkIOInfo.mDoesImport; }
	virtual bool	DoesExport() const					// Does plugin do export
		{ return mAdbkIOInfo.mDoesExport; }
#if __dest_os == __mac_os
	virtual const SFTypeList& GetFileType() const		// Returns the file type list
#else
	virtual const char* GetFileType() const				// Returns the file type list
#endif
		{ return mAdbkIOInfo.mTypes; }
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual OSType GetCreator() const
		{ return mAdbkIOInfo.mCreator; }
#endif

	// Call codes
	virtual long	DoImport(const fspec file_spec, CAddressList& addrs, CGroupList& grps);		// Do import
	virtual long	DoExport(const fspec file_spec, CAddressList& addrs, CGroupList& grps);		// Do export

protected:
	enum EAdbkIOPluginCall
	{
		// Derived plug-in calls
		eAdbkIOSetCallback = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
		eAdbkIODoImport,
		eAdbkIOStartExport,
		eAdbkIOExportAddress,
		eAdbkIOExportGroup,
		eAdbkIOStopExport
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Plug-in information class
	struct SAdbkIOPluginInfo
	{
		char mAdbkIOType[256];							// IO name for menu
		bool mDoesImport;
		bool mDoesExport;
#if __dest_os == __mac_os
		SFTypeList mTypes;
		OSType mCreator;
#elif __dest_os == __mac_os_x
		char mTypes[256];
		OSType mCreator;
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

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	// Class to handle group information
	class CAdbkIOPluginGroup
	{
	public:
		SAdbkIOPluginGroup mGroup;
		
		CAdbkIOPluginGroup();
		~CAdbkIOPluginGroup();
		
		void AddAddress(const char* addr);
		SAdbkIOPluginGroup* GetGroupData();

	private:
		long mNumAddresses;
	};

	SAdbkIOPluginInfo mAdbkIOInfo;

	virtual void	SetCallback(ImportCallbackProcPtr proc);	// Set callback into Mulberry
};

#endif
