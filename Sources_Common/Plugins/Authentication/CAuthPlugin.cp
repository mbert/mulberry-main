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

// CAuthPlugin.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based authentication plug-ins in Mulberry.
//
// History:
// CD:	13-Dec-1997:	Created initial header and implementation.
// CD:	17-Feb-1998:	Added DoAuthentication to handle entire authentication process.
// CD:	05-May-1998:	Added string based server types to avoid sync problem with plugin.
//

#include "CAuthPlugin.h"

#include "CAuthenticator.h"
#include "CLocalCommon.h"
#include "CMailControl.h"
#include "CTCPStream.h"
#include "CLog.h"

#include <memory>

#pragma mark ____________________________CAuthPlugin

// Process information from plugin
void CAuthPlugin::ProcessInfo(SPluginInfo* info)
{
	// Do default
	CPlugin::ProcessInfo(info);

	// Get authentication type ID from info
	mAuthTypeID = static_cast<const SAuthPluginInfo*>(info->mData)->mAuthTypeID;
	mPrefsDescriptor = static_cast<const SAuthPluginInfo*>(info->mData)->mPrefsDescriptor;
	mAuthUIType = static_cast<const SAuthPluginInfo*>(info->mData)->mAuthUIType;
}

// Plugin needs server's CNAME => do reverse lookup
bool CAuthPlugin::NeedCNAME() const
{
	// Fake it for now - ultimately this should be a plugin call
	return (mAuthUIType == eAuthKerberos);
}

// Set server type string from Mulberry
void CAuthPlugin::SetServerTypeString(const char* type)
{
	CallPlugin(eAuthSetServerTypeString, (void*) type);
}

// Set use user id from Mulberry
void CAuthPlugin::UseUserID(bool use_it)
{
	CallPlugin(eAuthUseUserID, (void*) use_it);
}

// Set user id from Mulberry
void CAuthPlugin::SetUserID(const char* str)
{
	CallPlugin(eAuthSetUserID, (void*) str);
}

// Set user id from Mulberry
const char* CAuthPlugin::GetUserID()
{
	const char* str = nil;
	CallPlugin(eAuthGetUserID, (void*) &str);
	return str;
}

// Set use password from Mulberry
void CAuthPlugin::UsePassword(bool use_it)
{
	CallPlugin(eAuthUsePassword, (void*) use_it);
}

// Set password from Mulberry
void CAuthPlugin::SetPassword(const char* str)
{
	CallPlugin(eAuthSetPassword, (void*) str);
}

// Set UserID from Mulberry
void CAuthPlugin::SetServer(const char* str)
{
	CallPlugin(eAuthSetServer, (void*) str);
}

// Set UserID from Mulberry
void CAuthPlugin::SetRealServer(const char* str)
{
	CallPlugin(eAuthSetRealServer, (void*) str);
}

// Process data
long CAuthPlugin::ProcessData(char* data, long length)
{
	SAuthPluginData info;
	info.length = length;
	info.data = data;

	return CallPlugin(eAuthProcessData, &info);
}

#pragma mark ____________________________Do Authentication

