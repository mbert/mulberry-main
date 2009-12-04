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

// CKerberosPluginDLL.cp
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

#include "CCRAMMD5PluginDLL.h"
#include <stdlib.h>
#include <stdio.h>
#include "CPluginInfo.h"

#include "CStringUtils.h"
#include "kbase64.h"
#include "hmac-md5.h"
#if __dest_os == __linux_os
#include <netinet/in.h>
#endif
#define AUTHERROR(xx_msg) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "CRAM-MD5 Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
  	return eAuthServerError; \
  } while (0)

#pragma mark ____________________________consts

const char* cPluginName = "CRAM-MD5 Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginNetworkAuthentication;
const char* cPluginDescription = "CRAM-MD5 authentication plugin for Mulberry." COPYRIGHT;
const char* cPluginAuthenticateID = "CRAM-MD5";
const char* cPluginPrefsDescriptor = "CRAM-MD5";
CAuthPluginDLL::EAuthPluginUIType cPluginAuthUIType = CAuthPluginDLL::eAuthUserPswd;

#pragma mark ____________________________CCRAMMD5PluginDLL

// Constructor
CCRAMMD5PluginDLL::CCRAMMD5PluginDLL()
{
	mState = eFirstLine;
}

// Destructor
CCRAMMD5PluginDLL::~CCRAMMD5PluginDLL()
{
}

// Initialise plug-in
void CCRAMMD5PluginDLL::Initialise(void)
{
	// Do default
	CAuthPluginDLL::Initialise();
	
	// Assign authentication ID
	::strncpy(mAuthInfo.mAuthTypeID, cPluginAuthenticateID, 255);
	mAuthInfo.mAuthTypeID[255] = 0;
	::strncpy(mAuthInfo.mPrefsDescriptor, cPluginPrefsDescriptor, 255);
	mAuthInfo.mPrefsDescriptor[255] = 0;
	mAuthInfo.mAuthUIType = cPluginAuthUIType;
}

// Does plug-in need to be registered
bool CCRAMMD5PluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'CMD5');
	return false;
}

// Can plug-in run as demo
bool CCRAMMD5PluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CCRAMMD5PluginDLL::CanRun(void)
{
	// Check for KClient etc here
	return true;
}

// Process data
long CCRAMMD5PluginDLL::ProcessData(SAuthPluginData* info)
{
	switch(mState)
	{
	case eFirstLine:
		return ProcessFirst(info);
	case eFirstLineLiteral:
		return ProcessFirstData(info);
	case eTagLine:
		return ProcessTag(info);
	default:
		return eAuthError;
	}
}

// Returns the name of the plug-in
const char* CCRAMMD5PluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CCRAMMD5PluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CCRAMMD5PluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CCRAMMD5PluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CCRAMMD5PluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Process first line from server
long CCRAMMD5PluginDLL::ProcessFirst(SAuthPluginData* info)
{
	char* p = info->data;

	// Must have continuation/tag
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerACAP:
		if ((*p != '+') || (p[1]!=' '))
			AUTHERROR("expected '+ ' continuation");
		p += 2;
		break;
	case eServerSMTP:
		if (::strncmp(p, "334 ", 4) != 0)
			AUTHERROR("expected '334 ' continuation");
		p += 4;
		break;
	case eServerManageSIEVE:
		// No tags or continuation
		break;
	default:
		AUTHERROR("Unknown server type");
	}

	// Determine type of server
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		{
			// Decode base64 from server in place
	    int len = kbase64_from64(info->data, p);
	    info->data[len] = 0;
			
	    // Process CRAM-MD5
	    return ProcessFirstData(info);
		}
	case eServerACAP:
	case eServerManageSIEVE:
		// Look for literal or string
		if (*p == '\"')
		{
			// Get quoted string
			char* q = ::strgetquotestr(&p);
			::strcpy(info->data, q);
			
			// Remove quoted
			p = info->data;
			q = p;
			while(*p)
			{
				if (*p == '\\')
					p++;
				else
					*q++ == *p++;
			}
			*q = 0;
			
			// Process CRAM-MD5
			return ProcessFirstData(info);
		}
		else if (*p == '{')
		{
			// Get more data from server and process directly
			mState = eFirstLineLiteral;
			return eAuthMoreData;
		}
		else
			AUTHERROR("illegal data return by server: not a string");
		break;
	default:
		AUTHERROR("cant use CRAM-MD5 with this server type");
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process first line data from server
long CCRAMMD5PluginDLL::ProcessFirstData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    int len = kbase64_from64(info->data, p);
	    info->data[len] = 0;
	}

	char* p = info->data;
	unsigned char digest[HMAC_MD5_SIZE];

	// Now do hmac-md5
	hmac_md5((unsigned char*) p, ::strlen(p), (unsigned char*) mPassword, ::strlen(mPassword), digest);

	// Convert to hex string
	char digest_hex[2*HMAC_MD5_SIZE + 1];
	for(int i = 0; i < HMAC_MD5_SIZE; i++)
	{
		unsigned char lo_q = digest[i];
		unsigned char hi_q = (lo_q >> 4);
		lo_q = lo_q & 0xF;
		digest_hex[2*i] = hi_q + ((hi_q >= 0xA) ? ('a' - 0x0A) : '0');
		digest_hex[2*i + 1] = lo_q + ((lo_q >= 0xA) ? ('a' - 0x0A) : '0');
	}
	digest_hex[2*HMAC_MD5_SIZE] = 0;

	*p = 0;
	
	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		::strcat(p, "\"");
		break;
	default:;
	}

	// First copy user name to buffer then cat hex value
	::strcat(p, mUserID);
	::strcat(p, " ");
	::strcat(p, digest_hex);

	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		::strcat(p, "\"");
		break;
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		{
			// Do base64 encoding in place
			char* q = p + ::strlen(p) + 1;
			kbase64_to64((unsigned char*) q, (unsigned char*) p, ::strlen(p));
			::memmove(p, q, ::strlen(q) + 1);
			break;
		}
	case eServerManageSIEVE:
		// Do base64 encoding and put in quoted string
		{
			char* q = p + ::strlen(p) + 3;
			kbase64_to64((unsigned char*) q, (unsigned char*) p, ::strlen(p));
			p[0] = '\"';
			::memmove(p + 1, q, ::strlen(q) + 1);
			::strcat(p, "\"");
			break;
		}
	default:;
	}

	// Return code to tell Mulberry to send this data and get a response from the server
	mState = eTagLine;
	return eAuthSendDataToServer;
}

// Process tag line from server
long CCRAMMD5PluginDLL::ProcessTag(SAuthPluginData* info)
{
	const char* p = info->data;
	
	// Now look at full response
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerIMSP:
	case eServerACAP:
		// Must have tag - not continuation or untagged
		if ((*p == '*') || (*p == '+'))
		{
			mState = eError;
			return eAuthError;
		}

		// Punt over tag
		while(*p && (*p != ' ')) p++;
		while(*p == ' ') p++;
		
		// Look for response
		if (::strncmpnocase(p, "OK", 2) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			mState = eError;
			return eAuthServerError;
		}
		break;
	case eServerPOP3:
		if (::strncmpnocase(p, "+OK", 3) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			mState = eError;
			return eAuthServerError;
		}
		break;
	case eServerSMTP:
		if (::strncmp(p, "235", 3) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			mState = eError;
			return eAuthServerError;
		}
		break;
	case eServerManageSIEVE:
		if (::strncmpnocase(p, "OK", 2) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			mState = eError;
			return eAuthServerError;
		}
	default:;
	}

	mState = eError;
	return eAuthServerError;
}
