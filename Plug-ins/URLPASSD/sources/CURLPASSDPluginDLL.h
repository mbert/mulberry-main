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

// CURLPASSDPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 10-Feb-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements URLPASSD password change DLL based plug-in for use in Mulberry.
//
// History:
// 10-Feb-1999: Created initial header and implementation.
//

#ifndef __URLPASSD_PLUGIN_MULBERRY__
#define __URLPASSD_PLUGIN_MULBERRY__

#include "CPswdChangePluginDLL.h"

// Classes
class CURLPASSDPluginDLL : public CPswdChangePluginDLL
{
public:

	// Actual plug-in class

	CURLPASSDPluginDLL();
	virtual ~CURLPASSDPluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
	virtual long	DoIt();								// Get plugin to do it
	virtual long	ProcessData(SPswdChangePluginData* info);	// Process data

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	enum EURLPASSDPluginState
	{
		eError= 0,
		eUser,
		ePass,
		eNewPass,
		eQuit,
		eDone
	};

	EURLPASSDPluginState mState;

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in

	bool LaunchURL(const char* url);
};

#endif
