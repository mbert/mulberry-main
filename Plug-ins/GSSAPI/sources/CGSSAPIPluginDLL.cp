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

// CGSSAPIPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 26-Dec-1999
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements GSSAPI authentication DLL based plug-in for use in Mulberry.
//
// History:
// 26-Dec-1999: Created initial header and implementation.
//

#include "CGSSAPIPluginDLL.h"
#include "CPluginInfo.h"
#include <stdlib.h>
#include <stdio.h>

#include "CStringUtils.h"
#include "kbase64.h"

#if __dest_os == __mac_os_x
#undef EFBIG
#include <Kerberos/gssapi_krb5.h>
#else
#include <gssapi_krb5.h>
#endif

#if __dest_os == __win32_os
#include "CDLLFunctions.h"
extern CDLLLoader* sGSSLoader;
CDLLLoader* sGSSLoader = NULL;
#elif __dest_os == __mac_os && TARGET_API_MAC_CARBON
#include "CMachOFunctions.h"
extern CMachOLoader* sGSSLoader;
CMachOLoader* sGSSLoader = NULL;
#endif

#if __dest_os == __mac_os && TARGET_API_MAC_CARBON || __dest_os == __mac_os_x
#define KERBEROSLOGIN_DEPRECATED
#include <Kerberos/KerberosLogin.h>
#endif

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#define AUTHERROR(xx_msg, copyout) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::snprintf(err_buf, 256, "GSSAPI Plugin Error: %s, %s, %d", xx_msg, __FILE__, __LINE__); \
    LogEntry(err_buf); \
    if (copyout) ::strcpy(info->data, xx_msg); \
  	return eAuthServerError; \
  } while (0)

#define SASL_PROT_NONE 1
#define SASL_PROT_INTEGRITY 2
#define SASL_PROT_PRIVACY 4
#define SASL_PROT_ANY (SASL_PROT_NONE|SASL_PROT_INTEGRITY|SASL_PROT_PRIVACY)

// Missing bits from MIT distribution

#if __dest_os == __mac_os
const gss_OID_desc krb5_gss_oid_array[] = {
   /* this is the official, rfc-specified OID */
   {9, (void*) "\052\206\110\206\367\022\001\002\002"},
   /* this is the unofficial, wrong OID */
   {5, (void*) "\053\005\001\005\002"},
   /* this is the v2 assigned OID */
   {9, (void*) "\052\206\110\206\367\022\001\002\003"},
   /* these two are name type OID's */
   {10, (void*) "\052\206\110\206\367\022\001\002\002\001"},
   {10, (void*) "\052\206\110\206\367\022\001\002\002\002"},
   { 0, 0 }
};

const gss_OID_desc * const gss_mech_krb5 = krb5_gss_oid_array+0;
const gss_OID_desc * const gss_mech_krb5_old = krb5_gss_oid_array+1;
const gss_OID_desc * const gss_mech_kgrerb5_v2 = krb5_gss_oid_array+2;
const gss_OID_desc * const gss_nt_krb5_name = krb5_gss_oid_array+3;
const gss_OID_desc * const gss_nt_krb5_principal = krb5_gss_oid_array+4;
#endif

static gss_OID_desc moids[] = {
   {10, (void*) "\052\206\110\206\367\022\001\002\001\001"},
   {10, (void*) "\052\206\110\206\367\022\001\002\001\002"},
   {10, (void*) "\052\206\110\206\367\022\001\002\001\003"},
   {10, (void*) "\052\206\110\206\367\022\001\002\001\004"},
   { 6, (void*) "\053\006\001\005\006\004"},
   { 6, (void*) "\053\006\001\005\006\002"},
};

gss_OID_desc * gss_nt_user_name = moids+0;
gss_OID_desc * gss_nt_machine_uid_name = moids+1;
gss_OID_desc * gss_nt_string_uid_name = moids+2;
gss_OID_desc * gss_nt_service_name = moids+3;
gss_OID_desc * gss_nt_exported_name = moids+4;
gss_OID_desc * gss_nt_service_name_v2 = moids+5;

#pragma mark ____________________________consts

const char* cPluginName = "GSSAPI Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginNetworkAuthentication;
const char* cPluginDescription = "GSSAPI authentication plugin for Mulberry." COPYRIGHT;
const char* cPluginAuthenticateID = "GSSAPI";
const char* cPluginPrefsDescriptor = "GSSAPI";
CAuthPluginDLL::EAuthPluginUIType cPluginAuthUIType = CAuthPluginDLL::eAuthKerberos;

