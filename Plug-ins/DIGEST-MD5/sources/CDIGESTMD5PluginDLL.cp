/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

// CDIGESTMD5PluginDLL.cp
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
// 02-Mar-1999: Created initial header and implementation.
//

#include "CDIGESTMD5PluginDLL.h"
#include <stdlib.h>
#include <stdio.h>
#include "CPluginInfo.h"

#include "CStringUtils.h"
#include "kbase64.h"
#include "hmac-md5.h"

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#include <memory>
#include <time.h>
//using namespace std;

void _H(unsigned char* , char*, unsigned long);
void _H(unsigned char* digest, char* s, unsigned long s_len)
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char*) s, s_len);
	MD5Final(digest, &ctx);
}

void _KD(unsigned char*, char*, char*);
void _KD(unsigned char* digest, char* k, char* s)
{
	std::auto_ptr<char> p(new char[::strlen(k) + ::strlen(s) + 2]);
	::strcpy(p.get(), k);
	::strcat(p.get(), ":");
	::strcat(p.get(), s);
	_H(digest, p.get(), ::strlen(p.get()));
}

void _HEX_IT(char*, unsigned char*);
void _HEX_IT(char* digest_hex, unsigned char* digest)
{
	for(int i = 0; i < HMAC_MD5_SIZE; i++)
	{
		unsigned char lo_q = digest[i];
		unsigned char hi_q = (lo_q >> 4);
		lo_q = lo_q & 0xF;
		digest_hex[2*i] = hi_q + ((hi_q >= 0xA) ? ('a' - 0x0A) : '0');
		digest_hex[2*i + 1] = lo_q + ((lo_q >= 0xA) ? ('a' - 0x0A) : '0');
	}
	digest_hex[2*HMAC_MD5_SIZE] = 0;
}

#define AUTHERROR(xx_msg) do { \
	char err_buf[256]; \
  	mState = eError; \
    ::strcpy(err_buf, "DIGEST-MD5 Plugin Error: "); \
    ::strcat(err_buf, xx_msg); \
    LogEntry(err_buf); \
  	return eAuthServerError; \
  } while (0)

#pragma mark ____________________________consts

const char* cPluginName = "DIGEST-MD5 Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginNetworkAuthentication;
const char* cPluginDescription = "DIGEST-MD5 authentication plugin for Mulberry." COPYRIGHT;
const char* cPluginAuthenticateID = "DIGEST-MD5";
const char* cPluginPrefsDescriptor = "DIGEST-MD5";
CAuthPluginDLL::EAuthPluginUIType cPluginAuthUIType = CAuthPluginDLL::eAuthUserPswd;

const char* cTokenSeparators = "()<>@,;:\\\"/[]?={} \t\r\n";

#pragma mark ____________________________CDIGESTMD5PluginDLL

// Constructor
CDIGESTMD5PluginDLL::CDIGESTMD5PluginDLL()
{
	mState = eFirstLine;
}

// Destructor
CDIGESTMD5PluginDLL::~CDIGESTMD5PluginDLL()
{
}

// Initialise plug-in
void CDIGESTMD5PluginDLL::Initialise(void)
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
bool CDIGESTMD5PluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'DMD5');
	return false;
}

// Can plug-in run as demo
bool CDIGESTMD5PluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

// Test for run ability
bool CDIGESTMD5PluginDLL::CanRun(void)
{
	// Check for KClient etc here
	return true;
}

// Process data
long CDIGESTMD5PluginDLL::ProcessData(SAuthPluginData* info)
{
	switch(mState)
	{
	case eFirstLine:
		return ProcessFirst(info);
	case eFirstLineLiteral:
		return ProcessFirstData(info);
	case eSecondLine:
		return ProcessSecond(info);
	case eSecondLineLiteral:
		return ProcessSecondData(info);
	case eTagLine:
		return ProcessTag(info);
	default:
		return eAuthError;
	}
}

