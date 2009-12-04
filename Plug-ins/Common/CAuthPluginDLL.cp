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

// CAuthPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 9-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL basedauthorization plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the authorization DLL being called by Mulberry.
//
// History:
// CD:	 9-Dec-1997:	Created initial header and implementation.
// CD:	13-Dec-1997:	Turned into abstract base class
//						Added fields set by Mulberry
//						Defined error codes
// CD:	21-May-1998:	Changed server inputs to have server cname as 'real' input
//

#include "CAuthPluginDLL.h"

#include "CStringUtils.h"

#include <string.h>

#pragma mark ____________________________CAuthPluginDLL

const char* cINETDescriptors[] =
	{"Unknown",
	 "IMAP",
	 "POP3",
	 "SMTP",
	 "NNTP",
	 "IMSP",
	 "ACAP",
	 "LDAP",
	 "Whois++",
	 "Finger",
	 "Local Adbk",
	 "Manage SIEVE"
	 };

// Constructor
CAuthPluginDLL::CAuthPluginDLL()
{
	mServerType = eServerUnknown;
	mUseUserID = false;
	*mUserID = 0;
	*mRecoveredUserID = 0;
	mUsePassword = false;
	*mPassword = 0;
	*mServer = 0;
	*mRealServer = 0;
}

// Destructor
CAuthPluginDLL::~CAuthPluginDLL()
{
}

// DLL entry point and dispatch
long CAuthPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CAuthPluginDLL::eAuthSetServerTypeString:
		SetServerTypeString((const char*) data);
		return 1;
	case CAuthPluginDLL::eAuthUseUserID:
		UseUserID(((long) data) ? true : false);
		return 1;
	case CAuthPluginDLL::eAuthSetUserID:
		SetUserID((const char*) data);
		return 1;
	case CAuthPluginDLL::eAuthGetUserID:
		*((const char**) data) = GetUserID();
		return 1;
	case CAuthPluginDLL::eAuthUsePassword:
		UsePassword(((long) data) ? true : false);
		return 1;
	case CAuthPluginDLL::eAuthSetPassword:
		SetPassword((const char*) data);
		return 1;
	case CAuthPluginDLL::eAuthSetServer:
		SetServer((const char*) data);
		return 1;
	case CAuthPluginDLL::eAuthSetRealServer:
		SetRealServer((const char*) data);
		return 1;

	case CAuthPluginDLL::eAuthProcessData:
		return ProcessData((SAuthPluginData*) data);

	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}

// Server in use by Mulberry
void CAuthPluginDLL::SetServerTypeString(const char* type)
{
	mServerType = eServerUnknown;
	for(int i = eServerFirst; i <= eServerLast; i++)
	{
		if (::strcmpnocase(type, cINETDescriptors[i]) == 0)
		{
			mServerType = (EINETServerType) i;
			break;
		}
	}
}

// Use use user id from Mulberry
void CAuthPluginDLL::UseUserID(bool use_it)
{
	mUseUserID = use_it;
}

// Set user id from Mulberry
void CAuthPluginDLL::SetUserID(const char* str)
{
	::strncpy(mUserID, str, cMaxAuthStringLength - 1);
	mUserID[cMaxAuthStringLength - 1] = 0;
}

// Set user id from Mulberry
const char* CAuthPluginDLL::GetUserID()
{
	return mUseUserID ? mUserID : mRecoveredUserID;
}

// Set use password from Mulberry
void CAuthPluginDLL::UsePassword(bool use_it)
{
	mUsePassword = use_it;
}

// Set password from Mulberry
void CAuthPluginDLL::SetPassword(const char* str)
{
	::strncpy(mPassword, str, cMaxAuthStringLength - 1);
	mPassword[cMaxAuthStringLength - 1] = 0;
}

// Set server from Mulberry
void CAuthPluginDLL::SetServer(const char* str)
{
	::strncpy(mServer, str, cMaxAuthStringLength - 1);
	mServer[cMaxAuthStringLength - 1] = 0;
}

// Set proxy server from Mulberry
void CAuthPluginDLL::SetRealServer(const char* str)
{
	::strncpy(mRealServer, str, cMaxAuthStringLength - 1);
	mRealServer[cMaxAuthStringLength - 1] = 0;
}
