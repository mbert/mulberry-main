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

// CPswdChangePluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 12-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based password changing plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the authorization DLL being called by Mulberry.
//
// History:
// CD:	12-Feb-1999:	Created initial header and implementation.
//

#ifndef __PSWDCHANGE_PLUGIN_MULBERRY__
#define __PSWDCHANGE_PLUGIN_MULBERRY__

#include "CPluginDLL.h"

const int cMaxPswdChangeStringLength = 256;

typedef unsigned short tcp_port;

// Classes
class CPswdChangePluginDLL : public CPluginDLL
{
public:

	enum EPswdChangePluginCall
	{
		// Derived plug-in calls
		ePswdChangeSetServer = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
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

	// Actual plug-in class

	CPswdChangePluginDLL();
	virtual ~CPswdChangePluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	virtual void	SetServer(const char* str);				// Set server from Mulberry (this may be an alias)
	virtual void	SetRealServer(const char* str);			// Set real server from Mulberry (this is the cname)
	virtual void	SetUserID(const char* str);				// Set UserID from Mulberry
	virtual void	SetOldPassword(const char* str);		// Set old password from Mulberry
	virtual void	SetNewPassword(const char* str);		// Set new password from Mulberry
	virtual void	SetConfiguration(const char* str);		// Set configuration from Mulberry
	virtual const char* GetServerIP();						// Return server address to use in network ops
	virtual tcp_port GetServerPort();						// Return server port to use in network ops

	virtual long	DoIt() = 0;								// Get plugin to do it
	virtual long	ProcessData(SPswdChangePluginData* info) = 0;	// Process data

protected:
	SPswdChangePluginInfo mPswdChangeInfo;						// Information about plug-in
	char mUserID[cMaxPswdChangeStringLength];				// User ID provided by Mulberry
	char mOldPassword[cMaxPswdChangeStringLength];			// User old password provided by Mulberry
	char mNewPassword[cMaxPswdChangeStringLength];			// User new password provided by Mulberry
	char mServer[cMaxPswdChangeStringLength];				// User server provided by Mulberry (this is user specified value)
	char mRealServer[cMaxPswdChangeStringLength];			// User real server provided by Mulberry (this is the actual server cname)
	char mConfiguration[cMaxPswdChangeStringLength];		// User configuration provided by Mulberry (this is the actual server cname)

	// These should be returned by specific sub-class
	virtual long GetDataLength(void) const					// Returns data of plug-in
		{ return sizeof(SPswdChangePluginData); }
	virtual const void* GetData(void) const					// Returns data of plug-in
		{ return &mPswdChangeInfo; }
};

#endif