// Returns the name of the plug-in
const char* CDIGESTMD5PluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CDIGESTMD5PluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CDIGESTMD5PluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CDIGESTMD5PluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CDIGESTMD5PluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Process first line from server
long CDIGESTMD5PluginDLL::ProcessFirst(SAuthPluginData* info)
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
			AUTHERROR("expected '+ ' continuation in Step 1");
		p += 2;
		break;
	case eServerSMTP:
		if (::strncmp(p, "334 ", 4) != 0)
			AUTHERROR("expected '334 ' continuation in Step 1");
		p += 4;
		break;
	case eServerManageSIEVE:
		// No tags or continuation
		break;
	default:
		AUTHERROR("cant use DIGEST-MD5 with this server type");
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

	    // Process DIGEST-MD5
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
					*q++ = *p++;
			}
			*q = 0;
			
			// Process DIGEST-MD5
			return ProcessFirstData(info);
		}
		else if (*p == '{')
		{
			// Get more data from server and process directly
			mState = eFirstLineLiteral;
			return eAuthMoreData;
		}
		else
			AUTHERROR("illegal data return by server: not a string in Step 1");
	default:
		AUTHERROR("Server Type not supported");
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process first line data from server
long CDIGESTMD5PluginDLL::ProcessFirstData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    int len = kbase64_from64(info->data, p);
	    info->data[len] = 0;
	}

	char* p = info->data;
	PuntLWS(p);

	bool got_qop = false;
	std::auto_ptr<const char> nonce;
	std::auto_ptr<const char> opaque;
	std::auto_ptr<const char> realm;
	std::auto_ptr<const char> cnonce;

	// Tokenize to get nonce
	while(*p)
	{
		// Get token name
		char* token = ::strgettokenstr(&p, cTokenSeparators);
		
		if (!token)
			AUTHERROR("Illegal token in Step 1");

		// Punt across token = if present (note it may have been removed by strgettokenstr)
		PuntLWS(p);
		if (*p == '=')
			PuntLWS(++p);

		if (::strcmpnocase(token, "nonce") == 0)
		{
			char* temp = ::strgettokenstr(&p, cTokenSeparators);
			nonce.reset(::strdup(temp));
		}
		else if (::strcmpnocase(token, "opaque") == 0)
		{
			char* temp = ::strgettokenstr(&p, cTokenSeparators);
			opaque.reset(::strdup(temp));
		}
		else if (::strcmpnocase(token, "realm") == 0)
		{
			char* temp = ::strgettokenstr(&p, cTokenSeparators);
			realm.reset(::strdup(temp));
		}
		else if (::strcmpnocase(token, "qop") == 0)
		{
			// Look for qop and reject unknown or missing
			char* param = ::strgettokenstr(&p, cTokenSeparators);
			if (param)
			{
				// Check list of qop items - must match what we expect
				char* q = ::strtok(param, ",");
				while(q && *q)
				{
					if ((::strcmpnocase(param, "auth") == 0) ||
				 		(::strcmpnocase(param, "auth-int") == 0) ||
				 		(::strcmpnocase(param, "auth-conf") == 0))
				 	{
						got_qop = true;
						q = ::strtok(NULL, ",");
					}
					else
						AUTHERROR("Unrecognised qop parameter in Step 1");
				}
			}
		}
		else
		{
			// Unknown/unwanted token - just ignore
			::strgettokenstr(&p, cTokenSeparators);
		}

		// must be pointing at end of string or comma
		PuntLWS(p);
		if (*p == ',')
		{
			p++;
			PuntLWS(p);
		}
	}
	
	// Must reject some states
	if (!got_qop)
		AUTHERROR("Missing qop parameter in Step 1");
	
	// Generate response
	
	// Generate a cnonce
	{
		char msg_id[256];
		::sprintf(msg_id, "%lu.%lu", clock(), time(NULL));

		// Do base64 encoding in place
		char* q = msg_id + ::strlen(msg_id) + 1;
		kbase64_to64((unsigned char*) q, (unsigned char*) msg_id, ::strlen(msg_id));
		::memmove(msg_id, q, ::strlen(q) + 1);
		cnonce.reset(::strdup(msg_id));
	}
	
	//nonce.reset(::strdup("123"));
	//cnonce.reset(::strdup("123"));

	// Split user id into user id/realm
	std::auto_ptr<const char> userid;
	const char* useridat = ::strrchr(mUserID, '@');
	if (useridat)
	{
		// Split at least '@' sign
		// If '@' is legitimetaly part of the user id, users can end with a trailing
		// '@' to force an empty realm
		userid.reset(::strndup(mUserID, useridat - mUserID));
		realm.reset(::strdup(useridat + 1));
	}
	else
		userid.reset(::strdup(mUserID));

	// Caclulate HEX(H(A1))
	char hex_h_a1[33];
	{
		std::auto_ptr<char> a1_data(new char[::strlen(userid.get()) +
									(realm.get() ? ::strlen(realm.get()) : 0) +
									::strlen(mPassword) + 3]);
		::strcpy(a1_data.get(), userid.get());
		::strcat(a1_data.get(), ":");
		if (realm.get())
			::strcat(a1_data.get(), realm.get());
		::strcat(a1_data.get(), ":");
		::strcat(a1_data.get(), mPassword);
		
		unsigned char digest[16];
		_H(digest, a1_data.get(), ::strlen(a1_data.get()));
		a1_data.reset(new char[16 +
							::strlen(nonce.get()) +
							::strlen(cnonce.get()) + 3]);
		::memcpy(a1_data.get(), digest, 16);
		char* a1_data_next = a1_data.get() + 16;
		::strcpy(a1_data_next, ":");
		::strcat(a1_data_next, nonce.get());
		::strcat(a1_data_next, ":");
		::strcat(a1_data_next, cnonce.get());
		
		_H(digest, a1_data.get(), ::strlen(a1_data_next) + 16);
		_HEX_IT(hex_h_a1, digest);
	}

	// Need digest-uri-value (ignore host != serv-name)
	std::auto_ptr<char> digest_uri_value(new char[
										4 + // serv-type
										1 + // /
										::strlen(mServer) + // host
										1]);
	switch(mServerType)
	{
	case eServerACAP:
		::strcpy(digest_uri_value.get(), "acap");
		break;
	case eServerIMAP:
		::strcpy(digest_uri_value.get(), "imap");
		break;
	case eServerPOP3:
		::strcpy(digest_uri_value.get(), "pop3");
		break;
	case eServerIMSP:
		::strcpy(digest_uri_value.get(), "imap");
		break;
	case eServerSMTP:
		::strcpy(digest_uri_value.get(), "smtp");
		break;
	case eServerManageSIEVE:
		::strcpy(digest_uri_value.get(), "sieve");
		break;
	default:;
	}
	::strcat(digest_uri_value.get(), "/");
	::strcat(digest_uri_value.get(), mServer);
	
	// Calculate client HEX(H(A2))
	char hex_h_a2_client[33];
	{
		std::auto_ptr<char> a2_data(new char[
									12 + // AUTHENTICATE:
									::strlen(digest_uri_value.get()) +
									1]);
		::strcpy(a2_data.get(), "AUTHENTICATE:");
		::strcat(a2_data.get(), digest_uri_value.get());
		unsigned char digest[16];
		_H(digest, a2_data.get(), ::strlen(a2_data.get()));
		_HEX_IT(hex_h_a2_client, digest);
	}
	
	// Calculate server HEX(H(A2))
	char hex_h_a2_server[33];
	{
		std::auto_ptr<char> a2_data(new char[
									1 + // :
									::strlen(digest_uri_value.get()) +
									1]);
		::strcpy(a2_data.get(), ":");
		::strcat(a2_data.get(), digest_uri_value.get());
		unsigned char digest[16];
		_H(digest, a2_data.get(), ::strlen(a2_data.get()));
		_HEX_IT(hex_h_a2_server, digest);
	}
	
	// Now do combined calc for client
	char hex_response_client[33];
	{
		char* kd1 = hex_h_a1;
		std::auto_ptr<char> kd2(new char[::strlen(nonce.get()) +
								::strlen(cnonce.get()) +
								4 + // :'s
								8 + // nc_value
								4 + // "auth"
								32 + // HEX(H(A2))
								1]);
		::strcpy(kd2.get(), nonce.get());
		::strcat(kd2.get(), ":00000001:");
		::strcat(kd2.get(), cnonce.get());
		::strcat(kd2.get(), ":auth:");
		::strcat(kd2.get(), hex_h_a2_client);
		
		unsigned char digest[16];
		_KD(digest, kd1, kd2.get());
		_HEX_IT(hex_response_client, digest);
	}

	// Now do combined calc for server
	{
		char* kd1 = hex_h_a1;
		std::auto_ptr<char> kd2(new char[::strlen(nonce.get()) +
								::strlen(cnonce.get()) +
								4 + // :'s
								8 + // nc_value
								4 + // "auth"
								32 + // HEX(H(A2))
								1]);
		::strcpy(kd2.get(), nonce.get());
		::strcat(kd2.get(), ":00000001:");
		::strcat(kd2.get(), cnonce.get());
		::strcat(kd2.get(), ":auth:");
		::strcat(kd2.get(), hex_h_a2_server);
		
		unsigned char digest[16];
		_KD(digest, kd1, kd2.get());
		_HEX_IT(hex_response_server, digest);
	}

	// Now form response string
	p = info->data;
	*p = 0;
	
	::strcat(p, "username=\"");
	::strcat(p, userid.get());
	if (realm.get())
	{
		::strcat(p, "\",realm=\"");
		::strcat(p, realm.get());
	}
	::strcat(p, "\",nonce=\"");
	::strcat(p, nonce.get());
	if (opaque.get())
	{
		::strcat(p, "\",opaque=\"");
		::strcat(p, opaque.get());
	}
	::strcat(p, "\",cnonce=\"");
	::strcat(p, cnonce.get());
	::strcat(p, "\",nc=00000001,qop=auth,digest-uri=\"");
	::strcat(p, digest_uri_value.get());
	::strcat(p, "\",response=");
	::strcat(p, hex_response_client);

	// May need to quote if string
	switch(mServerType)
	{
	case eServerACAP:
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
	{
		// Do base64 encoding in place
		char* q = p + ::strlen(p) + 3;
		kbase64_to64((unsigned char*) q, (unsigned char*) p, ::strlen(p));
		p[0] = '\"';
		::memmove(p + 1, q, ::strlen(q) + 1);
		::strcat(p, "\"");
		break;
	}
	default:;
	}

	// ACAP/SIEVE goes straight to tag line - others require second response
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
	case eServerSMTP:
		mState = eSecondLine;
		break;
	case eServerManageSIEVE:
	case eServerACAP:
		mState = eTagLine;
		break;
	default:;
	}

	// Return code to tell Mulberry to send this data and get a response from the server
	return eAuthSendDataToServer;
}

