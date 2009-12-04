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


// CPswdChangePlugin.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 10-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based password changing for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the addressbook IO DLL being called by Mulberry.
//
// History:
// CD:	10-Feb-1999:	Created initial header and implementation.
//

#include "CPswdChangePlugin.h"

#include "CAdminLock.h"
#ifndef LINUX_NOTYET
#include "CChangePswdDialog.h"
#endif
#include "CINETAccount.h"
#include "CINETClient.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CPreferenceKeys.h"
#include "CPswdChangeClient.h"

#include "cdstring.h"

#pragma mark ____________________________CPswdChangePlugin

// Constructor
CPswdChangePlugin::CPswdChangePlugin(fspec file_spec) :
		CPlugin(file_spec)
{
	mUsePreferences = true;
	mEnabled.Value() = CAdminLock::sAdminLock.mUsePASSD;
}

// Process information from plugin
void CPswdChangePlugin::ProcessInfo(SPluginInfo* info)
{
	// Do default
	CPlugin::ProcessInfo(info);

	// Get authentication type ID from info
	mPswdChangeInfo = *((SPswdChangePluginInfo*) info->mData);
}

// Set server from Mulberry
void CPswdChangePlugin::SetServer(const char* str)
{
	CallPlugin(ePswdChangeSetServer, (void*) str);
}

// Set real server from Mulberry
void CPswdChangePlugin::SetRealServer(const char* str)
{
	CallPlugin(ePswdChangeSetRealServer, (void*) str);
}

// Set user id from Mulberry
void CPswdChangePlugin::SetUserID(const char* str)
{
	CallPlugin(ePswdChangeSetUserID, (void*) str);
}

// Set user id from Mulberry
void CPswdChangePlugin::SetOldPassword(const char* str)
{
	CallPlugin(ePswdChangeSetOldPassword, (void*) str);
}

// Set user id from Mulberry
void CPswdChangePlugin::SetNewPassword(const char* str)
{
	CallPlugin(ePswdChangeSetNewPassword, (void*) str);
}

// Set configuration from Mulberry
void CPswdChangePlugin::SetConfiguration(const char* str)
{
	CallPlugin(ePswdChangeSetConfiguration, (void*) str);
}

// Set server from Mulberry
const char* CPswdChangePlugin::GetServerIP()
{
	const char* result = NULL;
	CallPlugin(ePswdChangeGetServerIP, (void*) &result);
	return result;
}

// Set server from Mulberry
tcp_port CPswdChangePlugin::GetServerPort()
{
	tcp_port result = 0;
	CallPlugin(ePswdChangeGetServerPort, (void*) &result);
	return result;
}

// Get Plugin to do it
long CPswdChangePlugin::DoIt()
{
	return CallPlugin(ePswdChangeDoIt, NULL);
}

// Process data
long CPswdChangePlugin::ProcessData(char* data, long length)
{
	SPswdChangePluginData info;
	info.length = length;
	info.data = data;

	return CallPlugin(ePswdChangeProcessData, &info);
}

#pragma mark ____________________________Do Password change

bool CPswdChangePlugin::DoPswdChange(const CINETAccount* acct)
{
	// See if password required
	cdstring old_pass;
	cdstring new_pass;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (PromptPassword() && !CChangePswdDialog::PromptPasswordChange(acct, old_pass, new_pass, CMulberryApp::sApp))
#elif __dest_os == __win32_os
	if (PromptPassword() && !CChangePswdDialog::PromptPasswordChange(acct, old_pass, new_pass))
#elif __dest_os == __linux_os
#warning Must do for Linux
#else
#error __dest_os
#endif
		return false;

	// Load plugin
	StLoadPlugin load(this);

	// Give data to plugin
	SetServer(acct->GetServerIP());

	cdstring uid;
	if (acct->GetAuthenticator().RequiresUserPswd())
		uid = acct->GetAuthenticatorUserPswd()->GetUID();
	else if (acct->GetAuthenticator().RequiresKerberos())
	{
		const CAuthenticatorKerberos* kerbv4 = acct->GetAuthenticatorKerberos();
		if (!kerbv4->GetDefaultUID())
			uid = kerbv4->GetUID();
	}
	SetUserID(uid);

	if (PromptPassword())
	{
		SetOldPassword(old_pass);
		SetNewPassword(new_pass);
	}
	SetConfiguration(mConfiguration.GetValue());

	// Now see what plugin wants to do
	if (UseNetwork())
	{
		bool result = true;
		try
		{
			CPswdChangeClient client;
			client.ChangePassword(this);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			result = false;
		}

		return result;
	}
	else
		return (DoIt() != ePswdChangeError);
}

void CPswdChangePlugin::ProcessPswdChange(CTCPStream& stream, CLog& log, char* buffer, size_t buflen)
{
	bool result = true;

	// Receive first info from server
	stream.qgetline(buffer, buflen);

	// Write to log file
	log.LogEntry(buffer);

	bool done = false;
	CPswdChangePlugin::EPswdChangePluginReturnCode plugin_result;
	while(!done)
	{
		plugin_result = (CPswdChangePlugin::EPswdChangePluginReturnCode) ProcessData(buffer, buflen);
		switch(plugin_result)
		{
		case CPswdChangePlugin::ePswdChangeError:
			CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_BadResponse);
			throw CINETClient::CINETException(CINETClient::CINETException::err_BadResponse);
			break;
		case CPswdChangePlugin::ePswdChangeServerError:
			CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
			throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
			break;
		case CPswdChangePlugin::ePswdChangeDone: // Complete - success
			done = true;
			break;
		case CPswdChangePlugin::ePswdChangeSendData:		// Send to server
			stream << buffer << net_endl << std::flush;
			if (CLog::AllowAuthenticationLog())
				log.LogEntry(buffer);
			break;
		case CPswdChangePlugin::ePswdChangeGetData:			// Get from server
			stream.qgetline(buffer, buflen);
			if (CLog::AllowAuthenticationLog())
				log.LogEntry(buffer);
			break;
		case CPswdChangePlugin::ePswdChangeSendGetData:		// Send to server, then get from server
			stream << buffer << net_endl << std::flush;
			if (CLog::AllowAuthenticationLog())
				log.LogEntry(buffer);
			stream.qgetline(buffer, buflen);
			if (CLog::AllowAuthenticationLog())
				log.LogEntry(buffer);
			break;
		default:;
			// Keep going
		}
	}
}

#pragma mark ____________________________Preferences

// Write data to a stream
void CPswdChangePlugin::DoWriteToMap(COptionsMap* theMap, bool dirty_only)
{
	// Write all to map
	mEnabled.WriteToMap(cPswdChangeEnabledKey, theMap, dirty_only);
	mConfiguration.WriteToMap(cPswdChangeConfigKey, theMap, dirty_only);
}

// Read data from a stream
void CPswdChangePlugin::DoReadFromMap(COptionsMap* theMap, NumVersion vers_prefs, NumVersion vers_plugin)
{
	// Read all to map
	mEnabled.ReadFromMap(cPswdChangeEnabledKey, theMap, vers_prefs);
	mConfiguration.ReadFromMap(cPswdChangeConfigKey, theMap, vers_prefs);
}
