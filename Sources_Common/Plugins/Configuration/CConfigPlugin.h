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

// CConfigPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 23-May-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based configuration plug-ins in Mulberry.
//
// History:
// CD:	23-May-1999:	Created initial header and implementation.
//

#ifndef __CCONFIGPLUGIN__MULBERRY__
#define __CCONFIGPLUGIN__MULBERRY__

#include "CPlugin.h"

// Classes
class CConfigPlugin : public CPlugin
{
public:

	// Actual plug-in class

	CConfigPlugin(fspec file_spec);
	virtual ~CConfigPlugin() {}

			void DoConfiguration(const char* uid, const char* pswd, const char* real_name, const char* server);

	// Initialisation
	virtual void ProcessInfo(SPluginInfo* info);		// Process information from plugin

	// Call codes
	virtual void SetInput(const char* user,				// User data from Mulberry
								const char* pswd,
								const char* real_name,
								const char* server);
	virtual void GetPrefs(const char** prefs);			// Return preference list to Mulberry
	virtual bool MorePrefs();							// More preferences to be sent by plugin

	// Properties
	bool PromptStartup() const
		{ return mConfigInfo.mPromptStartup; }
	bool PromptUserID() const
		{ return mConfigInfo.mPromptUserID; }
	bool PromptPswd() const
		{ return mConfigInfo.mPromptPassword; }
	bool PromptRealName() const
		{ return mConfigInfo.mPromptRealName; }
	bool PromptServers() const
		{ return mConfigInfo.mPromptServers; }
	const cdstring& GetServers() const
		{ return mServers; }

protected:
	enum EConfigPluginCall
	{
		// Derived plug-in calls
		eConfigSetInput = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
		eConfigGetPrefs,
		eConfigMorePrefs
	};

	enum EConfigPluginReturnCode
	{
		eConfigError = 0
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Plug-in information class
	struct SConfigPluginInfo
	{
		bool mPromptStartup;							// Plugin wants Mulberry to show multi-user dialog
		bool mPromptUserID;								// Plugin wants Mulberry to prompt for user id
		bool mPromptPassword;							// Plugin wants Mulberry to prompt for password
		bool mPromptRealName;							// Plugin wants Mulberry to prompt for real name
		bool mPromptServers;							// Plugin wants Mulberry to prompt for servers
		const char* mServers;							// List of servers or NULL
	};

	// Data structure containing text from/to server
	struct SConfigPluginInput
	{
		const char* mUserId;							// User id entered by user or NULL
		const char* mPswd;								// Password entered by user or NULL
		const char* mRealName;							// Real name entered by user or NULL
		const char* mServer;							// Server chosen by user or NULL
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	SConfigPluginInfo mConfigInfo;
	cdstring mServers;

	// Calls codes

	// Actions
};

#endif
