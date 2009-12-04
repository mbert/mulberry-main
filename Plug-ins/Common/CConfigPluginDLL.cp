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

// CConfigPluginDLL.cp
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

#include "CConfigPluginDLL.h"

#include <string.h>

#pragma mark ____________________________CConfigPluginDLL

// Constructor
CConfigPluginDLL::CConfigPluginDLL()
{
	// NULL out fields
	mUserID = NULL;
	mPassword = NULL;
	mRealName = NULL;
	mServer = NULL;
}

// Destructor
CConfigPluginDLL::~CConfigPluginDLL()
{
	// Delete any allocated fields
	delete mUserID;
	delete mPassword;
	delete mRealName;
	delete mServer;
}

// DLL entry point and dispatch
long CConfigPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CConfigPluginDLL::eConfigSetInput:
		SetInput(((SConfigPluginInput*) data)->mUserId,
					((SConfigPluginInput*) data)->mPswd,
					((SConfigPluginInput*) data)->mRealName,
					((SConfigPluginInput*) data)->mServer);
		return 1;
	case CConfigPluginDLL::eConfigGetPrefs:
		GetPrefs((const char**) data);
		return 1;
	case CConfigPluginDLL::eConfigMorePrefs:
		MorePrefs((bool*) data);
		return 1;
	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}

// Set input from Mulberry
void CConfigPluginDLL::SetInput(const char* user, const char* pswd, const char* real_name, const char* server)
{
	mUserID = ::strdup(user);
	mPassword = ::strdup(pswd);
	mRealName = ::strdup(real_name);
	mServer = ::strdup(server);
}