const char* cTokenSeparators = "()<>@,;:\\\"/[]?={} \t";

#pragma mark ____________________________CGSSAPIPluginDLL

// Constructor
CGSSAPIPluginDLL::CGSSAPIPluginDLL()
{
	mState = eStep;
	mInitContext = true;
	mGSSAPI_Context = GSS_C_NO_CONTEXT;
	mGSSAPI_ServerName = GSS_C_NO_NAME;
	
#if __dest_os == __win32_os
	if (sGSSLoader == NULL)
		sGSSLoader = new CDLLLoader("gssapi32.dll");
#elif __dest_os == __mac_os && TARGET_API_MAC_CARBON
	if (sGSSLoader == NULL)
		sGSSLoader = new CMachOLoader("Kerberos.Framework");
#endif
}

// Destructor
CGSSAPIPluginDLL::~CGSSAPIPluginDLL()
{
	// Clean up
	CleanContext();

#if __dest_os == __win32_os || __dest_os == __mac_os && TARGET_API_MAC_CARBON
	// Finished with bundle
	if (sGSSLoader)
		delete sGSSLoader;
	sGSSLoader = NULL;
#endif
}

// Initialise plug-in
void CGSSAPIPluginDLL::Initialise()
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
bool CGSSAPIPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'GAPI');
	return false;
}

// Can plug-in run as demo
bool CGSSAPIPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CGSSAPIPluginDLL::CanRun(void)
{
	// Check for KClient etc here
#if __dest_os == __win32_os || __dest_os == __mac_os && TARGET_API_MAC_CARBON
	return sGSSLoader ? sGSSLoader->IsLoaded() : false;
#else
	return true;
#endif
}

// Process data
long CGSSAPIPluginDLL::ProcessData(SAuthPluginData* info)
{
	switch(mState)
	{
	case eStep:
	case eNegStep:
		return ProcessStep(info);
	case eStepLiteral:
		return ProcessStepData(info);
	case eNegStepLiteral:
		return ProcessNegStepData(info);
	case eStepLiteralSend:
	case eNegStepLiteralSend:
	case eTagStepLiteralSend:
		return ProcessStepLiteralSend(info);
	case eTagLine:
		return ProcessTag(info);
	default:
		return eAuthError;
	}
}

// Returns the name of the plug-in
const char* CGSSAPIPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CGSSAPIPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CGSSAPIPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CGSSAPIPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CGSSAPIPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Process first line from server
long CGSSAPIPluginDLL::InitContext(SAuthPluginData* info)
{
	// Determine actual server principal
	char actual_principal[cMaxAuthStringLength];
	if (*mServer && ::strchr(mServer, '@'))
		// Principal is that specified by user
		::strcpy(actual_principal, mServer);
	else
	{
		*actual_principal = 0;

		// Prepend service name
		switch(mServerType)
		{
		case eServerIMAP:
			::strcat(actual_principal, "imap@");
			break;
		case eServerPOP3:
			::strcat(actual_principal, "pop@");
			break;
		case eServerIMSP:
			::strcat(actual_principal, "imap@");
			break;
		case eServerSMTP:
			::strcat(actual_principal, "smtp@");
			break;
		case eServerACAP:
			::strcat(actual_principal, "acap@");
			break;
		case eServerManageSIEVE:
			::strcat(actual_principal, "sieve@");
			break;
		default:;
		}
		
		// Add IP address
		size_t start_caps = ::strlen(actual_principal);
		if (*mServer)
			::strcat(actual_principal, mServer);
		else
			::strcat(actual_principal, mRealServer);
	}

	// Create descriptor
	gss_buffer_desc name_token;
    name_token.length = ::strlen(actual_principal);
    name_token.value = actual_principal;

	OM_uint32 maj_stat;
	OM_uint32 min_stat;
	maj_stat = ::gss_import_name(&min_stat, (gss_buffer_desc*) &name_token, gss_krb5_nt_service_name, &mGSSAPI_ServerName);
	
	if (maj_stat != GSS_S_COMPLETE)
		DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
	
#if __dest_os == __mac_os
	KLStatus kerr = ::KLAcquireTickets(NULL, NULL, NULL);
#elif __dest_os == __mac_os_x
	KLStatus kerr = ::KLAcquireInitialTickets(NULL, 0, NULL, NULL);
#endif

	return (maj_stat == GSS_S_COMPLETE);
}

