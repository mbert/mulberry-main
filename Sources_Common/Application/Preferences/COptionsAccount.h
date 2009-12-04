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


// COptionsAccount.h

// Header file for class/structs that define address book accounts

#ifndef __COPTIONSACCOUNT__MULBERRY__
#define __COPTIONSACCOUNT__MULBERRY__

#include "CINETAccount.h"
#include "ptrvector.h"

// Classes
class COptionsAccount;
typedef ptrvector<COptionsAccount> COptionsAccountList;

class COptionsAccount : public CINETAccount
{
public:
			COptionsAccount();
			COptionsAccount(const COptionsAccount& copy) : CINETAccount(copy)
				{ _copy(copy); }
	virtual ~COptionsAccount();

			COptionsAccount& operator=(const COptionsAccount& copy)					// Assignment with same type
				{ if (this != &copy) {CINETAccount::operator=(copy); _copy(copy);} return *this; }
	int operator==(const COptionsAccount& comp) const;								// Compare with same type

	// Getters/setters
	virtual void SetServerType(EINETServerType type);

	const cdstring& GetBaseRURL() const
	{
		return mBaseRURL;
	}
	void SetBaseRURL(const cdstring& rurl)
	{
		mBaseRURL = rurl;
	}

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	cdstring				mBaseRURL;
	
	void _copy(const COptionsAccount& copy);
};

#endif
