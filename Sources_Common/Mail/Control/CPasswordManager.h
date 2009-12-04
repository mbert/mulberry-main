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

#ifndef __CPASSWORDMANAGER__MULBERRY__
#define __CPASSWORDMANAGER__MULBERRY__

#include "cdstring.h"

class CINETAccount;

class CPasswordManager
{
public:
	static CPasswordManager* GetManager()
	{
		return sPasswordManager;
	}

	virtual bool GetPassword(const CINETAccount* acct, cdstring& pswd) = 0;
	virtual void AddPassword(const CINETAccount* acct, const cdstring& pswd) = 0;

protected:
	static CPasswordManager* sPasswordManager;

	CPasswordManager() {}
	virtual ~CPasswordManager() {}
};


#endif
