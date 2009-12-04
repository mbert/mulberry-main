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

// CPlugin.cp
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
//

#include "CPlugin.h"

#include "CAuthPlugin.h"
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CMulberryApp.h"
#include "COptionsMap.h"
#include "CPluginManager.h"
#include "CPreferenceKeys.h"
#include "CRegistration.h"
#include "CUnicodeStdLib.h"
#include "CUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"

//#include <winsock.h>
#include <UEnvironment.h>
#endif

#if __dest_os == __mac_os_x
#include "CLocalCommon.h"
#include <LCFString.h>
#include <LCFURL.h>
#endif

#pragma mark ____________________________consts
#if __dest_os == __mac_os
Str255 cPluginEntry = "\pMulberryPluginEntry";
#else
const char* cPluginEntry = "MulberryPluginEntry";
#endif

#pragma mark ____________________________CPluginInfo

/*
	This class is used to store information about the plug-in that is read
	in during initialisation. This information is cached so that the plug-in
	manager class can display information about the plug-in without having to reopen it.

	The plug-in should return a pointer to a copy of this class, and it should be copied
	by CPlugin in the application.
*/

// Constructor
CPlugin::CPluginInfo::CPluginInfo()
{
	*(long*) &mVersion = 0;
	mType = ePluginUnknown;
	mDataLength = 0;
	mData = NULL;
}

// Destructor
CPlugin::CPluginInfo::~CPluginInfo()
{
	// Deallocate all objects
	delete (char*) mData;
	mData = NULL;
}

// Copy another
CPlugin::CPluginInfo& CPlugin::CPluginInfo::operator=(const SPluginInfo& copy)
{
	mName = copy.mName;
	mVersion = copy.mVersion;
	mType = copy.mType;
	mManufacturer = copy.mManufacturer;
	mDescription = copy.mDescription;

	// Duplicate arbitrary data structure
	mDataLength = copy.mDataLength;
	if (mDataLength)
	{
		mData = new char[mDataLength];
		::memcpy((void*) mData, copy.mData, mDataLength);
	}

	return *this;
}

#pragma mark ____________________________CPlugin

CLog* CPlugin::sCurrentLog = NULL;

// Constructor
CPlugin::CPlugin(fspec file_spec)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mFSSpec = *file_spec;
	mResFile = 0;
#else
	mFname = file_spec;
#endif

#if __dest_os == __mac_os
	mConn = 0;
	mFragOffset = -1;
	mFragLength = -1;
#else
	mConn = 0;
#endif

	mProc = NULL;

	mRegistered = false;
	mDemo = false;
	mLoaded = 0;
	mRefCon = 0;

	mUsePreferences = false;
	mUpdateVers = false;

	// Create log
	mReplaced = false;
	mClone = false;
	mLog = NULL;
	mLog = new CLog;
	mLogProc = NULL;
}

// Clone for multi-access
CPlugin::CPlugin(const CPlugin& copy)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mResFile = copy.mResFile;
#endif

#if __dest_os == __mac_os
	mFragOffset = copy.mFragOffset;
	mFragLength = copy.mFragLength;
#endif

	mConn = copy.mConn;
	mProc = copy.mProc;

	mRegistered = copy.mRegistered;
	mDemo = copy.mDemo;
	mLoaded = 1;					// Must always be 1 so that load/unload never happens in this clone
	mRefCon = 0;					// Start with fresh copy

	mUsePreferences = copy.mUsePreferences;
	mUpdateVers = copy.mUpdateVers;

	mClone = true;
	mLog = copy.mLog;
}

// Destructor
CPlugin::~CPlugin()
{
	// Only delete these when not replaced
	if (!mReplaced && mRefCon)
	{
		// Force plugin to terminate - do not throw up
		try
		{
			TerminatePlugin();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}

		// Log is shared between parent and clones - only parent should delete it
		if (!mClone)
			delete mLog;
		else
			// Tell plug-in to initialise and get ref con
			CallPlugin(ePluginDestroy, NULL);
	}

	// Always do one more unload in case it has been permanently opened
	//if (!mReplaced)
	//	UnloadPlugin();
}

#pragma mark ____________________________Initialise/terminate

