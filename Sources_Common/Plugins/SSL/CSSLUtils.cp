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


// Header for CSSLUtls.cp


#include "CSSLUtils.h"

#include "CGetPassphraseDialog.h"

#include "cdstring.h"

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/x509.h>

//using namespace NSSL;

// Specialisations

namespace NSSL
{

// ASN1_HEADER
template<> void StSSLObject<ASN1_HEADER>::delete_obj()
{
	::ASN1_HEADER_free(mObj);
}

// BIO
template<> void StSSLObject<BIO>::delete_obj()
{
	::BIO_free(mObj);
}

// BUF_MEM
template<> void StSSLObject<BUF_MEM>::delete_obj()
{
	::BUF_MEM_free(mObj);
}

// EVP_PKEY
template<> void StSSLObject<EVP_PKEY>::delete_obj()
{
	::EVP_PKEY_free(mObj);
}

// PKCS12
template<> void StSSLObject<PKCS12>::delete_obj()
{
	::PKCS12_free(mObj);
}

// STACK_OF(X509) - also frees the certs in the stack
template<> void StSSLObject<STACK_OF(X509), X509>::delete_obj()
{
	::sk_X509_pop_free(mObj, X509_free);
}

//  STACK_OF(X509_INFO) - also frees the infos in the stack
template<> void StSSLObject<STACK_OF(X509_INFO), X509_INFO>::delete_obj()
{
	::sk_X509_INFO_pop_free(mObj, X509_INFO_free);
}

// X509
template<> void StSSLObject<X509>::delete_obj()
{
	::X509_free(mObj);
}

// Error strings

void ERR_better_errors(cdstring& shorterrs, cdstring& longerrs)
{
	unsigned long l;
	const char* file;
	const char* data;
	int line;
	int flags;

	while ((l = ::ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
	{
		char buf[256];

		::ERR_error_string_n(l, buf, sizeof(buf));
		longerrs += buf;
		longerrs += ":";
		longerrs += file;
		longerrs += ":";
		longerrs += (unsigned long) line;
		longerrs += os_endl;

		shorterrs += ::ERR_reason_error_string(l);
		shorterrs += os_endl;
	}
}

// Passphrase callback
int PassphraseCallback(char* buf, int bufsiz, int verify, char* out)
{
	// Ask user for passphrase
	cdstring passphrase;
	cdstring chosen_user;
	unsigned long index = 0;
	CGetPassphraseDialog::PoseDialog(passphrase, NULL, chosen_user, index);

	int result = passphrase.length();
	if (result > bufsiz)
		result = bufsiz;	
	::memcpy(buf, passphrase.c_str(), result);
	::strncpy(out, passphrase.c_str(), 1024);
	
	return result;
}

}
