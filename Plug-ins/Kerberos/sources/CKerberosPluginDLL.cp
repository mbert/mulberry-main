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
//#include <mlimits.h>

#include "CKerberosPluginDLL.h"
#include <stdlib.h>
#include <stdio.h>
#include "CPluginInfo.h"

#include "CStringUtils.h"
#include "kbase64.h"

#if __dest_os == __mac_os && TARGET_API_MAC_CARBON
#include <Kerberos/KerberosLogin.h>
#endif

#if __dest_os == __win32_os
#include "winsock.h"
#endif
#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#if __dest_os == __linux_os
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifndef USE_KCLIENT
#include <string>
#endif

#define AUTHERROR(xx_msg, copyout) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "Kerberos Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
    if (copyout) ::strcpy(info->data, xx_msg); \
  	return eAuthServerError; \
  } while (0)

#define AUTHERRORLOG(xx_msg, rc) do { \
	char err_buf[256]; \
	::sprintf(err_buf, "Kerberos Plugin Error: %s %d", xx_msg, rc); \
    LogEntry(err_buf); \
  } while (0)

#define SASL_PROT_NONE 1
#define SASL_PROT_INTEGRITY 2
#define SASL_PROT_PRIVACY 4
#define SASL_PROT_ANY (SASL_PROT_NONE|SASL_PROT_INTEGRITY|SASL_PROT_PRIVACY)

#define SASL_FAIL 1		/* Authentication failed */

#define SASL_DONE 3		/* Server has authenticated user */

#pragma mark ____________________________consts

const char* cPluginName = "Kerberos Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginNetworkAuthentication;
const char* cPluginDescription = "Kerberos V4 authentication plugin for Mulberry." OS_ENDL OS_ENDL
									"Includes cryptographic software written by Eric Young (eay@mincom.oz.au)."
									COPYRIGHT OS_ENDL
									"Thanks to Aaron Wohl, CMU for coding.";
const char* cPluginAuthenticateID = "KERBEROS_V4";
const char* cPluginPrefsDescriptor = "Kerberos V4";
CAuthPluginDLL::EAuthPluginUIType cPluginAuthUIType = CAuthPluginDLL::eAuthKerberos;

#pragma mark ____________________________CKerberosPluginDLL


#ifndef USE_KCLIENT
// Class to zero out allocated memory when destroyed
template <class T> class CKerberosPluginBurnData
{
public:
	CKerberosPluginBurnData(T* p) : p_(p) {}
	~CKerberosPluginBurnData()
		{ ::memset(p_, 0, sizeof(T)); }
private:
	T* p_;
};

#endif

// Constructor
CKerberosPluginDLL::CKerberosPluginDLL()
{
	mState = eFirstLine;
}

// Destructor
CKerberosPluginDLL::~CKerberosPluginDLL()
{
}

// Initialise plug-in
void CKerberosPluginDLL::Initialise(void)
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
bool CKerberosPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'Krb4');
	return false;
}

// Can plug-in run as demo
bool CKerberosPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CKerberosPluginDLL::CanRun(void)
{
	// Check for KClient etc here
	return true;
}

// Process data
long CKerberosPluginDLL::ProcessData(SAuthPluginData* info)
{
	switch(mState)
	{
	case eFirstLine:
		return ProcessFirst(info);
	case eFirstLineLiteral:
		return ProcessFirstData(info);
	case eFirstLineLiteralSend:
		return ProcessFirstLiteralSend(info);
	case eSecondLine:
		return ProcessSecond(info);
	case eSecondLineLiteral:
		return ProcessSecondData(info);
	case eSecondLineLiteralSend:
		return ProcessSecondLiteralSend(info);
	case eTagLine:
		return ProcessTag(info);
	default:
		return eAuthError;
	}
}

// Returns the name of the plug-in
const char* CKerberosPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CKerberosPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CKerberosPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CKerberosPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CKerberosPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Process first line from server
long CKerberosPluginDLL::ProcessFirst(SAuthPluginData* info)
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
	default:;
	}

	// Determine type of server
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
	{
	    if (::strlen(p) > 20)
	    	AUTHERROR("server first response is too long", true);

		// Decode base64 from server in place
	    int len = ::kbase64_from64(info->data, p);
	    p[len] = 0;
	    if (len != 4)
    		AUTHERROR("can't decode base64 of server first response", true);
			
		// set length
		mLiteralLength = 4;
			
	    // Process data
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
			
			// set length
			mLiteralLength = ::strlen(info->data);

			// Process data
			return ProcessFirstData(info);
		}
		else if (*p == '{')
		{
			mLiteralLength = ::atoi(p+1);

			// Get more data from server and process directly
			mState = eFirstLineLiteral;
			return eAuthMoreData;
		}
		else
			AUTHERROR("illegal data return by server: not a string", true);
		break;
	default:
		AUTHERROR("can't use Kerberos_V4 with this server type", true);
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process first line data from server
long CKerberosPluginDLL::ProcessFirstData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    mLiteralLength = ::kbase64_from64(info->data, p);
	    if (mLiteralLength < 0)
	    	mLiteralLength = 0;
	    p[mLiteralLength] = 0;
	}

	char* p = info->data;
	int rc;
