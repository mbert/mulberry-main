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

// CDIGESTMD5PluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 30-Dec-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements DIGEST-MD5 authentication DLL based plug-in for use in Mulberry.
//
// History:
// 30-Dec-1998: Created initial header and implementation.
//

#ifndef __DIGESTMD5_PLUGIN_MULBERRY__
#define __DIGESTMD5_PLUGIN_MULBERRY__

#include "CAuthPluginDLL.h"

// Classes
class CDIGESTMD5PluginDLL : public CAuthPluginDLL
{
public:

	// Actual plug-in class

	CDIGESTMD5PluginDLL();
	virtual ~CDIGESTMD5PluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
	virtual long	ProcessData(SAuthPluginData* info);	// Process data

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	enum EDIGESTMD5PluginState
	{
		eError= 0,
		eFirstLine,
		eFirstLineLiteral,
		eSecondLine,
		eSecondLineLiteral,
		eTagLine,
		eDone
	};

	EDIGESTMD5PluginState mState;
	char hex_response_server[33];

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in
	
	// Process data
	long ProcessFirst(SAuthPluginData* info);			// Process first line from server
	long ProcessFirstData(SAuthPluginData* info);		// Process first line data from server
	long ProcessSecond(SAuthPluginData* info);			// Process second line from server
	long ProcessSecondData(SAuthPluginData* info);		// Process second line data from server
	long ProcessTag(SAuthPluginData* info);				// Porcess tag line from server

private:
	void PuntLWS(char*& txt);
	
};

#endif
