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

/* 
	CHTTPAuthorizationDigest.cpp

	Author:			
	Description:	<describe the CHTTPAuthorizationDigest class here>
*/

#include "CHTTPAuthorizationDigest.h"

#include "CStringUtils.h"

#include "CHTTPDefinitions.h"
#include "CHTTPRequestResponse.h"

#include "base64.h"
#include "md5.h"

#include <memory>
#include <stdio.h>

using namespace http; 

void CHTTPAuthorizationDigest::GenerateAuthorization(std::ostream& os, const CHTTPRequestResponse* request) const
{
	// Generate response
	const_cast<CHTTPAuthorizationDigest*>(this)->GenerateResponse(request);
	
	// Generate header
	if (mQop.empty())
	{
		os << cHeaderAuthorization << cHeaderDelimiter << "Digest "
		<< "username=\"" << mUser << "\"," << net_endl
		<< " realm=\"" << mRealm << "\"," << net_endl
		<< " nonce=\"" << mNonce << "\"," << net_endl
		<< " uri=\"" << request->GetRURI() << "\"," << net_endl
		<< " response=\"" << mResponse << "\"";
	}
	else
	{
		os << cHeaderAuthorization << cHeaderDelimiter << "Digest "
		<< "username=\"" << mUser << "\"," << net_endl
		<< " realm=\"" << mRealm << "\"," << net_endl
		<< " nonce=\"" << mNonce << "\"," << net_endl
		<< " uri=\"" << request->GetRURI() << "\"," << net_endl
		<< " qop=auth," << net_endl
		<< " nc=" << mNC << "," << net_endl
		<< " cnonce=\"" << mCnonce << "\"," << net_endl
		<< " response=\"" << mResponse << "\"";
	}
	
	if (mAlgorithm.length() != 0)
		os << "," << net_endl << " algorithm=\"" << mAlgorithm << "\"";
	if (mOpaque.length() != 0)
		os << "," << net_endl << " opaque=\"" << mOpaque << "\"";
	
	os << net_endl;
}

void CHTTPAuthorizationDigest::ParseAuthenticateHeader(const cdstrvect& hdrs)
{
	for(cdstrvect::const_iterator iter = hdrs.begin(); iter != hdrs.end(); iter++)
	{
		// Strip any space
		cdstring temp(*iter);
		char* p = temp.c_str_mod();
		
		// Must have digest token
		if (::stradvtokcmp(&p, "Digest") != 0)
			continue;
		
		// Get each name/value pair
		while(true)
		{
			char* name = ::strgettokenstr(&p, SPACE_TAB "=");
			if ((name == NULL) || (*p == 0))
				return;
			
			char* value = ::strgettokenstr(&p, SPACE_TAB ",");
			if (value == NULL)
				return;
			
			if (::strcmpnocase(name, "realm") == 0)
			{
				mRealm = value;
			}
			else if (::strcmpnocase(name, "domain") == 0)
			{
				mDomain = value;
			}
			else if (::strcmpnocase(name, "nonce") == 0)
			{
				mNonce = value;
			}
			else if (::strcmpnocase(name, "opaque") == 0)
			{
				mOpaque = value;
			}
			else if (::strcmpnocase(name, "stale") == 0)
			{
				mStale = (::strcmpnocase(value, "false") != 0);
			}
			else if (::strcmpnocase(name, "algorithm") == 0)
			{
				mAlgorithm = value;
			}
			else if (::strcmpnocase(name, "qop") == 0)
			{
				mQop = value;
			}
			else
			{
				// Unknown token - ignore
			}
			
			// Punt over comma
			while((*p != 0) && (*p == ','))
				p++;
		}
		
		break;
	}
}

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

void _HEX(char*, unsigned char*);
void _HEX(char* digest_hex, unsigned char* digest)
{
	for(int i = 0; i < 16; i++)
	{
		unsigned char lo_q = digest[i];
		unsigned char hi_q = (lo_q >> 4);
		lo_q = lo_q & 0xF;
		digest_hex[2*i] = hi_q + ((hi_q >= 0xA) ? ('a' - 0x0A) : '0');
		digest_hex[2*i + 1] = lo_q + ((lo_q >= 0xA) ? ('a' - 0x0A) : '0');
	}
	digest_hex[2*16] = 0;
}

void CHTTPAuthorizationDigest::GenerateResponse(const CHTTPRequestResponse* request)
{
	if (mQop.empty())
	{
		// Clear out old values
		mNC.clear();
		mCnonce.clear();
	}
	else
	{
		// Generate nc-count
		++mClientCount;
		mNC.clear();
		mNC.reserve(256);
		::snprintf(mNC.c_str_mod(), 256, "%08x", mClientCount);
		
		mCnonce = mNC;
		mCnonce += mUser;
		mCnonce += mPswd;
		mCnonce += mRealm;
		mCnonce.md5(mCnonce);
	}
	
	// Determine HA1
	unsigned char digest[16];
	cdstring A1;
	A1 = mUser;
	A1 += ":";
	A1 += mRealm;
	A1 += ":";
	A1 += mPswd;
	_H(digest, A1, A1.length());
	if (mAlgorithm == "MD5-sess")
	{
		cdstring data = "XXXXXXXXXXXXXXXX";
		data += ":";
		data += mNonce;
		data += ":";
		data += mCnonce;
		cdstring::size_type datalen = data.length();
		::memcpy(data.c_str_mod(), digest, 16);
		
		_H(digest, data, datalen);
	}
	cdstring HA1;
	HA1.reserve(32);
	_HEX(HA1, digest);
	
	// Determine HA2
	cdstring A2;
	A2 = request->GetMethod();
	A2 += ":";
	A2 += request->GetRURI();
	if (mQop == "auth-int")
	{
		// Ughh we do not do this right now
		// Hash the body
	}
	_H(digest, A2, A2.length());
	cdstring HA2;
	HA2.reserve(32);
	_HEX(HA2, digest);
	
	// Now do KD...
	cdstring sval;
	sval = mNonce;
	sval += ":";
	if (!mQop.empty())
	{
		sval += mNC;
		sval += ":";
		sval += mCnonce;
		sval += ":";
		sval += mQop;
		sval += ":";
	}
	sval += HA2;
	
	_KD(digest, HA1, sval);
	mResponse.clear();
	mResponse.reserve(32);
	_HEX(mResponse, digest);
}
