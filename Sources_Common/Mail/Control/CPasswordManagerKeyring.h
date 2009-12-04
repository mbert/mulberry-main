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
	Class to handle a password keyring file.
*/

#ifndef __CPASSWORDMANAGERKEYRING__MULBERRY__
#define __CPASSWORDMANAGERKEYRING__MULBERRY__

#include "CPasswordManager.h"

class CINETAccount;

class CPasswordManagerKeyring : public CPasswordManager
{
public:
	static CPasswordManagerKeyring* GetKeyRingManager()
	{
		return static_cast<CPasswordManagerKeyring*>(sPasswordManager);
	}

	static void MakePasswordManagerKeyring();

	void SetPassphrase(const cdstring& passphrase)
	{
		mPassphrase = passphrase;
		mActive = true;
	}
	bool KeyringExists() const;

	virtual bool GetPassword(const CINETAccount* acct, cdstring& pswd);
	virtual void AddPassword(const CINETAccount* acct, const cdstring& pswd);

private:
	cdstring	mKeyringPath;
	cdstring	mPassphrase;
	bool		mActive;

	CPasswordManagerKeyring();
	virtual ~CPasswordManagerKeyring();

	cdstring GetAccountURL(const CINETAccount* acct) const;
	cdstrmap ReadEncryptedMap() const;
	void WriteEncryptedMap(const cdstrmap& pswds) const;

};


#endif
