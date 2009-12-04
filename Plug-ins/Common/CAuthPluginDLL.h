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

// CAuthPluginDLL.h
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

#ifndef __AUTH_PLUGIN_MULBERRY__
#define __AUTH_PLUGIN_MULBERRY__

#include "CPluginDLL.h"

const int cMaxAuthStringLength = 256;

// Classes
class CAuthPluginDLL : public CPluginDLL
{
public:

	enum EAuthPluginCall
	{
		// Derived plug-in calls
		eAuthSetServerTypeString = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
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

	enum EINETServerType
	{
		eServerFirst = 0,
		eServerUnknown = eServerFirst,
		eServerIMAP,
		eServerPOP3,
		eServerSMTP,
		eServerNNTP,
		eServerIMSP,
		eServerACAP,
		eServerLDAP,
		eServerWHOISPP,
		eServerFinger,
		eLocalAdbk,
		eServerManageSIEVE,
		eServerLast = eServerManageSIEVE
	};

	enum EAuthPluginUIType
	{
		eAuthUserPswd,
		eAuthKerberos,
		eAuthAnonymous
	};

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

	// Actual plug-in class

	CAuthPluginDLL();
	virtual ~CAuthPluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	virtual void	SetServerTypeString(const char* type);	// Server type string from Mulberry
	virtual void	UseUserID(bool use_it);					// Use user id from Mulberry
	virtual void	SetUserID(const char* str);				// Set UserID from Mulberry
	virtual const char*	GetUserID();						// Get UserID from plugin (in cases where uid is set externally e.g.Kerberos)
	virtual void	UsePassword(bool use_it);				// Use password from Mulberry
	virtual void	SetPassword(const char* str);			// Set password from Mulberry
	virtual void	SetServer(const char* str);				// Set server from Mulberry (this is user supplied value, maybe empty => use real name)
	virtual void	SetRealServer(const char* str);			// Set real server from Mulberry (this is the canonical/reverse lookup name)

	virtual long	ProcessData(SAuthPluginData* info) = 0;	// Process data

protected:
	SAuthPluginInfo mAuthInfo;						// Information about plug-in
	EINETServerType mServerType;					// Type of server in use
	bool mUseUserID;								// Use Mulberry supplied user id (otherwise try default)
	char mUserID[cMaxAuthStringLength];				// User ID provided by Mulberry
	char mRecoveredUserID[cMaxAuthStringLength];	// User ID determined by plugin
	bool mUsePassword;								// Use Mulberry supplied password (otherwise try default)
	char mPassword[cMaxAuthStringLength];			// User password provided by Mulberry
	char mServer[cMaxAuthStringLength];				// User server provided by Mulberry (this is user specified value)
	char mRealServer[cMaxAuthStringLength];			// User real server provided by Mulberry (this is the actual server cname)

	// These should be returned by specific sub-class
	virtual long GetDataLength(void) const					// Returns data of plug-in
		{ return sizeof(SAuthPluginData); }
	virtual const void* GetData(void) const					// Returns data of plug-in
		{ return &mAuthInfo; }
};

#endif
