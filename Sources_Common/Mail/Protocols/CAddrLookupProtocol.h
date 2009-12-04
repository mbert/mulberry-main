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


// CAddrLookupProtocol

// Virtual base class that represents an address lookup server

#ifndef __CADDRLOOKUPPROTOCOL__MULBERRY__
#define __CADDRLOOKUPPROTOCOL__MULBERRY__

#include "cdstring.h"
#include "CINETAccount.h"
#include "CAddressBookManager.h"

// Classes
class CAddressList;

class CAddrLookupProtocol
{
public:
			CAddrLookupProtocol() {}
	virtual ~CAddrLookupProtocol() {}
		
	virtual const CAddressAccount* GetAccount() const
		{ return mAccount; }
	virtual CAddressAccount* GetAccount()
		{ return mAccount; }
	virtual void SetAccount(CAddressAccount* acct)
		{ mAccount = acct; }

	virtual void Lookup(const cdstring& item,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field,
						CAddressList& addr_list) = 0;

protected:
	CAddressAccount* mAccount;						// Account info
};

#endif