bool CAuthPlugin::DoAuthentication(const CAuthenticator* acct_auth,
									CINETAccount::EINETServerType type, const char* type_string,
									CTCPStream& stream, CLog& log, char* buffer, size_t buflen)
{
	bool result = true;

	// This is complicated by need for multithreaded access to authenticator
	// What needs to happen is that this plugin is loaded (if not loaded in another thread)
	// to make sure shared library procptr is correct
	// Then the plugin is copied and a clone made (cloning reinits the mRefNum to get a
	// unique data section in shared library)
	// Then plugin calls go through clone
	// Clone is destroyed on completion
	// This plugin is then unloaded (if not loaded by another thread)

	// Load plugin
	//StLoadPlugin load(this);
	if (!IsLoaded())
		LoadPlugin();

	// Now clone it
	std::auto_ptr<CAuthPlugin> clone(new CAuthPlugin(*this));
	clone->ClonePlugin();

	// Start authentication process
	clone->SetServerTypeString(type_string);

	switch(mAuthUIType)
	{
	case eAuthUserPswd:
		{
			CAuthenticatorUserPswd* auth = (CAuthenticatorUserPswd*) acct_auth->GetAuthenticator();

			// Set info
			clone->UseUserID(true);
			clone->SetUserID(auth->GetUID().c_str());
			clone->UsePassword(true);
			clone->SetPassword(auth->GetPswd().c_str());
			clone->SetServer(cdstring::null_str);
			clone->SetRealServer(cdstring::null_str);
		}
		break;
	case eAuthKerberos:
		{
			CAuthenticatorKerberos* auth = (CAuthenticatorKerberos*) acct_auth->GetAuthenticator();

			// Set info
			clone->UseUserID(!auth->GetDefaultUID());
			if (auth->GetDefaultUID())
				clone->SetUserID(cdstring::null_str);
			else
				clone->SetUserID(auth->GetUID().c_str());
			clone->UsePassword(false);
			clone->SetPassword(cdstring::null_str);
			clone->SetServer(auth->GetServerPrincipal().c_str());
			if (auth->GetDefaultPrincipal())
			{
				clone->SetServer(cdstring::null_str);
				clone->SetRealServer(stream.GetRemoteCName());
			}
			else
			{
				clone->SetServer(auth->GetServerPrincipal().c_str());
				clone->SetRealServer(stream.GetRemoteCName());
			}
		}
	default:;
	}



	// Send first command
	switch(type)
	{
	case CINETAccount::eIMAP:
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		::strcpy(buffer, "a AUTHENTICATE ");
		if (type == CINETAccount::eACAP)
			::strcat(buffer, "\"");
		::strcat(buffer, mAuthTypeID.c_str());
		if (type == CINETAccount::eACAP)
			::strcat(buffer, "\"");
		break;
	case CINETAccount::eSMTP:
	case CINETAccount::ePOP3:
		::strcpy(buffer, "AUTH ");
		::strcat(buffer, mAuthTypeID.c_str());
		break;
	case CINETAccount::eManageSIEVE:
		::strcpy(buffer, "AUTHENTICATE ");
		::strcat(buffer, "\"");
		::strcat(buffer, mAuthTypeID.c_str());
		::strcat(buffer, "\"");
		break;
	default:;
	}
	stream << buffer << net_endl << std::flush;
	if (CLog::AllowAuthenticationLog())
		log.LogEntry(buffer);
	stream.qgetline(buffer, buflen);
	//buffer[::strlen(buffer) - 1] = 0;
	if (CLog::AllowAuthenticationLog())
		log.LogEntry(buffer);

	bool done = false;
	CAuthPlugin::EAuthPluginReturnCode plugin_result;
	while(!done)
	{
		bool read_more = false;
		plugin_result = (CAuthPlugin::EAuthPluginReturnCode) clone->ProcessData(buffer, buflen);
		switch(plugin_result)
		{
		case CAuthPlugin::eAuthError:
		case CAuthPlugin::eAuthServerError:
			result = false;
			done = true;
			break;
		case CAuthPlugin::eAuthDone: // Complete - success
			done = true;
			break;
		case CAuthPlugin::eAuthSendDataToServer:
			// Send to server
			stream << buffer << net_endl << std::flush;
			if (CLog::AllowAuthenticationLog())
				log.LogEntry(buffer);
			
			// Get another line
			read_more = true;
			break;
		case CAuthPlugin::eAuthSendLiteralDataToServer:
			{
				// Send literal to server
				long len = *(long*) buffer;
				const char* start = &buffer[4];
				stream.write(start, len);
				stream << net_endl << std::flush;
				
				// Get another line
				read_more = true;
			}
			break;
		case CAuthPlugin::eAuthMoreData:
			// Get another line
			read_more = true;
			break;
		case CAuthPlugin::eAuthMoreLiteralData:	// More data to come
			{
				// Read literal
				long len = *(long*) buffer;
				if (len)
				{
					stream.read(buffer, len);
					buffer[len] = 0;
				}
				else
					*buffer = 0;
				
				// Read in terminating CRLF but ignore it
				char dummy[256];
				stream.qgetline(dummy, 256);
				if (CLog::AllowAuthenticationLog())
					log.LogEntry(buffer);
			}
			break;
		default:;
			// Keep going
		}
		
		// Read some more if required
		if (read_more)
		{
			bool another_line = true;
			while(another_line)
			{
				// Get line of data from stream
				stream.qgetline(buffer, buflen);
				if (CLog::AllowAuthenticationLog())
					log.LogEntry(buffer);
				
				// Check for valid data
				// IMAP/IMSP may send unsolicited CAPABILITY which we MUST intercept here
				switch(type)
				{
				case CINETAccount::eIMAP:
				case CINETAccount::eIMSP:
					// Check for unsolicited
					if ((buffer[0] == '*') && (buffer[1] == ' '))
						// Keep going and get another lines
						break;
					
					// Fall through - this line should be passed to the plugin
				default:
					// Got a valid line for the plugin
					another_line = false;
					break;
				}
			}
		}
	}

	// Always recover user id after login attempt
	switch(mAuthUIType)
	{
	case eAuthKerberos:
		{
			CAuthenticatorKerberos* auth = (CAuthenticatorKerberos*) acct_auth->GetAuthenticator();
			cdstring recovered = clone->GetUserID();
			auth->SetRecoveredUID(recovered);
		}
	default:;
	}

	return result;
}
