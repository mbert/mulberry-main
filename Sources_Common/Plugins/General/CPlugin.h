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

// CPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 16-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based plug-ins in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for accessing the DLL.
//
// History:
// 16-Nov-1997: Created initial header and implementation.
// 14-Jan-1998: Combined adbk & mbox import & export into single IO plugin.
//

#ifndef __CPLUGIN__MULBERRY__
#define __CPLUGIN__MULBERRY__

#include "CLog.h"

#include "cdmutex.h"

#if __dest_os == __mac_os
#include <CodeFragments.h>
#include <MixedMode.h>
#endif

#if __dest_os == __mac_os_x
#include <PPxFSObject.h>
#endif

#include "cdstring.h"

// Typedefs

#ifdef __cplusplus
extern "C" {
#endif
typedef long (*PluginEntryPP)(long, void*, long);

#if __dest_os == __mac_os
enum
{
     kPluginProcInfo = kCStackBased
   | RESULT_SIZE(SIZE_CODE(sizeof(long)))
   | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
   | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*)))
   | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
 };
#endif
 
typedef long (*LoggingCallbackProcPtr)(const char* text);

#if __dest_os == __mac_os
enum
{
     kLoggingCallbackProcInfo = kCStackBased
   | RESULT_SIZE(SIZE_CODE(sizeof(long)))
   | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(const char*)))
};
#endif

#ifdef __cplusplus
}
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
typedef const PPx::FSObject* fspec;
typedef const FSSpec* fspec_old;
#else
typedef const char* fspec;
typedef const char* fspec_old;
#endif

// Classes
class COptionsMap;

class CPlugin
{
public:

	// Enums used for various codes
	enum EPluginType
	{
		ePluginUnknown = 0,
		ePluginAddressIO,
		ePluginMailboxIO,
		ePluginNetworkAuthentication,
		ePluginSecurity,
		ePluginSpelling,
		ePluginPswdChange,
		ePluginMailMerge,
		ePluginConfig,
		ePluginSSL,
		ePluginLast
	};

	enum EPluginCall
	{
		ePluginConstruct = 1,					// Construct new plugin object
		ePluginDestroy,							// Destroy plugin object
		ePluginInitialise,						// First call to plug-in - returns refCon
		ePluginTerminate,						// Last call to plug-in
		ePluginCanRun,							// Can it be run?
		ePluginStatus,							// Get status from plug-in
		ePluginGetInformation,					// Get plug-in info structure
		
		// Registration related
		ePluginUseRegistration,
		ePluginCanDemo,
		
		// Log related
		ePluginSetLoggingCallback,

		// Derived plug-in calls
		ePluginFirstSubCode = 1024				// First code availble for sub-class calls
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Plug-in information sent by plug-in
	struct SPluginInfo
	{
		const char* mName;						// Name of plug-in
		long		mVersion;					// Version number of plugin (as per Mac OS 'vers' rsrc)
		EPluginType	mType;						// Plug-in type
		const char* mManufacturer;				// Manufacturer
		const char* mDescription;				// Textual description
		long		mDataLength;				// Length of data
		const void*	mData;						// Plug-in specific info
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	// Plug-in information class stored locally
	class CPluginInfo
	{
		friend class CPlugin;					// Allow CPlugin access only
		CPluginInfo();
		~CPluginInfo();

		CPluginInfo& operator=(const SPluginInfo& copy);

		cdstring 	mName;						// Name of plug-in
		long	 	mVersion;					// Version number of plugin (as per Mac OS 'vers' rsrc)
		EPluginType	mType;						// Plug-in type
		cdstring	mManufacturer;				// Manufacturer
		cdstring	mDescription;				// Textual description
		long		mDataLength;				// Length of data
		const void*	mData;						// Plug-in specific info
	};

	// Actual plug-in class

	CPlugin(fspec file_spec);
	CPlugin(const CPlugin& copy);
	virtual ~CPlugin();
	
	// Initialise/terminate
	virtual bool InitPlugin(CPlugin*& plugin);			// Load, cache info
	virtual bool VerifyVersion() const;					// Make sure version matches
	virtual void ReplacePlugin(CPlugin* plugin);		// Replace old plugin with new
	virtual void Replaced();							// Reset plugin as its been replaced

	virtual void ClonePlugin();
	virtual void ProcessInfo(SPluginInfo* info);		// Process information from plugin
	virtual void TerminatePlugin();					// Unload

	// Load/unload
	virtual void LoadPlugin();
	virtual void UnloadPlugin();
	virtual bool IsLoaded() const
		{ return mLoaded; }

