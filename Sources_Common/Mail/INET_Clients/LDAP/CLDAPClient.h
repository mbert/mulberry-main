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


// CLDAPClient

// Class that does LDAP lookups

#ifndef __CLDAPCLIENT__MULBERRY__
#define __CLDAPCLIENT__MULBERRY__

#include "CAddrLookupProtocol.h"
#include "CMailControl.h"

#include "openssl_.h"

extern "C" {
	int tls_verify_cb(int ok, X509_STORE_CTX *ctx);
}

// Classes
class CAddressList;

class CLDAPClient : public CAddrLookupProtocol
{
	friend int tls_verify_cb(int ok, X509_STORE_CTX *ctx);

public:
			CLDAPClient();
	virtual ~CLDAPClient();
		
	static int BlockingHook();

	virtual void Lookup(const cdstring& item,
						CAdbkAddress::EAddressMatch match,
						CAdbkAddress::EAddressField field, 
						CAddressList& addr_list);

	virtual bool HandleResult(int code);

private:
	typedef std::vector<CLDAPClient*>	CLDAPClientList;

	static CBusyContext*		sBusy;
	static CLDAPClientList		sClients;
	CBusyContext				mBusy;
	std::vector<int>					mCertErrors;
	

			void SetStatus(const char* rsrcid);

	int Verify(int ok, X509_STORE_CTX* ctx);
	void AddCertError(int err);
};

#endif
