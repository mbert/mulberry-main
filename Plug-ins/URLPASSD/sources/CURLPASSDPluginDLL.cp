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

// CURLPASSDPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 10-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements URLPASSD password change DLL based plug-in for use in Mulberry.
//
// History:
// 10-Feb-1999: Created initial header and implementation.
//

#include "CURLPASSDPluginDLL.h"
#include <stdlib.h>
#include <string.h>
#include "CPluginInfo.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if TARGET_API_MAC_CARBON || TARGET_API_MAC_MACHO
#include <InternetConfig.h>
#else
#include <ICAPI.h>
#endif
#endif

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#define AUTHERROR(xx_msg) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "URLPASSD Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
  	return ePswdChangeServerError; \
  } while (0)

#pragma mark ____________________________consts

const char* cPluginName = "URLPASSD Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginPswdChange;
const char* cPluginDescription = "URL password change plugin for Mulberry." COPYRIGHT;
const char* cPluginPswdChangeID = "URLPASSD";

#pragma mark ____________________________CURLPASSDPluginDLL

// Constructor
CURLPASSDPluginDLL::CURLPASSDPluginDLL()
{
	mState = eUser;
}

// Destructor
CURLPASSDPluginDLL::~CURLPASSDPluginDLL()
{
}

// Initialise plug-in
void CURLPASSDPluginDLL::Initialise(void)
{
	// Do default
	CPswdChangePluginDLL::Initialise();
	
	// Assign authentication ID
	::strncpy(mPswdChangeInfo.mUIName, cPluginPswdChangeID, 255);
	mPswdChangeInfo.mUIName[255] = 0;
	mPswdChangeInfo.mPromptPassword = false;
	mPswdChangeInfo.mUseNetwork = false;
}

// Does plug-in need to be registered
bool CURLPASSDPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'PASS');
	return false;
}

// Can plug-in run as demo
bool CURLPASSDPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CURLPASSDPluginDLL::CanRun(void)
{
	// Always available
	return true;
}

// Returns the name of the plug-in
const char* CURLPASSDPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CURLPASSDPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CURLPASSDPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CURLPASSDPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CURLPASSDPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Process data
long CURLPASSDPluginDLL::DoIt()
{
	bool result = false;

	const char* launch = mConfiguration;
	
	// If it exists launch it
	if (!*mConfiguration)
		launch = "http://www.cyrusoft.com/mulberry/urlpassfail.html";

	// Do substitution
	char* url = (char*) ::malloc(::strlen(launch) + ::strlen(mServer) + ::strlen(mUserID) + 1);
	
	const char* p = launch;
	char* q = url;
	while(*p)
	{
		switch(*p)
		{
		case '%':
			switch(*(p+1))
			{
			case 'd':
				{
					char* r = ::strchr(mServer, '.');
					if (r)
					{
						::strcpy(q, r+1);
						q += ::strlen(r+1);
					}
					else
					{
						::strcpy(q, mServer);
						q += ::strlen(mServer);
					}
					p += 2;
				}
				break;
			case 's':
				::strcpy(q, mServer);
				q += ::strlen(mServer);
				p += 2;
				break;
			case 'u':
				::strcpy(q, mUserID);
				q += ::strlen(mUserID);
				p += 2;
				break;
			default:
				*q++ = *p++;
				break;
			}
			break;
		default:
			*q++ = *p++;
		}
	}
	*q = 0;

	result = LaunchURL(url);
	::free(url);

	return result ? ePswdChangeDone : ePswdChangeError;
}

// Process data
long CURLPASSDPluginDLL::ProcessData(SPswdChangePluginData* info)
{
	// Always an error
	return ePswdChangeError;
}

bool CURLPASSDPluginDLL::LaunchURL(const char* url)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	// Try to start IC
	short refnum = ::CurResFile();
	ICInstance instance;
	OSStatus err = ::ICStart(&instance, 'Mlby');
	::UseResFile(refnum);
	

#if TARGET_API_MAC_CLASSIC
	refnum = ::CurResFile();
	err = ::ICFindConfigFile(instance, 0, NULL);
	::UseResFile(refnum);
	if (err)
		return false;
#endif

	// Launch a URL
	long start = 0;
	long stop = strlen(url);
	err = ::ICLaunchURL(instance, "\p", (char*) url, ::strlen(url), &start, &stop);

	// Done with IC
	refnum = ::CurResFile();
	::ICStop(instance);
	::UseResFile(refnum);
	
	return (err == noErr);

#elif __dest_os == __win32_os
	char dir[MAX_PATH];
	if (::GetCurrentDirectory(MAX_PATH, dir))
	{
		HINSTANCE hinst = ::ShellExecute(::GetDesktopWindow(), "open", url, NULL, dir, 0);
		
		return (((int) hinst) > 32);
	}
	else
		return false;
#else __dest_os == __linux_os
	return false;
#endif
}
