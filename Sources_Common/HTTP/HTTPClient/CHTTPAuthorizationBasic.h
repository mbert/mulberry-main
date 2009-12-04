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
	CHTTPAuthorizationBasic.h

	Author:			
	Description:	<describe the CHTTPAuthorizationBasic class here>
*/

#ifndef CHTTPAuthorizationBasic_H
#define CHTTPAuthorizationBasic_H

#include "CHTTPAuthorization.h"

#include "cdstring.h"


namespace http {

class CHTTPAuthorizationBasic: public CHTTPAuthorization
{
public:
	CHTTPAuthorizationBasic() {}
	CHTTPAuthorizationBasic(const cdstring& user, const cdstring& pswd)
	{
		mUser = user;
		mPswd = pswd;
	}
	virtual ~CHTTPAuthorizationBasic() {}

	void SetDetails(const cdstring& user, const cdstring& pswd)
	{
		mUser = user;
		mPswd = pswd;
	}

	virtual void GenerateAuthorization(std::ostream& os, const CHTTPRequestResponse* request) const;

private:
	cdstring	mUser;
	cdstring	mPswd;
};

}	// namespace http

#endif	// CHTTPAuthorizationBasic_H