// Destructor
void CGSSAPIPluginDLL::CleanContext()
{
	// Clean up
	OM_uint32 maj_stat;
	OM_uint32 min_stat;

	if (mGSSAPI_Context != GSS_C_NO_CONTEXT)
		maj_stat = ::gss_delete_sec_context(&min_stat, &mGSSAPI_Context, GSS_C_NO_BUFFER);
	mGSSAPI_Context = GSS_C_NO_CONTEXT;

	if (mGSSAPI_ServerName != GSS_C_NO_NAME)
		maj_stat = ::gss_release_name(&min_stat, &mGSSAPI_ServerName);
	mGSSAPI_ServerName = GSS_C_NO_NAME;
}

void CGSSAPIPluginDLL::DisplayError(SAuthPluginData* info, OM_uint32 maj_status, OM_uint32 min_status, const char* file, int line)
{
	OM_uint32 maj_stat;
	OM_uint32 min_stat;
	OM_uint32 mctx = 0;
	gss_buffer_desc string_token;
	maj_stat = ::gss_display_status(&min_stat, min_status, GSS_C_MECH_CODE,
										GSS_C_NULL_OID, &mctx, &string_token);

	const char* err_title = "GSSAPI Plugin Error: ";
	int err_len = ::strlen(err_title);
	if (string_token.value)
		err_len += ::strlen((char*) string_token.value);
	err_len++;
	char* err_txt = new char[err_len + 256];
	if (string_token.value)
		::snprintf(err_txt, err_len + 256, "GSSAPI Plugin Error: %s, %s %d", (char*) string_token.value, file, line);
	else
		::snprintf(err_txt, err_len + 256, "GSSAPI Plugin Error: %s %d", file, line);
	LogEntry(err_txt);
	delete err_txt;
	if (string_token.value)
		::strcpy(info->data, (char*) string_token.value);
	else
		::strcpy(info->data, err_title);

	maj_stat = ::gss_release_buffer(&min_stat, &string_token);

  	mState = eError;
}

