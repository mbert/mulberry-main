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

// CGSSAPIPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 07-Nov-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements GSSAPI authentication DLL based plug-in for use in Mulberry.
//
// History:
// 26-Dec-1999: Created initial header and implementation.
//

#ifndef __GSSAPI_PLUG_MULBERRY__
#define __GSSAPI_PLUG_MULBERRY__

#include "CAuthPluginDLL.h"

#include <time.h>
#if __dest_os == __mac_os_x
#include <Kerberos/gssapi.h>
#define KERBEROSLOGIN_DEPRECATED
#include <Kerberos/KerberosLogin.h>
#else
#include <gssapi.h>
#endif

// Classes
class CGSSAPIPluginDLL : public CAuthPluginDLL
{
public:

	// Actual plug-in class

	CGSSAPIPluginDLL();
	virtual ~CGSSAPIPluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
	virtual long	ProcessData(SAuthPluginData* info);	// Process data

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	enum EGSSAPIPluginState
	{
		eError= 0,
		eStep,
		eStepLiteral,
		eStepLiteralSend,
		eNegStep,
		eNegStepLiteral,
		eNegStepLiteralSend,
		eTagLine,
		eTagStepLiteralSend,
		eDone
	};

	EGSSAPIPluginState mState;
	bool mInitContext;
	char hex_response_server[33];
	long 			mLiteralLength;
	char 			mLiteralBuffer[500];
	gss_ctx_id_t	mGSSAPI_Context;
	gss_name_t		mGSSAPI_ServerName;

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in
	
	// Process data
	long ProcessStep(SAuthPluginData* info);			// Process step from server
	long ProcessStepData(SAuthPluginData* info);		// Process step data from server
	long ProcessNegStepData(SAuthPluginData* info);		// Process negotiation step data from server
	long ProcessStepLiteralSend(SAuthPluginData* info);	// Process send of literal to server
	long ProcessTag(SAuthPluginData* info);				// Porcess tag line from server
	
	// Utility fns
	long InitContext(SAuthPluginData* info);			// Init GSSAPI context
	void CleanContext();								// Cleanup GSSAPI context
	void DisplayError(SAuthPluginData* info,			// Report text error
						OM_uint32 maj_status,
						OM_uint32 min_status,
						const char* file,
						int line);
#if __dest_os == __mac_os_x
    void DisplayKLError(KLStatus inErr, const char* file, int line);  // Report Kerberos error
#endif
};

#endif
