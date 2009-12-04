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

// CConfigPlugin.cp
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

#include "CConfigPlugin.h"

#include "CAdminLock.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CStringOptionsMap.h"

#pragma mark ____________________________CConfigPlugin

// Constructor
CConfigPlugin::CConfigPlugin(fspec file_spec) :
		CPlugin(file_spec)
{
}

void CConfigPlugin::DoConfiguration(const char* uid, const char* pswd, const char* real_name, const char* server)
{
	StLoadPlugin load(this);

	// See if user prompt was done
	if (PromptStartup())
	{
		const char* cuid = PromptUserID() ? uid : NULL;
		const char* cpswd = PromptPswd() ? pswd : NULL;
		const char* creal_name = PromptRealName() ? real_name : NULL;
		const char* cserver = PromptServers() ? server : NULL;

		// Now send to plugin
		SetInput(cuid, cpswd, creal_name, cserver);
	}

	cdstring prefs;

	do 
	{
		// Now read prefs from plugin
		const char* cprefs = NULL;
		GetPrefs(&cprefs);

		// Copy for tokenising
		if (!prefs.empty())
			prefs += "\r\n";
		prefs += cprefs;

	} while (MorePrefs());

	
	// Create and give to string map
	CStringOptionsMap map;
	map.SetString(&prefs);
	map.ReadMap();

	// Now read map into existing prefs (reset admin lock before)
	CAdminLock::sAdminLock.ResetLock();
	NumVersion vers_app = CMulberryApp::GetVersionNumber();
	NumVersion vers_prefs;
	CPreferences::sPrefs->ReadFromMap(&map, vers_app, vers_prefs);
	
	// Need to redo admin default items as they may have changed
	CAdminLock::sAdminLock.ProcessPrefs(CPreferences::sPrefs);

	// At this point prefs have been configured by plugin
}

// Process information from plugin
void CConfigPlugin::ProcessInfo(SPluginInfo* info)
{
	// Do default
	CPlugin::ProcessInfo(info);

	// Get authentication type ID from info
	mConfigInfo = *((SConfigPluginInfo*) info->mData);
	mServers = mConfigInfo.mServers;
	mConfigInfo.mServers = NULL;
}

// Set input to plugin
void CConfigPlugin::SetInput(const char* user,				// User data from Mulberry
								const char* pswd,
								const char* real_name,
								const char* server)
{
	SConfigPluginInput input;
	input.mUserId = user;
	input.mPswd = pswd;
	input.mRealName = real_name;
	input.mServer = server;

	CallPlugin(eConfigSetInput, (void*) &input);
}

// Get prefs text from plugin
void CConfigPlugin::GetPrefs(const char** prefs)
{
	CallPlugin(eConfigGetPrefs, (void*) prefs);
}

// See if plugin has more prefs
bool CConfigPlugin::MorePrefs()
{
	bool result = false;
	CallPlugin(eConfigMorePrefs, (void*) &result);
	return result;
}