// Process first line from server
long CGSSAPIPluginDLL::ProcessStep(SAuthPluginData* info)
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
			AUTHERROR("expected '+ ' continuation", false);
		p += 2;
		break;
	case eServerSMTP:
		if (::strncmp(p, "334 ", 4) != 0)
			AUTHERROR("expected '334 ' continuation", false);
		p += 4;
		break;
	case eServerManageSIEVE:
		// No tags or continuation
		break;
	default:
		AUTHERROR("Unsupported Server Type", false);
	}

	// Determine type of server
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		// Decode base64 from server in place
	    mLiteralLength = kbase64_from64(info->data, p);
	    if (mLiteralLength < 0)
	    	mLiteralLength = 0;
	    p[mLiteralLength] = 0;

	    // Process data
	    return (mState == eStep) ? ProcessStepData(info) : ProcessNegStepData(info);

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
			
			// set length
			mLiteralLength = ::strlen(info->data);

			// Process data
			return (mState == eStep) ? ProcessStepData(info) : ProcessNegStepData(info);
		}
		else if (*p == '{')
		{
			mLiteralLength = ::atoi(p+1);

			// Copy data length to server buffer
			char* p = info->data;
			for(int i = 0; i < info->length; i++)
				*p++ = 0;
			::memcpy(info->data, &mLiteralLength, 4);

			// Get more data from server and process directly
			mState = (mState == eStep) ? eStepLiteral : eNegStepLiteral;
			return eAuthMoreLiteralData;
		}
		else
			AUTHERROR("illegal data return by server: not a string", true);
		break;

	default:
		AUTHERROR("can't use GSSAPI with this server type", true);
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process first line data from server
long CGSSAPIPluginDLL::ProcessStepData(SAuthPluginData* info)
{
	// First make sure context is init'd
	if (mInitContext)
	{
		if (!InitContext(info))
			return eAuthServerError;
		mInitContext = false;
	}

	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    mLiteralLength = kbase64_from64(info->data, p);
	    if (mLiteralLength < 0)
	    	mLiteralLength = 0;
	    p[mLiteralLength] = 0;
	}

	// Prepare input/output buffers
	gss_buffer_desc input_token;
	gss_buffer_desc output_token;

	input_token.length = 0;
	input_token.value = NULL;
	output_token.length = 0;
	output_token.value = NULL;

	if (mLiteralLength)
	{
		input_token.length = mLiteralLength;
		input_token.value = info->data;
	}

	// Do GSSAPI step
	OM_uint32 maj_stat;
	OM_uint32 min_stat;
	maj_stat = ::gss_init_sec_context(&min_stat,
										GSS_C_NO_CREDENTIAL,
										&mGSSAPI_Context,
										mGSSAPI_ServerName,
										GSS_C_NO_OID,
										GSS_C_MUTUAL_FLAG | GSS_C_SEQUENCE_FLAG,
										0,
										GSS_C_NO_CHANNEL_BINDINGS,
										&input_token,
										NULL,
										&output_token,
										NULL,
										NULL);

	switch(maj_stat)
	{
	case GSS_S_COMPLETE:
		// Auth step done - do qop negotiation
		mState = eNegStep;
		break;
	case GSS_S_CONTINUE_NEEDED:
		// More auth steps required
		mState = eStep;
		break;
	default:
		if (output_token.value)
		    ::gss_release_buffer(&min_stat, &output_token);

		// Error
		DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
		return eAuthServerError;
	}

	// Now form response string
	char* p = info->data;
	*p = 0;

	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		mLiteralLength = output_token.length;
		::sprintf(p, "{%ld}", mLiteralLength);
		::memcpy(mLiteralBuffer, &mLiteralLength, 4);
		::memcpy(mLiteralBuffer + 4, output_token.value, mLiteralLength);

		// Return code to tell Mulberry to send a literal
		mState = (mState == eStep) ? eStepLiteralSend : eNegStepLiteralSend;
		if (output_token.value)
		    ::gss_release_buffer(&min_stat, &output_token);
		return eAuthSendDataToServer;

	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		// Do base64 encoding
	  	kbase64_to64((unsigned char*) p, (unsigned char*) output_token.value, output_token.length);
		break;

	case eServerManageSIEVE:
		// Do base64 encoding and put in quoted string
		p[0] = '\"';
	  	kbase64_to64((unsigned char*) p+1, (unsigned char*) output_token.value, output_token.length);
	  	::strcat(p, "\"");
		break;
	default:;
	}

	// Return code to tell Mulberry to send this data and get a response from the server
	if (output_token.value)
	    ::gss_release_buffer(&min_stat, &output_token);
	return eAuthSendDataToServer;
}