#ifdef USE_KCLIENT
    rc = ::KClientNewSession(&kses,0,0,0,0);
    if (rc != 0)
    	AUTHERROR("create kclient session failed", true);
#else
	std::string service, realm, instance;
#endif

	// Determine actual server principal
#ifdef USE_KCLIENT
	char actual_principal[cMaxAuthStringLength];
	if (*mServer && (::strchr(mServer, '@')))
	{
		// Principal is that specified by user
		::strcpy(actual_principal, mServer);
	}
	else
	{
		*actual_principal = 0;

		// Prepend service name
		switch(mServerType)
		{
		case eServerIMAP:
			::strcat(actual_principal, "imap.");
			break;
		case eServerPOP3:
			::strcat(actual_principal, "pop.");
			break;
		case eServerIMSP:
			::strcat(actual_principal, "imap.");
			break;
		case eServerSMTP:
			::strcat(actual_principal, "smtp.");
			break;
		case eServerACAP:
			::strcat(actual_principal, "acap.");
			break;
		case eServerManageSIEVE:
			::strcat(actual_principal, "sieve.");
			break;
		default:;
		}
		
		// Add first part of IP address followed by '@'
		char* p = ::strchr(mRealServer, '.');
		if (p)
			::strncat(actual_principal, mRealServer, p - mRealServer);
		::strlower(actual_principal);
		::strcat(actual_principal, "@");
		
		// Add remainder of IP address and capitalise
		size_t start_caps = ::strlen(actual_principal);
		if (*mServer)
			::strcat(actual_principal, mServer);
		else
			::strcat(actual_principal, p ? ++p : mRealServer);
		::strupper(actual_principal + start_caps);
	}

#else
	char * at;
	if (*mServer && (at = ::strchr(mServer, '@')))
	{
		// Principal is that specified by user
		realm = at + 1;
		char *dot = ::strchr(mServer, '.');
		service.assign(mServer,  dot - mServer);
		instance.assign(dot + 1, at - dot - 1);
	}
	else
	{

		// Prepend service name
		switch(mServerType)
		{
		case eServerIMAP:
			service = "imap";
			break;
		case eServerPOP3:
			service = "pop";
			break;
		case eServerIMSP:
			service = "imap";
			break;
		case eServerSMTP:
			service = "smtp";
			break;
		case eServerACAP:
			service = "acap";
			break;
		case eServerManageSIEVE:
			service = "sieve";
			break;
		default:;
		}

		realm = ::krb_realmofhost(mRealServer);
		int len = strlen(mRealServer);
		for (int i = 0; i < len; i++)
		{
			if (mRealServer[i]=='.')
				break;
			instance += tolower(mRealServer[i]);
		}
	}

#if __dest_os == __mac_os && TARGET_API_MAC_CARBON
	// Make sure valid tickets exist and prompt user if not
	KLStatus kerr = ::KLAcquireTickets(NULL, NULL, NULL);
#endif
#endif

    ::memcpy(&ran_num, p, 4);
    ran_num = ntohl(ran_num);

    char tkt[1500];
    char b64_tkt[4000];
    unsigned long tkt_len = sizeof(tkt);
#ifdef USE_KCLIENT
    rc = ::KClientGetTicketForServiceFull(&kses, actual_principal, tkt, &tkt_len, ran_num);
