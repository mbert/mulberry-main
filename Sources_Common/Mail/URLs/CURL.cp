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


// CURL.cp

// Initially only constants

#include "CURL.h"

const char cURLHierarchy = '/';
const char* cURLMainScheme = "URL:";
const long cURLMainSchemeLength = 4;
const char* cMailtoURLScheme = "mailto:";
const char* cFileURLScheme = "file://";
const char* cURLLocalhost = "localhost/";
const char* cMboxURLScheme = "mbox://";
const char* cIMAPURLScheme = "imap://";
const char* cPOPURLScheme = "pop://";
const char* cIMSPURLScheme = "imsp://";
const char* cIMSPURLOption = "option/";
const char* cIMSPURLAddress = "addressbook/";
const char* cACAPURLScheme = "acap://";
const char* cSMTPURLScheme = "smtp://";
const char* cLDAPURLScheme = "ldap://";
const char* cADBKURLScheme = "adbk://";
const char* cCALURLScheme = "x-cal://";
const char* cHTTPURLScheme = "http://";
const char* cHTTPSURLScheme = "https://";
const char* cWEBCALURLScheme = "webcal://";
const char* cURLAUTH = ";AUTH=";
const char* cURLAPOP = ";AUTH=+APOP";

const char* cSchemes[] = 
{
	cURLMainScheme,
	cMailtoURLScheme,
	cFileURLScheme,
	cURLLocalhost,
	cIMAPURLScheme,
	cPOPURLScheme,
	cIMSPURLScheme,
	cACAPURLScheme,
	cSMTPURLScheme,
	cLDAPURLScheme,
	cADBKURLScheme,
	cCALURLScheme,
	cHTTPURLScheme,
	cHTTPSURLScheme,
	cWEBCALURLScheme
};

const char cURLEscape = '%';
const char* cURLReserved = ";/?:@&=";
const char cURLUnreserved[] = // Allowable URL chars
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,		// 48 - 63
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1,		// 80 - 95
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char cURLCharacter[] = // Allowable URL chars -- all
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,		// 48 - 63
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1,		// 80 - 95
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char cURLXCharacter[] = // Allowable URL chars (all)
							  // RFC2732 uses '[...]' for IPv6 addressing - [] are now allowed
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,		// 48 - 63
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,		// 80 - 95
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char* cHexChar = "0123456789ABCDEF";
const char cFromHex[] =
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 0 - 15
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 16 - 31
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 32 - 47
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 48 - 63
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 64 - 79
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 80 - 95
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 96 - 111
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 112 - 127
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 128 - 143
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 144 - 159
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 160 - 175
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 176 - 191
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 192 - 207
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 208 - 223
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		// 224 - 239
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};	// 240 - 255


CURL::CURL(EScheme scheme, const cdstring& rurl, bool decode)
{
	mSchemeType = scheme;
	mScheme = cSchemes[mSchemeType];
	mPath = rurl;
	if (decode)
		mPath.DecodeURL();
}