// Process first line data from server
long CGSSAPIPluginDLL::ProcessNegStepData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    mLiteralLength = kbase64_from64(info->data, p);
	    if (mLiteralLength < 0)
	    	mLiteralLength = 0;
	    p[mLiteralLength] = 0;
	}

	// Prepare input/output buffers
	gss_buffer_desc input_token;
	gss_buffer_desc output_token;

	input_token.length = 0;
	input_token.value = NULL;
	output_token.length = 0;
	output_token.value = NULL;

	if (mLiteralLength)
	{
		input_token.length = mLiteralLength;
		input_token.value = info->data;
	}

	// Decrypt it
	OM_uint32 maj_stat;
	OM_uint32 min_stat;
	maj_stat = ::gss_unwrap(&min_stat,
							mGSSAPI_Context,
							&input_token,
							&output_token,
							NULL,
							NULL);

	switch(maj_stat)
	{
	case GSS_S_COMPLETE:
		// Qop step done
		mState = eTagLine;
		break;
	default:
		if (output_token.value)
		    ::gss_release_buffer(&min_stat, &output_token);

		// Error
		DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
		return eAuthServerError;
	}

	// output_token:
	// byte 1 : bitmask for server supported modes
	// bytes 2-4 : max buffer size for encryption

	// Set mode to none
	char buf[500];
	::memcpy(buf, output_token.value, 4);
	buf[0] = SASL_PROT_NONE;

	if (output_token.value)
	    ::gss_release_buffer(&min_stat, &output_token);
	output_token.length = 0;
	output_token.value = NULL;

	if(mUseUserID)
		::strcpy(buf + 4, mUserID);
	else
	{
		gss_name_t user = GSS_C_NO_NAME;
	    maj_stat = ::gss_inquire_context(&min_stat, mGSSAPI_Context, &user, NULL, NULL, NULL,  NULL, NULL, NULL);
		if (maj_stat != GSS_S_COMPLETE)
		{
			// Error
			DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
			return eAuthServerError;
		}

		gss_buffer_desc name_token;
	    name_token.length = 0;
		maj_stat = ::gss_display_name(&min_stat, user, &name_token, NULL);
		if (maj_stat != GSS_S_COMPLETE)
		{
			if (name_token.value)
			    ::gss_release_buffer(&min_stat, &name_token);
			::gss_release_name(&min_stat, &user);

			// Error
			DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
			return eAuthServerError;
		}
		else
		{
			if (::strchr((char*) name_token.value, '@'))
				*::strchr((char*) name_token.value, '@') = 0;
			::strcpy(buf + 4, (char*) name_token.value);
		    ::gss_release_buffer(&min_stat, &name_token);
			::gss_release_name(&min_stat, &user);
		}
	}
	// Save current id as recovered user id
	::strcpy(mRecoveredUserID, buf+4);

	input_token.length = 4 + ::strlen(buf + 4);
	input_token.value = buf;

	// Now encrypt it
	maj_stat = ::gss_wrap(&min_stat,
							mGSSAPI_Context,
							0,
							GSS_C_QOP_DEFAULT,
							&input_token,
							NULL,
							&output_token);

	switch(maj_stat)
	{
	case GSS_S_COMPLETE:
		// Qop step done
		mState = eTagLine;
		break;
	default:
		if (output_token.value)
		    ::gss_release_buffer(&min_stat, &output_token);

		// Error
		DisplayError(info, maj_stat, min_stat, __FILE__, __LINE__);
		return eAuthServerError;
	}

	// Now form response string
	char* p = info->data;
	*p = 0;

	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		mLiteralLength = output_token.length;
		::sprintf(p, "{%ld}", mLiteralLength);
		::memcpy(mLiteralBuffer, &mLiteralLength, 4);
		::memcpy(mLiteralBuffer + 4, output_token.value, mLiteralLength);

		// Return code to tell Mulberry to send a literal
		mState = eTagStepLiteralSend;

		if (output_token.value)
		    ::gss_release_buffer(&min_stat, &output_token);

		return eAuthSendDataToServer;

	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		// Do base64 encoding
	  	kbase64_to64((unsigned char*) p, (unsigned char*) output_token.value, output_token.length);
		break;

	case eServerManageSIEVE:
		// Do base64 encoding and put in quoted string
		p[0] = '\"';
	  	kbase64_to64((unsigned char*) p+1, (unsigned char*) output_token.value, output_token.length);
	  	::strcat(p, "\"");
		break;
	default:;
	}

	if (output_token.value)
	    ::gss_release_buffer(&min_stat, &output_token);

	// Return code to tell Mulberry to send this data and get a response from the server
	return eAuthSendDataToServer;
}

// Process literal send to server
long CGSSAPIPluginDLL::ProcessStepLiteralSend(SAuthPluginData* info)
{
	char* p = info->data;
	// Must have continuation
	if (*p != '+')
		AUTHERROR("expected '+ ' continuation", false);

	// Copy data to server buffer
	::memcpy(p, mLiteralBuffer, mLiteralLength + 4);

	// Send the literal
	switch(mState)
	{
	case eStepLiteralSend:
		mState = eStep;
		break;
	case eNegStepLiteralSend:
		mState = eNegStep;
		break;
	case eTagStepLiteralSend:
	default:
		mState = eTagLine;
		break;
	}
	return eAuthSendLiteralDataToServer;
}

// Process tag line from server
long CGSSAPIPluginDLL::ProcessTag(SAuthPluginData* info)
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
			AUTHERROR(p, true);
		}

	case eServerPOP3:
		if (::strncmpnocase(p, "+OK", 3) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			AUTHERROR(p, true);
		}

	case eServerSMTP:
		if (::strncmp(p, "235", 3) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			AUTHERROR(p, true);
		}

	case eServerManageSIEVE:
		if (::strncmpnocase(p, "OK", 2) == 0)
		{
			mState = eDone;
			return eAuthDone;
		}
		else
		{
			AUTHERROR(p, true);
		}
	default:;
	}

	mState = eError;
	return eAuthServerError;
}
