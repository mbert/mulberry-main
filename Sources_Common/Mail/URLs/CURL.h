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


// CURL.h

#ifndef __CURL__MULBERRY__
#define __CURL__MULBERRY__

#include "cdstring.h"

// Initially only constants

extern const char  cURLHierarchy;
extern const char* cURLMainScheme;
extern const long cURLMainSchemeLength;
extern const char* cMailtoURLScheme;
extern const char* cFileURLScheme;
extern const char* cURLLocalhost;
extern const char* cMboxURLScheme;
extern const char* cIMAPURLScheme;
extern const char* cPOPURLScheme;
extern const char* cIMSPURLScheme;
extern const char* cIMSPURLOption;
extern const char* cIMSPURLAddress;
extern const char* cACAPURLScheme;
extern const char* cSMTPURLScheme;
extern const char* cLDAPURLScheme;
extern const char* cADBKURLScheme;
extern const char* cCALURLScheme;
extern const char* cHTTPURLScheme;
extern const char* cHTTPSURLScheme;
extern const char* cWEBCALURLScheme;
extern const char* cURLAUTH;
extern const char* cURLAPOP;

extern const char  cURLEscape;
extern const char* cURLReserved;
extern const char cURLUnreserved[];
extern const char cURLCharacter[];
extern const char cURLXCharacter[];


extern const char* cHexChar;
extern const unsigned char cFromHex[];

class CURL
{
public:
	enum EScheme
	{
		eNone = 0,
		eMailto,
		eFile,
		eLocalhost,
		eMbox,
		eIMAP,
		ePOP,
		eIMSP,
		eACAP,
		eSMTP,
		eLDAP,
		eADBK,
		eCAL,
		eHTTP,
		eHTTPS,
		eWebcal,
		eUnknown
	};
	
	enum EConversion
	{
		eAbsolute = 0,
		eRelative,
		eLastPath
	};

	CURL(const cdstring& url, bool decode = false)
	{
		Parse(url, decode);
	}
	CURL(const char* url, bool decode = false)
	{
		cdstring temp(url);
		Parse(temp, decode);
	}
	CURL(EScheme scheme, const cdstring& rurl, bool decode = false);
	
	EScheme SchemeType() const
	{
		return mSchemeType;
	}
	const cdstring& Scheme() const
	{
		return mScheme;
	}
	
	const cdstring& User() const
	{
		return mUser;
	}
	
	const cdstring& Password() const
	{
		return mPassword;
	}
	
	const cdstring& Server() const
	{
		return mServer;
	}
	
	const cdstring& Path() const
	{
		return mPath;
	}

	const cdstring& Extended() const
	{
		return mExtended;
	}

	cdstring ToString(EConversion conversion = eAbsolute, bool encode = true) const;

	bool Equal(const CURL& comp) const;
	bool EqualRelative(const CURL& comp) const;

protected:
	EScheme		mSchemeType;
	cdstring	mScheme;
	cdstring	mUser;
	cdstring	mPassword;
	cdstring	mServer;
	cdstring	mPath;
	cdstring	mExtended;
	
	void Parse(const cdstring& url, bool decode = false);

};
#endif