	// Get information
	virtual const cdstring& GetName() const			// Returns the name of the plug-in
		{ return mInfo.mName; }
	virtual long GetVersion() const					// Returns the version number of the plug-in
		{ return mInfo.mVersion; }
	virtual EPluginType GetType() const				// Returns the type of the plug-in
		{ return mInfo.mType; }
	virtual const cdstring& GetManufacturer() const	// Returns manufacturer of plug-in
		{ return mInfo.mManufacturer; }
	virtual const cdstring& GetDescription() const	// Returns description of plug-in
		{ return mInfo.mDescription; }
	virtual const void* GetData() const				// Returns data of plug-in
		{ return mInfo.mData; }
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual const PPx::FSObject& GetSpec() const
		{ return mFSSpec; }
	virtual short GetResFile() const
		{ return mResFile; }
#else
	virtual const cdstring& GetFName() const
		{ return mFname; }
#endif
#if __dest_os == __mac_os
	virtual CFragConnectionID GetConnection() const
		{ return mConn; }
#elif __dest_os == __mac_os_x
	virtual CFBundleRef GetConnection() const
		{ return mConn; }
#elif __dest_os == __win32_os
	virtual HMODULE GetModule() const
		{ return (HMODULE) mConn; }
	virtual HMODULE GetConnection() const
		{ return (HMODULE) mConn; }
#elif __dest_os == __linux_os
	virtual void* GetConnection() const
		{ return mConn; }
#else
#error __dest_os
#endif

	// Registration
	virtual bool IsRegistered()	const					// Is it registered
		{ return mRegistered; }
	virtual bool IsDemo() const							// Is it in demo mode
		{ return mDemo; }
	virtual unsigned long GetRegistrationKey() const;	// Get registration key from plugin
	virtual std::pair<bool, bool> DoRegistration(bool allow_demo, bool allow_delete);			// Register it

	// Preferences
	virtual void 	WriteToMap(COptionsMap* theMap, bool dirty_only);	// Write data to a stream
	virtual bool	ReadFromMap(COptionsMap* theMap,
							NumVersion vers_app,
							NumVersion& vers_prefs);			// Read data from a stream

protected:
	mutable CLog* mLog;
	static CLog* sCurrentLog;
	LoggingCallbackProcPtr mLogProc;
	bool mReplaced;
	bool mClone;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject	mFSSpec;
	short			mResFile;
#else
	cdstring mFname;
#endif
#if __dest_os == __mac_os
	CFragConnectionID mConn;
	long mFragOffset;
	long mFragLength;
#elif __dest_os == __mac_os_x
	CFBundleRef mConn;
#elif __dest_os == __win32_os
	HANDLE mConn;
#elif __dest_os == __linux_os
	void* mConn;
#else
#error __dest_os
#endif
	CPluginInfo mInfo;									// Information about plug-in
	PluginEntryPP mProc;								// Proc ptr for plug-in entry
	bool mRegistered;									// Is it registered
	bool mDemo;											// Is it in demo mode
	long mLoaded;										// Reference count of loads
	long mRefCon;										// Reference constant used by plugin: not used in app
	bool mUsePreferences;								// Does it read/write preferences?
	bool mUpdateVers;									// Must update prefs version
	cdmutex _mutex;										// Multithread lock

	// Registration
	virtual std::pair<bool, bool> TestRegistration();		// Test and do registration if required
	virtual bool UseRegistration() const;				// Does plug-in need to be registered
	virtual bool CanDemo() const;						// Can plug-in run as demo

	// Preferences
	virtual void 	DoWriteToMap(COptionsMap* theMap, bool dirty_only);	// Write data to a stream
	virtual void	DoReadFromMap(COptionsMap* theMap,
							NumVersion vers_prefs,
							NumVersion vers_plugin);				// Read data from a stream

	// Do call
	virtual long CallPlugin(long code, void* data) const;

private:

	// Logging call back
	static long LoggingCallback(const char* text);				// Callback for logging

	void	StartLogCallback();								// Start logging via callback
	void	StopLogCallback();								// Stop logging via callback

	// Preferences
	void	StartSection(COptionsMap* theMap) const;			// Start prefs map section
	void	StopSection(COptionsMap* theMap) const;				// Stop prefs map section
};

class StLoadPlugin
{
public:
	StLoadPlugin(CPlugin* aPlugin)
		{ mPlugin = aPlugin; mPlugin->LoadPlugin(); }
	~StLoadPlugin()
		{ if (mPlugin) mPlugin->UnloadPlugin(); }
	void Reset(CPlugin* plugin)
		{ mPlugin = plugin; }
private:
	CPlugin* mPlugin;
};

typedef std::vector<CPlugin*> CPluginList;

#endif
