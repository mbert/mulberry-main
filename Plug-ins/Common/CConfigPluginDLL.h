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

// CConfigPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 23-May-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based configuration plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the DLL being called by Mulberry.
//
// History:
// CD:	23-May-1999:	Created initial header and implementation.
//

#pragma once

#include "CPluginDLL.h"

// Classes
class CConfigPluginDLL : public CPluginDLL
{
public:

	enum EConfigPluginCall
	{
		// Derived plug-in calls
		eConfigSetInput = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
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

	// Actual plug-in class

	CConfigPluginDLL();
	virtual ~CConfigPluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	virtual void	SetInput(const char* user,				// User data from Mulberry
								const char* pswd,
								const char* real_name,
								const char* server);
	virtual void GetPrefs(const char** prefs) = 0;			// Return preference list to Mulberry
	virtual void MorePrefs(bool* more) = 0;					// Return preference list to Mulberry

protected:
	SConfigPluginInfo	mConfigInfo;	// Plugin requirements
	const char* mUserID;				// User ID provided by Mulberry
	const char* mPassword;				// User password provided by Mulberry
	const char* mRealName;				// User real name provided by Mulberry
	const char* mServer;				// User server provided by Mulberry

	// These should be returned by specific sub-class
	virtual long GetDataLength(void) const					// Returns data of plug-in
		{ return sizeof(SConfigPluginInfo); }
	virtual const void* GetData(void) const					// Returns data of plug-in
		{ return &mConfigInfo; }
};