// Process second line from server
long CDIGESTMD5PluginDLL::ProcessSecond(SAuthPluginData* info)
{
	char* p = info->data;

	// Must have continuation/tag
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerPOP3:
	case eServerIMSP:
		if ((*p != '+') || (p[1]!=' '))
			AUTHERROR("expected '+ ' continuation in Step 3");
		p += 2;
		break;
	case eServerSMTP:
		if (::strncmp(p, "334 ", 4) != 0)
			AUTHERROR("expected '334 ' continuation in Step 3");
		p += 4;
		break;
	case eServerManageSIEVE:
	case eServerACAP:
		AUTHERROR("cannot do separate Step 3");
		break;
	default:
		AUTHERROR("cannot use DIGEST-MD5 with this server type");
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

	    // Process DIGEST-MD5
	    return ProcessSecondData(info);
	}
	default:
		AUTHERROR("Server Type not supported");
	}
	
	// Must be error at this point
	mState = eError;
	return eAuthError;
}

// Process second line data from server
long CDIGESTMD5PluginDLL::ProcessSecondData(SAuthPluginData* info)
{
	// Special - ManageSIEVE data is base64 encoded and we need to adjust that here
	if (mServerType == eServerManageSIEVE)
	{
		char* p = info->data;
	    int len = kbase64_from64(info->data, p);
	    info->data[len] = 0;
	}

	char* p = info->data;
	PuntLWS(p);

	bool got_rspauth = false;
	std::auto_ptr<const char> rspauth;

	// Tokenize to get nonce
	while(*p)
	{
		char* token = ::strgettokenstr(&p, cTokenSeparators);
		
		if (!token)
			AUTHERROR("Illegal token in Step 3");

		// Punt across token = if present (note it may have been removed by strgettokenstr)
		PuntLWS(p);
		if (*p == '=')
			PuntLWS(++p);

		if (::strcmpnocase(token, "rspauth") == 0)
		{
			char* temp = ::strgettokenstr(&p, cTokenSeparators);
			rspauth.reset(::strdup(temp));
			got_rspauth = true;
		}

		// must be pointing at end of string or comma
		PuntLWS(p);
		if (*p == ',')
		{
			p++;
			PuntLWS(p);
		}
	}
	
	// Must reject some states
	if (!got_rspauth)
		AUTHERROR("Missing rspauth parameter in Step 3");
	
	// Verify response
	if (::strcmp(rspauth.get(), hex_response_server))
		AUTHERROR("Server rspauth parameter wrong in Step 3");

	// Null data to send
	p = info->data;
	*p = 0;

	// Return code to tell Mulberry to get another response from the server
	mState = eTagLine;
	return eAuthSendDataToServer;
}

// Process tag line from server
long CDIGESTMD5PluginDLL::ProcessTag(SAuthPluginData* info)
{
	const char* p = info->data;
	
	// Now look at full response
	switch(mServerType)
	{
	case eServerIMAP:
	case eServerIMSP:
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
			// Now look for SASL response code
			p += 2;
			if (::strncmpnocase(p, " (SASL ", 7) == 0)
			{
				// Now check respauth
				mState = eDone;
				return eAuthDone;
			}
		}
		mState = eError;
		return eAuthServerError;

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

void CDIGESTMD5PluginDLL::PuntLWS(char*& txt)
{
	while(true)
	{
		switch(*txt)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			txt++;
			break;
		default:
			return;
		}
	}
}
