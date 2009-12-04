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

// CSSLPluginDLL.cp
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

#include "CSSLPluginDLL.h"

#include "CPluginInfo.h"

#include <openssl/opensslv.h>

#pragma mark ____________________________consts

const char* cPluginName = "SSL Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSSL;
const char* cPluginDescription = "SSL Security plugin for Mulberry." OS_ENDL OS_ENDL
			"This product includes software developed by the OpenSSL Project "
			"for use in the OpenSSL Toolkit (http://www.openssl.org/): " OPENSSL_VERSION_TEXT COPYRIGHT;

#pragma mark ____________________________CSSLPluginDLL

// Constructor
CSSLPluginDLL::CSSLPluginDLL()
{
}

// Destructor
CSSLPluginDLL::~CSSLPluginDLL()
{
}

// Does plug-in need to be registered
bool CSSLPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CSSLPluginDLL::CanDemo()
{
	// Must be registered
	return true;
}

// Test for run ability
bool CSSLPluginDLL::CanRun()
{
	// Check for SSLsdk
	return true;
}

// Returns the name of the plug-in
const char* CSSLPluginDLL::GetName() const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CSSLPluginDLL::GetVersion() const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CSSLPluginDLL::GetType() const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CSSLPluginDLL::GetManufacturer() const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CSSLPluginDLL::GetDescription() const
{
	return cPluginDescription;
}
