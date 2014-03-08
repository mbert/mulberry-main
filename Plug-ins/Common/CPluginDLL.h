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

// CPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 17-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the DLL being called by Mulberry.
//
// History:
// CD:	17-Nov-1997:	Created initial header and implementation.
// CD:	13-Dec-1997:	Turned into abstract base class
// CD:	14-Jan-1998:	Converted import/export into single type
// CD:	12-Feb-1998:	Added logging capability
//

#ifndef __PLUGIN_MULBERRY__
#define __PLUGIN_MULBERRY__

// Typedefs

#ifdef __cplusplus
extern "C" {
#endif
typedef long (*LoggingCallbackProcPtr)(const char* text);
#ifdef __cplusplus
}
#endif

// Macro to do error report with function, file & line details
#define REPORTERROR(xx_errno, xx_errtxt)	ErrorReport(xx_errno, xx_errtxt, __FUNCTION__, __FILE__, __LINE__)

// Classes
class CPluginDLL
{
public:

	// Enums used for various codes
	enum EPluginType
	{
		ePluginUnknown = 0,
		ePluginAddressIO,
		ePluginMailboxIO,
		ePluginNetworkAuthentication,
		ePluginSecurity,
		ePluginSpelling,
		ePluginPswdChange,
		ePluginMailMerge,
		ePluginConfig,
		ePluginSSL,
		ePluginLast
	};

	enum EPluginCall
	{
		ePluginConstruct = 1,					// Construct new plugin object
		ePluginDestroy,							// Destroy plugin object
		ePluginInitialise,						// First call to plug-in - returns refCon
		ePluginTerminate,						// Last call to plug-in
		ePluginCanRun,							// Indicates whether OS locale allows plugin to run
		ePluginStatus,							// Get status from plug-in
		ePluginGetInformation,					// Get plug-in info structure
		
		// Registration related
		ePluginUseRegistration,
		ePluginCanDemo,

		// Log related
		ePluginSetLoggingCallback,

		// Derived plug-in calls
		ePluginFirstSubCode = 1024				// First code availble for sub-class calls
	};

	// Plug-in information class
	struct SPluginInfo
	{
		const char* mName;						// Name of plug-in
		long  mVersion;							// Version number of plugin (as per Mac OS 'vers' rsrc)
		EPluginType mType;						// Plug-in type
		const char* mManufacturer;				// Manufacturer
		const char* mDescription;				// Textual description
		long mDataLength;						// Length of data
		const void* mData;						// Plug-in specific info
	};

	// Actual plug-in class

	CPluginDLL();
	virtual ~CPluginDLL();
	
	// Entry point
	virtual long Entry(long code, void* data, long refCon);		// DLL entry point and dispatch

	// Get information
	virtual const SPluginInfo& GetInfo(void)					// Returns the info for the plug-in
		{ return mInfo; }
	virtual long GetRefCon(void)								// Returns refCon of plug-in
		{ return mRefCon; }

	// Entry codes
	virtual void	Construct(void);							// Construct plugin
	virtual void	Destroy(void);								// Destroy plugin
	virtual void	Initialise(void);							// Initialise plug-in
	virtual void	Terminate(void);							// Terminate plug-in
	virtual bool	CanRun(void) = 0;							// Test whether plug-in can run
	virtual long	Status(void);								// Get plug-in status
	virtual void	GetInformation(const SPluginInfo** copy);	// Copy info

	// Registration
	virtual bool UseRegistration(unsigned long* key) = 0;		// Does plug-in need to be registered
	virtual bool CanDemo(void) = 0;								// Can plug-in run as demo

protected:
	LoggingCallbackProcPtr mLogging;						// Callback into application for logging
	SPluginInfo mInfo;										// Information about plug-in
	long mRefCon;											// Reference constant used by plugin: not used in app

	virtual void ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno);
	virtual void LogEntry(const char* text) const;			// Log some text

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const = 0;			// Returns the name of the plug-in
	virtual long GetVersion(void) const = 0;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const = 0;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const = 0;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const = 0;		// Returns description of plug-in
	virtual long GetDataLength(void) const = 0;				// Returns data of plug-in
	virtual const void* GetData(void) const = 0;			// Returns data of plug-in
};

#endif
