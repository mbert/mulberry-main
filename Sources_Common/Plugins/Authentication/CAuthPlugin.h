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

// CAuthPlugin.h
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

#ifndef __CAUTHPLUGIN__MULBERRY__
#define __CAUTHPLUGIN__MULBERRY__

#include "CPlugin.h"

#include "CINETAccount.h"

// Classes
class CAuthenticator;
class CLog;
class CTCPStream;

class CAuthPlugin : public CPlugin
{
public:

	enum EAuthPluginUIType
	{
		eAuthUserPswd,
		eAuthKerberos,
		eAuthAnonymous
	};

	// Actual plug-in class

	CAuthPlugin(fspec files) :
		CPlugin(files) {}
	CAuthPlugin(const CAuthPlugin& copy) :
		CPlugin(copy)
		{ mAuthTypeID = copy.mAuthTypeID; }
	virtual ~CAuthPlugin() {}

	// Initialisation
	virtual void ProcessInfo(SPluginInfo* info);			// Process information from plugin

	// Get information
	virtual const cdstring& GetAuthTypeID() const			// Returns the authenticate type ID
		{ return mAuthTypeID; }
	virtual const cdstring& GetPrefsDescriptor() const		// String to use in Prefs and popups
		{ return mPrefsDescriptor; }
	virtual EAuthPluginUIType GetAuthUIType() const			// Get UI type from plugin
		{ return mAuthUIType; }

	virtual bool NeedCNAME() const;							// Plugin needs server's CNAME => do reverse lookup

	// Calls codes
	virtual bool	DoAuthentication(const CAuthenticator* acct_auth,
										CINETAccount::EINETServerType type,
										const char* type_string,
										CTCPStream& stream,
										CLog& log,
										char* buffer,
										size_t buflen,
										cdstring& capabilities);

protected:
	enum EAuthPluginCall
	{
		// Derived plug-in calls
		eAuthSetServerTypeString = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
		eAuthUseUserID,
		eAuthSetUserID,
		eAuthGetUserID,
		eAuthUsePassword,
		eAuthSetPassword,
		eAuthSetServer,
		eAuthSetRealServer,
		eAuthProcessData
	};

	enum EAuthPluginReturnCode
	{
		eAuthError = 0,
		eAuthServerError,
		eAuthDone,
		eAuthSendDataToServer,
		eAuthSendLiteralDataToServer,
		eAuthMoreData,
		eAuthMoreLiteralData
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Plug-in information class
	struct SAuthPluginInfo
	{
		char mAuthTypeID[256];					// Case-insensitive Auth type
		char mPrefsDescriptor[256];				// Visual auth name
		EAuthPluginUIType mAuthUIType;			// Type of UI required for user
	};

	// Data structure containing text from/to server
	struct SAuthPluginData
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

	cdstring mAuthTypeID;									// Authentication type ID
	cdstring mPrefsDescriptor;								// Visual name
	EAuthPluginUIType mAuthUIType;							// Type of UI required for user

	// Calls codes
	virtual void	SetServerTypeString(const char* type);	// Set string type of server in use in Mulberry
	virtual void	UseUserID(bool use_it);					// Use user id from Mulberry
	virtual void	SetUserID(const char* str);				// Set UserID from Mulberry
	virtual const char*	GetUserID();						// Get UserID from plugin (in case where uid is externally set - e.g Kerberos)
	virtual void	UsePassword(bool use_it);				// Use password from Mulberry
	virtual void	SetPassword(const char* str);			// Set password from Mulberry
	virtual void	SetServer(const char* str);				// Set server from Mulberry (this may be an alias)
	virtual void	SetRealServer(const char* str);			// Set real server from Mulberry (this is the cname)

	virtual long	ProcessData(char* data, long length);	// Process data
};

#endif
