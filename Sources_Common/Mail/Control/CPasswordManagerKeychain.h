/*
    Copyright (c) 2009 Cyrus Daboo. All rights reserved.
    
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

// CPasswordManager.h

/*
	Class to handle a password keychain.
*/

#ifndef __CPASSWORDMANAGERKEYCHAIN__MULBERRY__
#define __CPASSWORDMANAGERKEYCHAIN__MULBERRY__

#include "CPasswordManager.h"

#if __dest_os == __mac_os_x
#include <Security/Security.h>
#else
#error Only available on Mac OS X
#endif

class CPasswordManagerKeychain : public CPasswordManager
{
public:
	static void MakePasswordManagerKeychain();

	virtual bool GetPassword(const CINETAccount* acct, cdstring& pswd);
	virtual void AddPassword(const CINETAccount* acct, const cdstring& pswd);

private:
	CPasswordManagerKeychain();
	virtual ~CPasswordManagerKeychain();

	void GetSecurityDetails(const CINETAccount* acct, cdstring& server, cdstring& aname, UInt16& port, SecProtocolType& protocol, SecAuthenticationType& authenticationType);
	bool GetPasswordWithItem(const CINETAccount* acct, cdstring& pswd, SecKeychainItemRef* item);

};


#endif