#else
	KTEXT_ST authent;
	// Using CKrb4PluginBurnData ensures that this memory will get erased when
	// we leave this function.  Yes, it gets deallocated becaues it is on the
	// stack, but we wouldn't want to leave anything around for anyone to snoop
	// in any longer than we have to. Ben would yell at me if I did that.
	CKerberosPluginBurnData<KTEXT_ST> burnAuth(&authent);
	
	CREDENTIALS creds;
	CKerberosPluginBurnData<CREDENTIALS> brunCred(&creds);
	rc = ::krb_mk_req(&authent, const_cast<char*>(service.c_str()), 
									const_cast<char*>(instance.c_str()),
									const_cast<char*>(realm.c_str()), 0);
	if (rc)
	{
		std::string error_txt;
		error_txt = "krb_mk_req failed: ";
		error_txt += service;
		error_txt += ", ";
		error_txt += instance;
		error_txt += ", ";
		error_txt += realm;
		error_txt += ",";
		AUTHERRORLOG(error_txt.c_str(), rc);
	}

	rc = ::krb_get_cred(const_cast<char*>(service.c_str()), 
										const_cast<char*>(instance.c_str()), 
										const_cast<char*>(realm.c_str()), &creds);
	if (rc)
		AUTHERRORLOG("krb_get_cred failed:", rc);

	memcpy(mSession, creds.session, sizeof(des_cblock));
	::des_key_sched(&mSession[0], &mSchedule[0]);
	if (rc == 0)
	{
		rc = ::krb_mk_req(&authent, const_cast<char*>(service.c_str()), 
										const_cast<char*>(instance.c_str()), 
										const_cast<char*>(realm.c_str()),
										ran_num);
		if (rc)
		{
			std::string error_txt;
			error_txt = "krb_mk_req 2 failed: ";
			error_txt += service;
			error_txt += ", ";
			error_txt += instance;
			error_txt += ", ";
			error_txt += realm;
			error_txt += ",";
			AUTHERRORLOG(error_txt.c_str(), rc);
		}
	}
#endif
    if (rc != 0) 
    	AUTHERROR("can't get a service ticket for server", true);

#ifndef USE_KCLIENT
	::memcpy(tkt, authent.dat, authent.length);
	tkt_len = authent.length;
#endif
	*p = 0;
	
#ifdef USE_KCLIENT
/* skip over the 4 byte length foo */
#define CHOP_LEN (4)
#else
#define CHOP_LEN (0)
#endif

	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		mLiteralLength = tkt_len - CHOP_LEN;
		::sprintf(p, "{%ld}", mLiteralLength);
		::memcpy(mLiteralBuffer, &mLiteralLength, 4);
		::memcpy(mLiteralBuffer + 4, (unsigned char *)((char*)(tkt) + CHOP_LEN), mLiteralLength);

		// Return code to tell Mulberry to send a literal
		mState = eFirstLineLiteralSend;
		return eAuthSendDataToServer;
		break;
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		// Do base64 encoding
	  	::kbase64_to64((unsigned char *)b64_tkt, (unsigned char *)(tkt+CHOP_LEN), tkt_len-CHOP_LEN);
		::strcpy(p,b64_tkt);
		break;
	case eServerManageSIEVE:
		// Do base64 encoding
	  	::kbase64_to64((unsigned char *)b64_tkt, (unsigned char *)(tkt+CHOP_LEN), tkt_len-CHOP_LEN);
		::strcpy(p, "\"");
		::strcat(p, b64_tkt);
		::strcat(p, "\"");
		break;
	default:;
	}

	// Return code to tell Mulberry to send this data and get a response from the server
	mState = eSecondLine;
	return eAuthSendDataToServer;
}

// Process first line from server
long CKerberosPluginDLL::ProcessFirstLiteralSend(SAuthPluginData* info)
{
	char* p = info->data;
	// Must have continuation
	if (*p != '+')
		AUTHERROR("expected '+ ' continuation", false);

	// Copy data to server buffer
	::memcpy(p, mLiteralBuffer, mLiteralLength + 4);

	// Send the literal
	mState = eSecondLine;
	return eAuthSendLiteralDataToServer;
}

// Process second line from server
long CKerberosPluginDLL::ProcessSecond(SAuthPluginData* info)
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
	default:;
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
	    int len = ::kbase64_from64(info->data, p);
	    p[len] = 0;
		if (len < 8)
			AUTHERROR("second auth response is too short", true);
		
		// set length
		mLiteralLength = len;
			
	    // Process data
	    return ProcessSecondData(info);
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
			
			// set length
			mLiteralLength = ::strlen(info->data);

			// Process data
			return ProcessSecondData(info);
		}
		else if (*p == '{')
		{
			mLiteralLength = ::atoi(p+1);

			// Get more data from server and process directly
			mState = eSecondLineLiteral;
			return eAuthMoreData;
		}
		else
			AUTHERROR("illegal data return by server: not a string", true);
		break;
	default:
		AUTHERROR("can't use KERBEROS_V4 with this server type", true);
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process second line from server
long CKerberosPluginDLL::ProcessSecondData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    mLiteralLength = ::kbase64_from64(info->data, p);
	    if (mLiteralLength < 0)
	    	mLiteralLength = 0;
	    p[mLiteralLength] = 0;
	}

	char* p = info->data;

	char b64_buf[500];
	char buf[500];
	char long_buf[4];
	long srv_ran_num;
	long maxbufsize;
	int prot_flags;
	int len;
	int rc;
	
	::memcpy(buf, p, mLiteralLength);
	buf[mLiteralLength] = 0;

