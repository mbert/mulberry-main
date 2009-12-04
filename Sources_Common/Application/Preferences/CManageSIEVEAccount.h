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


// CManageSIEVEAccount.h

// Header file for class/structs that define ManageSIEVE accounts

#ifndef __CMANAGESIEVEACCOUNT__MULBERRY__
#define __CMANAGESIEVEACCOUNT__MULBERRY__

#include "CINETAccount.h"

#include "CFutureItems.h"

// Classes

class CManageSIEVEAccount : public CINETAccount
{
public:
			CManageSIEVEAccount();
			CManageSIEVEAccount(const CManageSIEVEAccount& copy) : CINETAccount(copy)
				{ _copy(copy); }
	virtual	~CManageSIEVEAccount() {}

	CManageSIEVEAccount& operator=(const CManageSIEVEAccount& copy)				// Assignment with same type
		{ if (this != &copy) { CINETAccount::operator=(copy); _copy(copy); } return *this; }
	int operator==(const CManageSIEVEAccount& comp) const;							// Compare with same type

	// Getters/setters

	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	CFutureItems		mFuture;

private:
	void _copy(const CManageSIEVEAccount& copy);
};

typedef ptrvector<CManageSIEVEAccount> CManageSIEVEAccountList;

#endif
