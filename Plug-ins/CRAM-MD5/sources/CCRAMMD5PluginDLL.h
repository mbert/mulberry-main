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

// CCRAMMD5PluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 12-Feb-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements CRAM-MD5 authentication DLL based plug-in for use in Mulberry.
//
// History:
// 12-Feb-1998: Created initial header and implementation.
//

#ifndef __CRAMMD5_PLUGIN_MULBERRY__
#define __CRAMMD5_PLUGIN_MULBERRY__

#include "CAuthPluginDLL.h"

// Classes
class CCRAMMD5PluginDLL : public CAuthPluginDLL
{
public:

	// Actual plug-in class

	CCRAMMD5PluginDLL();
	virtual ~CCRAMMD5PluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
	virtual long	ProcessData(SAuthPluginData* info);	// Process data

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	enum ECRAMMD5PluginState
	{
		eError= 0,
		eFirstLine,
		eFirstLineLiteral,
		eTagLine,
		eDone
	};

	ECRAMMD5PluginState mState;
	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in
	
	// Process data
	long ProcessFirst(SAuthPluginData* info);			// Process first line from server
	long ProcessFirstData(SAuthPluginData* info);		// Process first line data from server
	long ProcessTag(SAuthPluginData* info);				// Porcess tag line from server
	
};

#endif