#ifdef USE_KCLIENT
	rc=::KClient_des_ecb_encrypt(&kses, (unsigned char *)buf, (unsigned char *)buf,0);
#else
	::des_ecb_encrypt((C_Block*)buf, (C_Block*)buf, mSchedule, 0);
	rc = 0;
#endif
	if (rc != 0)
		AUTHERROR("server challenge doesn't match", true);
	::memcpy(&srv_ran_num,buf,4);
	srv_ran_num = ntohl(srv_ran_num);

	if (srv_ran_num != ran_num + 1)
		AUTHERROR("server challenge doesn't match", true);
	::memcpy(&long_buf, buf + 4, 4);
	long_buf[0] = 0;		/*one byte is used for flags... go figure*/
	::memcpy(&maxbufsize, long_buf, 4);
	maxbufsize = ntohl(maxbufsize);

	*(int *)buf = htonl(ran_num);
	*(int *)(buf + 4) = htonl(maxbufsize);
	buf[4]=SASL_PROT_NONE;

	if	(mUseUserID)
		::strcpy(buf + 8, mUserID);
#ifdef USE_KCLIENT
	else if((rc = ::KClientGetUserName(buf + 8)) != 0)
	{
		AUTHERROR("can't get default user from KClient", true);
		// ???
		::strcpy(buf + 8, "");		//??? fix this
	}
#else
	else
	{
		char user[MAX_K_NAME_SZ+1];
		char uRealm[INST_SZ];
		char uInstance[INST_SZ];
		char tkt_file_name[1024];
		::strncpy(tkt_file_name, ::tkt_string(), 1024);
		tkt_file_name[1023] = 0;
		if (::krb_get_tf_fullname(&tkt_file_name[0], &user[0], &uInstance[0], &uRealm[0]))
			AUTHERROR("Can't get default Kerberos user", true);
 		::strcpy(buf + 8, user);
	}
#endif
	// Save current id as recovered user id
	::strcpy(mRecoveredUserID, buf+8);

	::memset(buf + 8 + ::strlen(buf + 8), 0, 8);

	len = ::strlen(buf + 8) + 8 + 1;
	len += 7;			/*make the length a multiple of 8*/
	len &= ~7;
#ifdef USE_KCLIENT
	rc = ::KClient_des_pcbc_encrypt(&kses, (unsigned char *)buf, (unsigned char *)buf, len, 1);
#else
#if __dest_os == __linux_os
	::des_pcbc_encrypt((C_Block*)buf, (C_Block*)buf, len, &mSchedule[0], &mSession, 1);
#else
	::des_pcbc_encrypt((C_Block*)buf, (C_Block*)buf, len, &mSchedule[0], &mSession, 1);
#endif
	rc = 0;
#endif
	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
		mLiteralLength = len;
		::sprintf(p, "{%ld}", mLiteralLength);
		::memcpy(mLiteralBuffer, &mLiteralLength, 4);
		::memcpy(mLiteralBuffer + 4, (unsigned char *)buf, mLiteralLength);

		// Return code to tell Mulberry to send a literal
		mState = eSecondLineLiteralSend;
		return eAuthSendDataToServer;
		break;
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		// Do base64 encoding
	  	::kbase64_to64((unsigned char *)b64_buf,(unsigned char *)buf,len);
		::strcpy(p,b64_buf);
		break;
	case eServerManageSIEVE:
		// Do base64 encoding
	  	::kbase64_to64((unsigned char *)b64_buf,(unsigned char *)buf,len);
		::strcpy(p, "\"");
		::strcat(p, b64_buf);
		::strcat(p, "\"");
		break;
	default:;
	}
	
	// Return code to tell Mulberry to send this data and get a response from the server
	mState = eTagLine;
	return eAuthSendDataToServer;
}

// Process first line from server
long CKerberosPluginDLL::ProcessSecondLiteralSend(SAuthPluginData* info)
{
	char* p = info->data;
	// Must have continuation
	if (*p != '+')
		AUTHERROR("expected '+ ' continuation", false);

	// Copy data to server buffer
	::memcpy(p, mLiteralBuffer, mLiteralLength + 4);

	// Send the literal
	mState = eTagLine;
	return eAuthSendLiteralDataToServer;
}

// Process tag line from server
long CKerberosPluginDLL::ProcessTag(SAuthPluginData* info)
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

