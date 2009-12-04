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

// CSSLPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 28-May-2000
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a SSL security DLL based plug-in for use in Mulberry.
//
// History:
// 28-May-2000: Created initial header and implementation.
//

#ifndef __CSSLPLUGINDLL_MULBERRY__
#define __CSSLPLUGINDLL_MULBERRY__

#include "CPluginDLL.h"

// Classes
class CSSLPluginDLL : public CPluginDLL
{
public:

	// Actual plug-in class

	CSSLPluginDLL();
	virtual ~CSSLPluginDLL();
	
	// Entry codes
	virtual bool	CanRun();							// Test whether plug-in can run

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo();								// Can plug-in run as demo

protected:
	// These should be returned by specific sub-class
	virtual const char* GetName() const;				// Returns the name of the plug-in
	virtual long GetVersion() const;					// Returns the version number of the plug-in
	virtual EPluginType GetType() const;				// Returns the type of the plug-in
	virtual const char* GetManufacturer() const;		// Returns manufacturer of plug-in
	virtual const char* GetDescription() const;			// Returns description of plug-in
	virtual long GetDataLength() const					// Returns data of plug-in
		{ return 0; }
	virtual const void* GetData() const					// Returns data of plug-in
		{ return 0L; }
	
};

#endif
