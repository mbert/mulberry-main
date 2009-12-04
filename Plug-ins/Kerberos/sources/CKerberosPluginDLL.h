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

// CKerberosPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements Kerberos authentication DLL based plug-in for use in Mulberry.
//
// History:
// 13-Dec-1997: Created initial header and implementation.
//

#ifndef __KERBEROS_PLUG_MULBERRY__
#define __KERBEROS_PLUG_MULBERRY__

#include "CAuthPluginDLL.h"
#if __dest_os == __win32_os || __dest_os == __mac_os && !TARGET_API_MAC_CARBON
#define USE_KCLIENT	1
#endif

#ifdef USE_KCLIENT
#include "KClientPublic.h"
#else
#if __dest_os == __linux_os || __dest_os == __mac_os && TARGET_API_MAC_CARBON
#include <krb.h>
#else
#include <Kerberos/krb.h>
#endif
#endif

// Classes
class CKerberosPluginDLL : public CAuthPluginDLL
{
public:

	// Actual plug-in class

	CKerberosPluginDLL();
	virtual ~CKerberosPluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
	virtual long	ProcessData(SAuthPluginData* info);	// Process data

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	enum EKerberosPluginState
	{
		eError= 0,
		eFirstLine,
		eFirstLineLiteral,
		eFirstLineLiteralSend,
		eSecondLine,
		eSecondLineLiteral,
		eSecondLineLiteralSend,
		eTagLine,
		eDone
	};

	EKerberosPluginState mState;
	long mLiteralLength;
	char mLiteralBuffer[500];

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in
	
	// Process data
	long ProcessFirst(SAuthPluginData* info);				// Process first line from server
	long ProcessFirstData(SAuthPluginData* info);			// Process first line data from server
	long ProcessFirstLiteralSend(SAuthPluginData* info);	// Process send of literal to server
	long ProcessSecond(SAuthPluginData* info);				// Process second line from server
	long ProcessSecondData(SAuthPluginData* info);			// Process second line data from server
	long ProcessSecondLiteralSend(SAuthPluginData* info);	// Process send of literal to server
	long ProcessTag(SAuthPluginData* info);					// Porcess tag line from server

#ifdef USE_KCLIENT
	KClientSessionInfo kses;
#else
	des_cblock mSession; /* Our session key */
	des_key_schedule mSchedule; /* session key schedule*/
#endif
   long ran_num;

};

#endif
