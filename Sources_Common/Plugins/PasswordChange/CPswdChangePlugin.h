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


// CPswdChangePlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 10-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based password changing plug-ins in Mulberry.
//
// History:
// CD:	10-Feb-1999:	Created initial header and implementation.
//

#ifndef __CPSWDCHANGEPLUGIN__MULBERRY__
#define __CPSWDCHANGEPLUGIN__MULBERRY__

#include "CPlugin.h"

#include "CPreferenceValue.h"
#include "CTCPSocket.h"

// Classes
class CINETAccount;
class CLog;
class CTCPStream;

class CPswdChangePlugin : public CPlugin
{
public:

	// Actual plug-in class

	CPswdChangePlugin(fspec file_spec);
	virtual ~CPswdChangePlugin() {}

	// Initialisation
	virtual void ProcessInfo(SPluginInfo* info);		// Process information from plugin

	// Get information
	virtual bool Enabled() const						// Is it enabled
		{ return mEnabled.GetValue(); }
	virtual const char* GetUIName() const				// Return name to use in UI
		{ return mPswdChangeInfo.mUIName; }
	virtual bool	PromptPassword() const				// Does plugin do import
		{ return mPswdChangeInfo.mPromptPassword; }
	virtual bool	UseNetwork() const					// Does plugin do export
		{ return mPswdChangeInfo.mUseNetwork; }

	// Call codes
	virtual const char* GetServerIP();					// Return server address to use in network ops
	virtual tcp_port GetServerPort();					// Return server port to use in network ops
	virtual bool	DoPswdChange(const CINETAccount* acct_auth);
	virtual void	ProcessPswdChange(CTCPStream& stream,
										CLog& log,
										char* buffer,
										size_t buflen);

protected:
	enum EPswdChangePluginCall
	{
		// Derived plug-in calls
		ePswdChangeSetServer = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
		ePswdChangeSetRealServer,
		ePswdChangeSetUserID,
		ePswdChangeSetOldPassword,
		ePswdChangeSetNewPassword,
		ePswdChangeSetConfiguration,
		ePswdChangeGetServerIP,
		ePswdChangeGetServerPort,
		ePswdChangeDoIt,
		ePswdChangeProcessData
	};

	enum EPswdChangePluginReturnCode
	{
		ePswdChangeError = 0,
		ePswdChangeServerError,
		ePswdChangeDone,
		ePswdChangeSendData,
		ePswdChangeGetData,
		ePswdChangeSendGetData
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Plug-in information class
	struct SPswdChangePluginInfo
	{
		char mUIName[256];								// Name for menu
		bool mPromptPassword;							// Plugin wants Mulberry to prompt for new password
		bool mUseNetwork;								// Plugin wants Mulberry to create network connection
	};

	// Data structure containing text from/to server
	struct SPswdChangePluginData
	{
		long length;							// Length of buffed
		char* data;								// input: c-string containing last line from server
												// output: c_string containing next line sent to server (no CRLF at end)
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	SPswdChangePluginInfo mPswdChangeInfo;
	CPreferenceValueMap<bool> mEnabled;
	CPreferenceValueMap<cdstring> mConfiguration;

	// Calls codes
	virtual void	SetServer(const char* str);				// Set server from Mulberry (this may be an alias)
	virtual void	SetRealServer(const char* str);			// Set real server from Mulberry (this is the cname)
	virtual void	SetUserID(const char* str);				// Set UserID from Mulberry
	virtual void	SetOldPassword(const char* str);		// Set password from Mulberry
	virtual void	SetNewPassword(const char* str);		// Set password from Mulberry
	virtual void	SetConfiguration(const char* str);		// Set prefs config from Mulberry

	virtual long	DoIt();									// Get plugin to do it
	virtual long	ProcessData(char* data, long length);	// Process data

	// Preferences
	virtual void 	DoWriteToMap(COptionsMap* theMap, bool dirty_only);	// Write data to a stream
	virtual void	DoReadFromMap(COptionsMap* theMap,
							NumVersion vers_prefs,
							NumVersion vers_plugin);				// Read data from a stream
};

#endif
