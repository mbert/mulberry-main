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
	CHTTPAuthorizationDigest.h

	Author:			
	Description:	<describe the CHTTPAuthorizationDigest class here>
*/

#ifndef CHTTPAuthorizationDigest_H
#define CHTTPAuthorizationDigest_H

#include "CHTTPAuthorization.h"

#include "cdstring.h"


namespace http {

class CHTTPAuthorizationDigest: public CHTTPAuthorization
{
public:
	CHTTPAuthorizationDigest()
	{
		mStale = false;
		mClientCount = 0;
	}
	CHTTPAuthorizationDigest(const cdstring& user, const cdstring& pswd, const cdstrvect& www_authenticate)
	{
		mUser = user;
		mPswd = pswd;
		ParseAuthenticateHeader(www_authenticate);
		mStale = false;
		mClientCount = 0;
	}
	virtual ~CHTTPAuthorizationDigest() {}

	void SetDetails(const cdstring& user, const cdstring& pswd, const cdstrvect& www_authenticate)
	{
		mUser = user;
		mPswd = pswd;
		ParseAuthenticateHeader(www_authenticate);
	}

	virtual void GenerateAuthorization(std::ostream& os, const CHTTPRequestResponse* request) const;

private:
	cdstring		mUser;
	cdstring		mPswd;
	cdstring		mWWWAuthenticate;
	cdstring		mRealm;
	cdstring		mDomain;
	cdstring		mNonce;
	cdstring		mOpaque;
	bool			mStale;
	cdstring		mAlgorithm;
	cdstring		mQop;
	cdstring		mNC;
	unsigned long	mClientCount;
	cdstring		mCnonce;
	cdstring		mResponse;

	void ParseAuthenticateHeader(const cdstrvect& hdrs);
	void GenerateResponse(const CHTTPRequestResponse* request);
};

}	// namespace http

#endif	// CHTTPAuthorizationDigest_H