void CURL::Parse(const cdstring& url, bool decode)
{
	cdstring temp(url);

	// Strip off main scheme
	if (temp.compare_start(cURLMainScheme, true))
		temp.erase(0, ::strlen(cURLMainScheme));

	// Only handle HTTP, HTTPS & Webcal right now
	uint32_t punt_size = 0;
	if (temp.compare_start(cHTTPURLScheme, true))
	{
		mSchemeType = eHTTP;
		punt_size = ::strlen(cHTTPURLScheme);
	}
	else if (temp.compare_start(cHTTPSURLScheme, true))
	{
		mSchemeType = eHTTPS;
		punt_size = ::strlen(cHTTPSURLScheme);
	}
	else if (temp.compare_start(cWEBCALURLScheme, true))
	{
		mSchemeType = eWebcal;
		punt_size = ::strlen(cWEBCALURLScheme);
	}
	else if (temp.compare_start(cMailtoURLScheme, true))
	{
		mSchemeType = eMailto;
		punt_size = ::strlen(cMailtoURLScheme);
	}
	
	// Special if it starts with a / its a relative http url
	else if (temp[(cdstring::size_type)0] == '/')
	{
		mSchemeType = eHTTP;
		punt_size = 0;
	}
	else
	{
		mSchemeType = eUnknown;
		const char* p = ::strchr(temp.c_str(), ':');
		if (p != NULL)
			punt_size = p - temp.c_str();
	}

	switch(mSchemeType)
	{
	case eHTTP:
	case eHTTPS:
	case eWebcal:
	{
		if (punt_size != 0)
		{
			mScheme.assign(temp, 0, punt_size);
			temp.erase(0, punt_size);
		}
		
		// Look for server
		const char* p = ::strchr(temp.c_str(), '/');
		cdstring host;
		if (p == NULL)
		{
			host = temp;
		}
		else
		{
			punt_size = p - temp.c_str();
			if (punt_size != 0)
			{
				host.assign(temp, 0, punt_size);
				temp.erase(0, punt_size);
			}
			
			mPath = temp;
			if (decode)
				mPath.DecodeURL();
		}
		
		// Now decode user:password@server
		p = ::strchr(host.c_str(), '@');
		cdstring userpswd;
		if (p == NULL)
		{
			mServer = host;
		}
		else
		{
			punt_size = p - host.c_str();
			if (punt_size != 0)
			{
				userpswd.assign(host, 0, punt_size);
				host.erase(0, punt_size+1);
			}
			mServer = host;
			
			p = ::strchr(userpswd.c_str(), ':');
			if (p == NULL)
			{
				mUser = userpswd;
			}
			else
			{
				punt_size = p - userpswd.c_str();
				if (punt_size != 0)
				{
					mUser.assign(userpswd, 0, punt_size);
					userpswd.erase(0, punt_size+1);
				}
				
				mPassword = userpswd;
			}
		}
		break;
	}
	case eMailto:
	{
		if (punt_size != 0)
		{
			mScheme.assign(temp, 0, punt_size);
			temp.erase(0, punt_size);
		}
		
		// Look for server
		mServer = temp;
		break;
	}
		
	default:;
	}
}

cdstring CURL::ToString(EConversion conversion, bool encode) const
{
	cdstring result;

	// Add scheme & host if not relative
	if (conversion == eAbsolute)
	{
		result += mScheme;
		switch(mSchemeType)
		{
		case eHTTP:
		case eHTTPS:
		case eWebcal:
		{
			if (!mUser.empty())
			{
				result += mUser;
				if (!mPassword.empty())
				{
					result += ":";
					result += mPassword;
				}
				result += "@";
			}
		}
		default:;
		}
		result += mServer;
	}

	// Get path (or last part of it if required)
	cdstring temp(mPath);
	if (conversion == eLastPath)
	{
		if (temp.find('/') != cdstring::npos)
			temp.assign(mPath, mPath.rfind('/') + 1, cdstring::npos);
	}

	// Now encode if required
	if (encode)
	{
		temp.EncodeURL('/');
	}
	result += temp;
	
	return result;
}

bool CURL::Equal(const CURL& comp) const
{
	// Compare each component
	if (mScheme.compare(comp.mScheme, true) != 0)
		return false;
	
	if (mUser.compare(comp.mUser, true) != 0)
		return false;
	
	if (mPassword.compare(comp.mPassword, true) != 0)
		return false;
	
	if (mServer.compare(comp.mServer, true) != 0)
		return false;
	
	if (mPath.compare(comp.mPath) != 0)
	{
		if (mPath.compare_end("/"))
		{
			cdstring temp(mPath);
			temp.erase(temp.length() - 1);
			if (temp.compare(comp.mPath) == 0)
				return true;
		}
		else if (comp.mPath.compare_end("/"))
		{
			cdstring temp(comp.mPath);
			temp.erase(temp.length() - 1);
			if (mPath.compare(temp) == 0)
				return true;
		}
	}
	else
		return true;
	
	return false;
}

bool CURL::EqualRelative(const CURL& comp) const
{
	// Must be relative
	if (!comp.mServer.empty())
		return false;

	// Just compare paths
	if (mPath.compare(comp.mPath) != 0)
	{
		if (mPath.compare_end("/"))
		{
			cdstring temp(mPath);
			temp.erase(temp.length() - 1);
			if (temp.compare(comp.mPath) == 0)
				return true;
		}
		else if (comp.mPath.compare_end("/"))
		{
			cdstring temp(comp.mPath);
			temp.erase(temp.length() - 1);
			if (mPath.compare(temp) == 0)
				return true;
		}
	}
	else
		return true;

	return false;
}