// Load information
bool CPlugin::InitPlugin(CPlugin*& plugin)
{
	bool result = false;
	std::pair<bool, bool> reg_result(false, false);
	
	{
		// Attempt to load
		StLoadPlugin _load(this);

		// See if it actually loaded - may not be right kind
		if (!mProc)
			return result;

		// Check if plug-in can run
		result = (CallPlugin(ePluginCanRun, NULL) == 1);
		if (!result)
			return result;

		// Tell plug-in to initialise and get ref con
		CallPlugin(ePluginInitialise, NULL);

		// Do call for information
		SPluginInfo* info = NULL;
		CallPlugin(ePluginGetInformation, &info);

#if __dest_os == __win32_os
		// Fix for enums are int change
		info->mType = static_cast<EPluginType>(0xff & info->mType);
#endif

		// Copy locally
		ProcessInfo(info);

		// Now get the actual plugin for this type
		plugin = CPluginManager::sPluginManager.CreateActualPluginType(this);

		// Now try re-doing the info for new type
		if (plugin)
		{
			// Reset the loader to new plugin
			_load.Reset(plugin);

			// Do call for information
			SPluginInfo* info = NULL;
			plugin->CallPlugin(ePluginGetInformation, &info);

			// Copy locally
			plugin->ProcessInfo(info);

			// Check registration
			reg_result = plugin->TestRegistration();

			// Verify version is OK for us to run, assuming registration was OK and we are not deleting
			result = reg_result.first && !reg_result.second && plugin->VerifyVersion();
		}
	}

	// Delete plugin if not required
	if (reg_result.first && reg_result.second)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		try
		{
			mFSSpec.Delete();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
#else
		::remove_utf8(mFname);
#endif
	}

	// Get rid of new plugin if not used
	if (!result && plugin)
		delete plugin;

	return result;
}

// Make sure version matches
bool CPlugin::VerifyVersion() const
{
	// Default
	return true;
}

// Replace old plugin with new
void CPlugin::ReplacePlugin(CPlugin* plugin)
{
	// Copy various fields
	delete mLog;
	mLog = plugin->mLog;
	mLogProc = plugin->mLogProc;
	mClone = plugin->mClone;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mFSSpec = plugin->mFSSpec;
	mResFile = plugin->mResFile;
#else
	mFname = plugin->mFname;
#endif

#if __dest_os == __mac_os
	mConn = plugin->mConn;
	mFragOffset = plugin->mFragOffset;
	mFragLength = plugin->mFragLength;
#else
	mConn = plugin->mConn;
#endif

	mProc = plugin->mProc;
	mRegistered = plugin->mRegistered;
	mDemo = plugin->mDemo;
	mLoaded = plugin->mLoaded;
	mRefCon = plugin->mRefCon;
	// mUsePreferences = plugin->mUsePreferences; <-- don't do this - honor the setting in the derived class
	mUpdateVers = plugin->mUpdateVers;
	
	// Reset original
	plugin->Replaced();
}

// Reset plugin as its been replaced
void CPlugin::Replaced()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mResFile = 0;
#endif

#if __dest_os == __mac_os
	mConn = 0;
	mFragOffset = -1;
	mFragLength = -1;
#else
	mConn = 0;
#endif

	mProc = NULL;

	mRegistered = false;
	mDemo = false;
	mLoaded = 0;
	mRefCon = 0;

	mUsePreferences = false;
	mUpdateVers = false;

	mClone = false;
	mLog = NULL;

	mReplaced = true;
}

// Clone plugin:
// assume its already been loaded and registration info etc checked
void CPlugin::ClonePlugin()
{
	// Attempt to load - will cache new refcon
	StLoadPlugin _load(this);

	// Tell plug-in to initialise and get ref con
	mRefCon = CallPlugin(ePluginConstruct, NULL);

	// Start logging for this new instance
	StartLogCallback();
}

// Process information from plugin
void CPlugin::ProcessInfo(SPluginInfo* info)
{
	mInfo = *info;

	// Log the name so we know what it is
	mLog->LogEntry(GetName());
}

// Terminate use
void CPlugin::TerminatePlugin(void)
{
	// Attempt to load
	StLoadPlugin _load(this);

	// Tell plug-in to terminate
	CallPlugin(ePluginTerminate, NULL);
}

#pragma mark ____________________________Load/unload

