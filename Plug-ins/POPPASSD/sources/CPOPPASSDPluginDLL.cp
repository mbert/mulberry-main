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

// CPOPPASSDPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 10-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements POPPASSD password change DLL based plug-in for use in Mulberry.
//
// History:
// 10-Feb-1999: Created initial header and implementation.
//

#include "CPOPPASSDPluginDLL.h"
#include <string.h>
#include "CPluginInfo.h"

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#define AUTHERROR(xx_msg) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "POPPASSD Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
  	return ePswdChangeServerError; \
  } while (0)

#pragma mark ____________________________consts

const char* cPluginName = "POPPASSD Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginPswdChange;
const char* cPluginDescription = "POPPASSD password change plugin for Mulberry." COPYRIGHT;
const char* cPluginPswdChangeID = "POPPASSD";

#pragma mark ____________________________CPOPPASSDPluginDLL

// Constructor
CPOPPASSDPluginDLL::CPOPPASSDPluginDLL()
{
	mState = eUser;
}

// Destructor
CPOPPASSDPluginDLL::~CPOPPASSDPluginDLL()
{
}

// Initialise plug-in
void CPOPPASSDPluginDLL::Initialise(void)
{
	// Do default
	CPswdChangePluginDLL::Initialise();
	
	// Assign authentication ID
	::strncpy(mPswdChangeInfo.mUIName, cPluginPswdChangeID, 255);
	mPswdChangeInfo.mUIName[255] = 0;
	mPswdChangeInfo.mPromptPassword = true;
	mPswdChangeInfo.mUseNetwork = true;
}

// Does plug-in need to be registered
bool CPOPPASSDPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'PASS');
	return false;
}

// Can plug-in run as demo
bool CPOPPASSDPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CPOPPASSDPluginDLL::CanRun(void)
{
	// Always available
	return true;
}

// Returns the name of the plug-in
const char* CPOPPASSDPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CPOPPASSDPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CPOPPASSDPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CPOPPASSDPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CPOPPASSDPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Get server IP for Mulberry
const char* CPOPPASSDPluginDLL::GetServerIP()
{
	// If a configuration was set, use that, otherwise default to input server
	return (*mConfiguration ? mConfiguration : mServer);
}

// Get server port for Mulberry
tcp_port CPOPPASSDPluginDLL::GetServerPort()
{
	return 106;	// Default poppassd port
}

// Process data
long CPOPPASSDPluginDLL::DoIt()
{
	// Always an error
	return ePswdChangeError;
}

// Process data
long CPOPPASSDPluginDLL::ProcessData(SPswdChangePluginData* info)
{
	switch(mState)
	{
	case eUser:
		// Check response
		if (CheckResponse(info->data))
		{
			char* p = info->data;
			::strcpy(p, "user ");
			::strcat(p, mUserID);
			mState = ePass;
			return ePswdChangeSendGetData;
		}
		else
			return ePswdChangeServerError;
	case ePass:
		// Check response
		if (CheckResponse(info->data))
		{
			char* p = info->data;
			::strcpy(p, "pass ");
			::strcat(p, mOldPassword);
			mState = eNewPass;
			return ePswdChangeSendGetData;
		}
		else
			return ePswdChangeServerError;
	case eNewPass:
		// Check response
		if (CheckResponse(info->data))
		{
			char* p = info->data;
			::strcpy(p, "newpass ");
			::strcat(p, mNewPassword);
			mState = eQuit;
			return ePswdChangeSendGetData;
		}
		else
			return ePswdChangeServerError;
	case eQuit:
		// Check response
		if (CheckResponse(info->data))
		{
			// Special need to check for servers that close after the newpass
			// command rather than waiting for quit from client
			if (!::strncmp(info->data, "200 BYE", 7))
				return ePswdChangeDone;
			else
			{
				char* p = info->data;
				::strcpy(p, "quit");
				mState = eDone;
				return ePswdChangeSendGetData;
			}
		}
		else
			return ePswdChangeServerError;
	case eDone:
		// Check response
		if (CheckResponse(info->data))
			return ePswdChangeDone;
		else
			return ePswdChangeServerError;
	default:
		return ePswdChangeError;
	}
}

// Check response from server
bool CPOPPASSDPluginDLL::CheckResponse(const char* data) const
{
	return (::strncmp(data, "200", 3) == 0);
}
