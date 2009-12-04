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

// CExamplePluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 23-May-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements an example configuration DLL based plug-in for use in Mulberry.
//
// History:
// 23-May-1999: Created initial header and implementation.
//

#include "CExamplePluginDLL.h"

#include <stdio.h>
#include <string.h>
#include "CPluginInfo.h"

// If plugin is for Mulberry v2, uncomment the next line
// #define MULBERRY_V2

#include "CPrefsStrings.h"


#define AUTHERROR(xx_msg) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "Example Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
  	return eConfigError; \
  } while (0)

#pragma mark ____________________________consts

const char* cPluginName = "Example Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginConfig;
const char* cPluginDescription = "Example configuration plugin for Mulberry." COPYRIGHT;
const char* cPluginConfigurationID = "Example";

#pragma mark ____________________________CExamplePluginDLL

// Constructor
CExamplePluginDLL::CExamplePluginDLL()
{
	mBuffer = NULL;
}

// Destructor
CExamplePluginDLL::~CExamplePluginDLL()
{
	delete mBuffer;
}

// Initialise plug-in
void CExamplePluginDLL::Initialise(void)
{
	// Do default
	CConfigPluginDLL::Initialise();
	
	// Prompt for user id and password only
	
	// NB Assumes that authentication mechanism uses user id & pswd

	mConfigInfo.mPromptStartup = true;
	mConfigInfo.mPromptUserID = true;
	mConfigInfo.mPromptPassword = true;
	mConfigInfo.mPromptRealName = false;
	mConfigInfo.mPromptServers = false;
	mConfigInfo.mServers = NULL;
}

// Does plug-in need to be registered
bool CExamplePluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CExamplePluginDLL::CanDemo(void)
{
	// Does not need to be registered
	return true;
}

// Test for run ability
bool CExamplePluginDLL::CanRun(void)
{
	// Always available
	return true;
}

// Returns the name of the plug-in
const char* CExamplePluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CExamplePluginDLL::GetVersion(void) const
{
#if __dest_os == __mac_os
	return cPluginVersion;
#elif __dest_os == __win32_os
	return ::htonl(cPluginVersion);
#endif
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CExamplePluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CExamplePluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CExamplePluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

#pragma mark ____________________________Modify these ->

// Mulberry will issue one GetPrefs call and then call MorePrefs
// If MorePrefs returns true Mulberry will do another GetPrefs call and another MorePrefs etc
// If MorePrefs returns false Mulberry will carry on and process all the preferences it has up to that point

// GetPrefs returns a string with a list of preferences in key=value format, separated
// by line ends (Mac OS: \r; Win32 \r\n; unix: \n)
// GetPrefs can return as many prefs in each call as it likes

// Return preference list to Mulberry
void CExamplePluginDLL::GetPrefs(const char** prefs)
{
	// IMPORTANT - must have user id at this point
	if (!mUserID || !*mUserID)
	{
		prefs = NULL;
		return;
	}

	// All we do here is return the mail account preference with the
	// server name changed to include the user id as a prefix, and with the
	// user id inserted into the authentication field

	// Allocate buffer for output
	size_t len = ::strlen(cMailAccountPreference) + 2 * ::strlen(mUserID);
	mBuffer = new char[len + 1];
	
	// Do c-style printf into buffer using template preference
	::sprintf(mBuffer, cMailAccountPreference, mUserID, mUserID);
	
	// Now return it
	*prefs = mBuffer;
}

// Return true if there are more preferences
void CExamplePluginDLL::MorePrefs(bool* more)
{
	// Example plugin returns all prefs in one GetPrefs call
	*more = false;
}