// Load/unload
void CPlugin::LoadPlugin(void)
{
	// Must obtain exclusive lock on this plugin before fiddling with reference count
	cdmutex::lock_cdmutex _lock(_mutex);

	// Only if not loaded by someone else
	if (mLoaded++)
		return;

	try
	{
		// Create connection to library
#if __dest_os == __mac_os_x
		// Get CFURL for item
		LCFURL bundleURL(mFSSpec.UseRef());
		
		mConn = ::CFBundleCreate(kCFAllocatorDefault, bundleURL);

		if (!mConn)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
#elif __dest_os == __win32_os
		mConn = ::LoadLibrary(mFname.win_str());

		// Throw if error
		if (!mConn)
		{
			long err_last = ::GetLastError();
			CLOG_LOGTHROW(CGeneralException, err_last);
			throw CGeneralException(err_last);
		}
#elif __dest_os == __linux_os
		mConn = dlopen(mFname.c_str(), RTLD_NOW);
		if (!mConn)
		{
			cerr << "Error loading plugin: " << dlerror() <<endl;
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

#else
#error __dest_os
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// lean up then throw
		::CloseResFile(mResFile);
		mResFile = 0;
#endif

		// Clean up
		mLoaded = 0;
		CLOG_LOGRETHROW;
		throw;
	}

	try
	{
		// Now try to get ptr to plug-in code
#if __dest_os == __mac_os

		CFragSymbolClass symClass;

#if !TARGET_RT_MAC_CFM && !TARGET_RT_MAC_MACHO // if this is classic 68k code...

		// Create a routine descriptor
		Ptr cfmProc;
		ThrowIfOSErr_(::FindSymbol(mConn, cPluginEntry, &cfmProc, &symClass));

		// Must check whether 68K native or 68K emulated on PPC
		// If 68K native use CFM68K, if 68K emulated use PPC CFM
		long response;
		bool is_68kCPU = (::Gestalt(gestaltSysArchitecture, &response) == noErr) && (response == gestalt68k);
		if (is_68kCPU)
			mProc = (PluginEntryPP) ::NewRoutineDescriptorTrap((ProcPtr) cfmProc, kPluginProcInfo, kM68kISA | kCFM68kRTA);
		else
			mProc = (PluginEntryPP) ::NewRoutineDescriptorTrap((ProcPtr) cfmProc, kPluginProcInfo, kPowerPCISA | kPowerPCRTA);

#else
		// Create CFM proc
		ThrowIfOSErr_(::FindSymbol(mConn, cPluginEntry, (Ptr*) &mProc, &symClass));

#endif

#elif __dest_os == __mac_os_x

		// Load function
		mProc = (PluginEntryPP) ::CFBundleGetFunctionPointerForName(mConn, LCFString(cPluginEntry));
		if (!mProc) 
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

#elif __dest_os == __win32_os

		mProc = (PluginEntryPP) ::GetProcAddress((HMODULE) mConn, cPluginEntry);

		// Throw if error
		if (!mProc)
		{
			long err_last = ::GetLastError();
			CLOG_LOGTHROW(CGeneralException, err_last);
			throw CGeneralException(err_last);
		}

#elif __dest_os == __linux_os
		mProc = (PluginEntryPP) dlsym(mConn, cPluginEntry);
		if (!mProc) 
		{
			//cerr << "Error loading plugin: " << dlerror() <<endl;
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
#else
#error __dest_os
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Must open res fork
		if (!mResFile)
			mResFile = ::FSOpenResFile(&mFSSpec.UseRef(), fsRdPerm);
#endif

		// Force construction
		mRefCon = CallPlugin(ePluginConstruct, NULL);

		// Start logging
		StartLogCallback();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up
	}
}

void CPlugin::UnloadPlugin(void)
{
	// Must obtain exclusive lock on this plugin before fiddling with reference count
	cdmutex::lock_cdmutex _lock(_mutex);

	// Only if not loaded by someone else
	if ((mLoaded == 0) || --mLoaded)
		return;

	// Stop logging
	StopLogCallback();

	// Force destruction
	if (mProc)
		CallPlugin(ePluginDestroy, NULL);
	mRefCon = 0;
	// Disconnect from fragment
#if __dest_os == __mac_os
	OSErr err = ::CloseConnection(&mConn);

	// Reset instance variables before throwing
	mConn = 0;
#if !TARGET_RT_MAC_CFM && !TARGET_RT_MAC_MACHO // if this is classic 68k code...
	if (mProc)
		::DisposeRoutineDescriptorTrap((UniversalProcPtr) mProc);
#endif
	mProc = NULL;

	// Throw if error
	if (err != noErr)
	{
		CLOG_LOGTHROW(CGeneralException, err);
		throw CGeneralException(err);
	}

#elif __dest_os == __mac_os_x
	::CFBundleUnloadExecutable(mConn);
	::CFRelease(mConn);

	// Reset instance variables before throwing
	mConn = 0;
	mProc = NULL;
#elif __dest_os == __win32_os
	bool err = !::FreeLibrary((HMODULE) mConn);

	// Reset instance variables before throwing
	mConn = 0;
	mProc = NULL;

	// Throw if error
	if (err)
	{
		long err_last = ::GetLastError();
		CLOG_LOGTHROW(CGeneralException, err_last);
		throw CGeneralException(err_last);
	}
#elif __dest_os == __linux_os
	int res = dlclose(mConn);

	// Reset instance variables before throwing
	mConn = 0;
	mProc = NULL;

	//To be honest, the linux man pages doesnt' say what
	//dlclose will return, so I guess we'll just assume we succeeded.
	//And you know what... who cares if we didn't? - honus
#else
#error __dest_os
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (mResFile)
	{
		::CloseResFile(mResFile);
		mResFile = 0;
	}
#endif

	// End log entry
	mLog->StopLog();
}

#pragma mark ____________________________Registration

// Test and do registration if required
// return.first - registration done
// return.second - delete plugin
std::pair<bool, bool> CPlugin::TestRegistration()
{
	std::pair<bool, bool> result(true, false);

	// Attempt to load - will cache new refcon
	StLoadPlugin _load(this);

	// Check registration
	if (UseRegistration())
	{
		// If app running in demo, then run plugin in demo
		if (CMulberryApp::sApp->IsDemo())
		{
			// Only allow demo if plugin allows demo
			if (CanDemo())
				mDemo = true;
			else
			{
				// Throw to prevent plugin running in demo
#ifdef __MULBERRY_V2
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
#else
				throw -1;
#endif
			}
		}

		else
		{
			// Compare with actual code
			if (!CRegistration::sRegistration.CheckCryptoRegistration())
			{
				bool allow_demo = CanDemo();

				// Check for demo
				mDemo = CRegistration::sRegistration.CryptoDemoValid();
				if (mDemo && CRegistration::sRegistration.CryptoDemoExpired())
				{
					// Should do alert here about demo expirary
					CErrorHandler::PutStopAlertRsrcStr("Alerts::General::IllegalPluginExpired", GetName().c_str());

					// Reset to no demo and prevent demo
					mDemo = false;
					allow_demo = false;
				}

				if (!mDemo)
				{
					// Force registration/demo (delete allowed)
					return DoRegistration(allow_demo, true);
				}
			}
			else
				mRegistered = true;
		}
	}
	
	return result;
}

// Does plug-in need to be registered
bool CPlugin::UseRegistration(void) const
{
	return (CallPlugin(ePluginUseRegistration, NULL) == 1);
}

// Can plug-in run in demo mode
bool CPlugin::CanDemo(void) const
{
	return (CallPlugin(ePluginCanDemo, NULL) == 1);
}

// Does plug-in need to be registered
unsigned long CPlugin::GetRegistrationKey(void) const
{
	unsigned long key = 0UL;
	CallPlugin(ePluginUseRegistration, &key);
	return key;
}

// Do registration
std::pair<bool, bool> CPlugin::DoRegistration(bool allow_demo, bool allow_delete)
{
	bool done = false;
	bool delete_it = false;

	try
	{
		// Try to register - throw if cancelled
		bool demo = false;
		if (!CRegistration::sRegistration.DoPluginRegister(*this, demo, delete_it, allow_demo, allow_delete))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		else
		{
			if (demo)
				mDemo = true;
			else
				mRegistered = true;
		}
		done = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	return std::pair<bool, bool>(done, delete_it);
}


#pragma mark ____________________________Preferences

// Write data to a stream
void CPlugin::WriteToMap(COptionsMap* theMap, bool dirty_only)
{
	// Only do if its got a name
	if (!mUsePreferences || mInfo.mName.empty())
		return;

	// Set up section
	StartSection(theMap);

	// Write version to file (if required)
	if (!dirty_only || mUpdateVers)
	{
		char temp[256];
		::snprintf(temp, 256, "%#08x",*(long*) &mInfo.mVersion);
		cdstring txt = temp;
		theMap->WriteValue(cVersionKey, txt);
	}

	// Write actually data
	DoWriteToMap(theMap, dirty_only);

	// Remove section
	StopSection(theMap);
}

// Read data from a stream
bool CPlugin::ReadFromMap(COptionsMap* theMap, NumVersion vers_app, NumVersion& vers_prefs)
{
	// Only do if its got a name
	if (!mUsePreferences || mInfo.mName.empty())
		return false;

	// Set up section
	StartSection(theMap);

	// Read prefs version
	NumVersion actual_vers;
	cdstring txt;
	theMap->ReadValue(cVersionKey, txt, vers_app);
	*(long*) &actual_vers = ::strtol(txt, NULL, 0);
	mUpdateVers = (*(long*) &actual_vers != *(long*) &mInfo.mVersion);

	DoReadFromMap(theMap, vers_prefs, actual_vers);

	// Remove section
	StopSection(theMap);

	return mUpdateVers;
}

// Start prefs map section
void CPlugin::StartSection(COptionsMap* theMap) const
{
	// Set up section '<<TYPE>>.<<NAME>>'
	switch(mInfo.mType)
	{
	case ePluginAddressIO:
		theMap->PushSection(cPluginAddressIO);
		break;
	case ePluginMailboxIO:
		theMap->PushSection(cPluginMailboxIO);
		break;
	case ePluginNetworkAuthentication:
		theMap->PushSection(cPluginNetworkAuthentication);
		break;
	case ePluginSecurity:
		theMap->PushSection(cPluginSecurity);
		break;
	case ePluginSpelling:
		theMap->PushSection(cPluginSpelling);
		break;
	case ePluginPswdChange:
		theMap->PushSection(cPluginPswdChange);
		break;
	case ePluginMailMerge:
		theMap->PushSection(cPluginMailMerge);
		break;
	default:
		// Unknown type - cannot do prefs
		return;
	}
	theMap->PushSection(mInfo.mName);
}

// Stop prefs map section
void CPlugin::StopSection(COptionsMap* theMap) const
{
	// Remove sections
	theMap->PopSection();	// '<<NAME>>'
	theMap->PopSection();	// '<<TYPE>>'
}

// Write data to a stream
void CPlugin::DoWriteToMap(COptionsMap* theMap, bool dirty_only)
{
	// Default - no preferences to write
}

// Read data from a stream
void CPlugin::DoReadFromMap(COptionsMap* theMap, NumVersion vers_prefs, NumVersion vers_plugin)
{
	// Default - no preferences to read
}

#pragma mark ____________________________Call

long CPlugin::CallPlugin(long code, void* data) const
{
	// Make sure log is set to this plugins log
	sCurrentLog = mLog;

	long result = (*mProc)(code, data, mRefCon);

	// Make sure log is cleared
	sCurrentLog = NULL;

	return result;
}

#pragma mark ____________________________Logging

// Logging call back
long CPlugin::LoggingCallback(const char* text)
{
	if (sCurrentLog)
		sCurrentLog->LogEntry(text);

	return 1;
}

// Start logging via callback
void CPlugin::StartLogCallback(void)
{
	// Create routine descriptor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Classic
	mLogProc = (LoggingCallbackProcPtr) NewRoutineDescriptor((ProcPtr) LoggingCallback, kLoggingCallbackProcInfo, GetCurrentArchitecture());
#else
	mLogProc = (LoggingCallbackProcPtr) LoggingCallback;
#endif
#else
	mLogProc = (LoggingCallbackProcPtr) LoggingCallback;
#endif

	// Create log entry
	mLog->StartLog(CLog::eLogPlugin, GetName());

	// Set log call back
	CallPlugin(ePluginSetLoggingCallback, (void*) mLogProc);
}

// Stop logging via callback
void CPlugin::StopLogCallback(void)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Classic
	if (mLogProc)
		DisposeRoutineDescriptor((UniversalProcPtr)mLogProc);
#endif
#endif
}
