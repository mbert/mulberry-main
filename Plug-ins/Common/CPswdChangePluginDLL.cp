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

// CPswdChangePluginDLL.cp
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

#include "CPswdChangePluginDLL.h"

#include <string.h>

#pragma mark ____________________________CPswdChangePluginDLL

// Constructor
CPswdChangePluginDLL::CPswdChangePluginDLL()
{
	*mUserID = 0;
	*mOldPassword = 0;
	*mNewPassword = 0;
	*mServer = 0;
	*mRealServer = 0;
	*mConfiguration = 0;
}

// Destructor
CPswdChangePluginDLL::~CPswdChangePluginDLL()
{
}

// DLL entry point and dispatch
long CPswdChangePluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CPswdChangePluginDLL::ePswdChangeSetServer:
		SetServer((const char*) data);
		return 1;
	case CPswdChangePluginDLL::ePswdChangeSetRealServer:
		SetRealServer((const char*) data);
		return 1;
	case CPswdChangePluginDLL::ePswdChangeSetUserID:
		SetUserID((const char*) data);
		return 1;
	case CPswdChangePluginDLL::ePswdChangeSetOldPassword:
		SetOldPassword((const char*) data);
		return 1;
	case CPswdChangePluginDLL::ePswdChangeSetNewPassword:
		SetNewPassword((const char*) data);
		return 1;
	case CPswdChangePluginDLL::ePswdChangeSetConfiguration:
		SetConfiguration((const char*) data);
		return 1;

	case CPswdChangePluginDLL::ePswdChangeGetServerIP:
		*((const char**) data) = GetServerIP();
		return 1;
	case CPswdChangePluginDLL::ePswdChangeGetServerPort:
		*((tcp_port*) data) = GetServerPort();
		return 1;

	case CPswdChangePluginDLL::ePswdChangeDoIt:
		return DoIt();

	case CPswdChangePluginDLL::ePswdChangeProcessData:
		return ProcessData((SPswdChangePluginData*) data);

	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}

// Set server from Mulberry
void CPswdChangePluginDLL::SetServer(const char* str)
{
	::strncpy(mServer, str, cMaxPswdChangeStringLength - 1);
	mServer[cMaxPswdChangeStringLength - 1] = 0;
}

// Set proxy server from Mulberry
void CPswdChangePluginDLL::SetRealServer(const char* str)
{
	::strncpy(mRealServer, str, cMaxPswdChangeStringLength - 1);
	mRealServer[cMaxPswdChangeStringLength - 1] = 0;
}

// Set user id from Mulberry
void CPswdChangePluginDLL::SetUserID(const char* str)
{
	::strncpy(mUserID, str, cMaxPswdChangeStringLength - 1);
	mUserID[cMaxPswdChangeStringLength - 1] = 0;
}

// Set old password from Mulberry
void CPswdChangePluginDLL::SetOldPassword(const char* str)
{
	::strncpy(mOldPassword, str, cMaxPswdChangeStringLength - 1);
	mOldPassword[cMaxPswdChangeStringLength - 1] = 0;
}

// Set new password from Mulberry
void CPswdChangePluginDLL::SetNewPassword(const char* str)
{
	::strncpy(mNewPassword, str, cMaxPswdChangeStringLength - 1);
	mNewPassword[cMaxPswdChangeStringLength - 1] = 0;
}

// Set new password from Mulberry
void CPswdChangePluginDLL::SetConfiguration(const char* str)
{
	::strncpy(mConfiguration, str, cMaxPswdChangeStringLength - 1);
	mConfiguration[cMaxPswdChangeStringLength - 1] = 0;
}

// Get server IP for Mulberry
const char* CPswdChangePluginDLL::GetServerIP()
{
	return mServer;
}

// Get server port for Mulberry
tcp_port CPswdChangePluginDLL::GetServerPort()
{
	return 0;
}
