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

// CCertificate.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 14-Jun-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate for Mulberry.
//
// History:
// CD:	 14-Jun-2003:	Created initial header and implementation.
//

#include "CCertificate.h"

#include "CCertificateStore.h"
#include "CGeneralException.h"
#include "CLog.h"
#include "CSSLUtils.h"
#include "CURL.h"

#include <openssl/x509v3.h>

CCertificate::CCertificate(CCertificateStore* store, X509* cert, EVP_PKEY* pkey, const cdstring* passphrase) :
	mSubject(cdstring::null_str, false),
	mIssuer(cdstring::null_str, false),
	mValidityOK(false),
	mValidity(NULL),
	mHash(cdstring::null_str, false),
	mFingerprint(cdstring::null_str, false),
	mName(cdstring::null_str, false),
	mCert(cert),
	mPKey(pkey),
	mPassphrase(passphrase ? *passphrase : cdstring::null_str),
	mStore(store)
{
	mDNS.second = false;
	mEmail.second = false;

	// Always add reference to pkey/cert
	if (mCert)
		CRYPTO_add(&mCert->references, 1, CRYPTO_LOCK_X509);
	if (mPKey)
		CRYPTO_add(&mPKey->references, 1, CRYPTO_LOCK_EVP_PKEY);

	// Always cache data here if cert is provided
	CacheData();
}

CCertificate::CCertificate(CCertificateStore* store, const CCertificate& copy, bool add_ref) :
	mSubject(copy.mSubject),
	mIssuer(copy.mIssuer),
	mValidityOK(false),
	mValidity(NULL),
	mHash(copy.mHash),
	mFingerprint(copy.mFingerprint),
	mName(copy.mName),
	mDNS(copy.mDNS),
	mEmail(copy.mEmail),
	mCert(NULL),
	mPKey(NULL),
	mStore(store)
{
	if (add_ref)
	{
		if (copy.mCert)
		{
			mCert = copy.mCert;
			CRYPTO_add(&mCert->references, 1, CRYPTO_LOCK_X509);
		}
		if (copy.mPKey)
		{
			mPKey = copy.mPKey;
			CRYPTO_add(&mPKey->references, 1, CRYPTO_LOCK_EVP_PKEY);
		}
	}
}

CCertificate::~CCertificate()
{
	// Free what we loaded

	if (mPKey != NULL)
		::EVP_PKEY_free(mPKey);
	mPKey = NULL;

	if (mCert != NULL)
		::X509_free(mCert);
	mCert = NULL;

	if (mValidity)
		::X509_VAL_free(mValidity);
	mValidity = NULL;
}

// Compare function used for STL sort
bool CCertificate::SubjectCompare(const CCertificate* cert1, const CCertificate* cert2)
{
	return cert1->GetSubject() < cert2->GetSubject();
}

// Check for and load cert maager
X509* CCertificate::GetCertificate(bool add_ref) const
{
	// Need to load it if missing
	if ((mStore != NULL) && (mCert == NULL))
		mStore->LoadCertificate(const_cast<CCertificate&>(*this));

	// Bump up reference count on object if required
	if ((mCert != NULL) && add_ref)
		CRYPTO_add(&mCert->references, 1, CRYPTO_LOCK_X509);

	return mCert;
}

void CCertificate::SetCertificate(X509* cert)
{
	// Clear existing
	ClearCertificate();

	mCert = cert;

	// Always add reference to cert
	if (mCert)
		CRYPTO_add(&mCert->references, 1, CRYPTO_LOCK_X509);

	// Always cache data here if cert is provided
	CacheData();
}

void CCertificate::ClearCertificate()
{
	// Free what we loaded
	if (mCert != NULL)
		::X509_free(mCert);
	mCert = NULL;
}

// Check for and load cert maager
EVP_PKEY* CCertificate::GetPKey(bool add_ref) const
{
	// Bump up reference count on object if required
	if ((mPKey != NULL) && add_ref)
		CRYPTO_add(&mPKey->references, 1, CRYPTO_LOCK_EVP_PKEY);

	return mPKey;
}

void CCertificate::SetPKey(EVP_PKEY* pkey, const cdstring& passphrase)
{
	// Clear existing
	ClearPKey();

	mPKey = pkey;

	// Always add reference to cert
	if (mPKey)
		CRYPTO_add(&mPKey->references, 1, CRYPTO_LOCK_EVP_PKEY);
	
	SetPassphrase(passphrase);
}

void CCertificate::ClearPKey()
{
	// Free what we loaded
	if (mPKey != NULL)
		::EVP_PKEY_free(mPKey);
	mPKey = NULL;
	SetPassphrase(cdstring::null_str);
}

void CCertificate::CacheData()
{
	if (mCert == NULL)
		return;

	// Just load each item
	GetSubject();
	GetIssuer();
	GetValidity();
	GetHash();
	GetFingerprint();
	GetDNS();
	GetEmail();
}

void CCertificate::UncacheData()
{
	mSubject = cdstrbool(cdstring::null_str, false);
	mIssuer = cdstrbool(cdstring::null_str, false);
	mValidityOK = false;
	if (mValidity)
		::X509_VAL_free(mValidity);
	mValidity = NULL;
	mHash = cdstrbool(cdstring::null_str, false);
	mFingerprint = cdstrbool(cdstring::null_str, false);
	mName = cdstrbool(cdstring::null_str, false);
	mDNS.first.clear();
	mDNS.second = false;
	mEmail.first.clear();
	mEmail.second = false;
}

const cdstring& CCertificate::GetSubject() const
{
	// Check cached value first
	if (mSubject.second)
		return mSubject.first;

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return cdstring::null_str;

	// Get subject text
	char x509_buf[BUFSIZ];
	char* str = ::X509_NAME_oneline(::X509_get_subject_name(cert), x509_buf, BUFSIZ);
	if (str)
	{
		mSubject.first = str;
		mSubject.second = true;
		return mSubject.first;
	}
	else
		return cdstring::null_str;
}

const cdstring& CCertificate::GetIssuer() const
{
	// Check cached value first
	if (mIssuer.second)
		return mIssuer.first;

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return cdstring::null_str;

	// Get subject text
	char x509_buf[BUFSIZ];
	char* str = ::X509_NAME_oneline(::X509_get_issuer_name(cert), x509_buf, BUFSIZ);
	if (str)
	{
		mIssuer.first = str;
		mIssuer.second = true;
		return mIssuer.first;
	}
	else
		return cdstring::null_str;
}

const cdstrpair CCertificate::GetValidity() const
{
	// Check cached value first
	if (!mValidityOK)
	{
		// First make sure crt is cached
		X509* cert = GetCertificate();
		if (cert == NULL)
			return cdstrpair(cdstring::null_str, cdstring::null_str);

		// Make copy of times
		mValidity = ::X509_VAL_new();
		mValidity->notBefore = M_ASN1_TIME_dup(X509_get_notBefore(cert));
		mValidity->notAfter = M_ASN1_TIME_dup(X509_get_notAfter(cert));
		
		mValidityOK = true;
	}
	
	cdstring notBefore;
	cdstring notAfter;

	{
		// Create BIO for output to memeory
		NSSL::StSSLObject<BIO> out1(::BIO_new(::BIO_s_mem()));
		if (out1.get() == NULL)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Print X509 not before to BIO
		::ASN1_TIME_print(out1.get(), mValidity->notBefore);

		// Copy BIO into cdstring
		BUF_MEM* buf1 = NULL;
		::BIO_get_mem_ptr(out1.get(), &buf1);
		notBefore.assign(buf1->data, ::BIO_number_written(out1.get()));
		notBefore.ConvertToOS();
	}

	{
		// Create BIO for output to memeory
		NSSL::StSSLObject<BIO> out2(::BIO_new(::BIO_s_mem()));
		if (out2.get() == NULL)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Print X509 not before to BIO
		::ASN1_TIME_print(out2.get(), mValidity->notAfter);

		// Copy BIO into cdstring
		BUF_MEM* buf2 = NULL;
		::BIO_get_mem_ptr(out2.get(), &buf2);
		notAfter.assign(buf2->data, ::BIO_number_written(out2.get()));
		notAfter.ConvertToOS();
	}

	return cdstrpair(notBefore, notAfter);
}

bool CCertificate::IsNotBeforeValid() const
{
	if (!mValidityOK)
		GetValidity();
	return mValidityOK ? (::X509_cmp_current_time(mValidity->notBefore) < 0) : false;
}

bool CCertificate::IsNotAfterValid() const
{
	if (!mValidityOK)
		GetValidity();
	return mValidityOK ? (::X509_cmp_current_time(mValidity->notAfter) > 0) : false;
}

const cdstring& CCertificate::GetHash() const
{
	// Check cached value first
	if (mHash.second)
		return mHash.first;

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return cdstring::null_str;

	// Get the hash
	cdstring hash;
	hash.reserve(32);
	::sprintf(hash.c_str_mod(), "%08lx", ::X509_subject_name_hash(cert));

	mHash.first = hash;
	mHash.second = true;
	return mHash.first;
}

const cdstring& CCertificate::GetFingerprint() const
{
	// Check cached value first
	if (mFingerprint.second)
		return mFingerprint.first;

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return cdstring::null_str;

	// Need to make sure SHA1 is actually calculated
	// Can do this by compare to self
	::X509_cmp(cert, cert);

	// Now get hex form of SHA1
	cdstring fingerprint;
	fingerprint.reserve(2 * SHA_DIGEST_LENGTH);
	char* temp = fingerprint.c_str_mod();
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		*temp++ = cHexChar[cert->sha1_hash[i] >> 4];
		*temp++ = cHexChar[cert->sha1_hash[i] & 0x0F];
	}
	*temp = 0;
	
	mFingerprint.first = fingerprint;
	mFingerprint.second = true;
	return mFingerprint.first;
}

const cdstring& CCertificate::GetName() const
{
	// Check cached value first
	if (mName.second)
		return mName.first;

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return cdstring::null_str;

	cdstring data;
	X509_NAME* xn = ::X509_get_subject_name(cert);

	char x509_buf[BUFSIZ];
	if ((::X509_NAME_get_text_by_NID(xn, NID_commonName, x509_buf, sizeof(x509_buf)) != -1) && (x509_buf[0] != 0))
		data = x509_buf;
	else if ((::X509_NAME_get_text_by_NID(xn, NID_organizationalUnitName, x509_buf, sizeof(x509_buf)) != -1) && (x509_buf[0] != 0))
		data = x509_buf;
	else if ((::X509_NAME_get_text_by_NID(xn, NID_organizationName, x509_buf, sizeof(x509_buf)) != -1) && (x509_buf[0] != 0))
		data = x509_buf;
	else
		data = GetSubject();

	mName.first = data;
	mName.second = true;
	return mName.first;
}

const cdstrvect& CCertificate::GetDNS() const
{
	// Check cached value first
	if (!mDNS.second)
	{
		GetNIDs(GEN_DNS, NID_commonName, mDNS.first);
		mDNS.second = true;
	}
	return mDNS.first;
}

const cdstrvect& CCertificate::GetEmail() const
{
	// Check cached value first
	if (!mEmail.second)
	{
		GetNIDs(GEN_EMAIL, NID_pkcs9_emailAddress, mEmail.first);
		mEmail.second = true;
	}
	return mEmail.first;
}

cdstring CCertificate::GetNID(int gen_type, int nid) const
{
	cdstrvect temp;
	cdstring data;
	GetNIDs(gen_type, nid, temp);
	if (temp.size() != 0)
		data = temp.front();
	
	return data;
}

void CCertificate::GetNIDs(int gen_type, int nid, cdstrvect& results) const
{
	results.clear();

	// First make sure crt is cached
	X509* cert = GetCertificate();
	if (cert == NULL)
		return;

	bool result = false;

	// Look for subject alt name
	int i = ::X509_get_ext_by_NID(cert, NID_subject_alt_name, -1);
	if (i >= 0)
	{
		X509_EXTENSION* ex = ::X509_get_ext(cert, i);
		STACK_OF(GENERAL_NAME)* alt = (STACK_OF(GENERAL_NAME)*) ::X509V3_EXT_d2i(ex);
		if (alt)
		{
			// Look at each item
			int n = sk_GENERAL_NAME_num(alt);
			for(i = 0; i < n; i++)
			{
				GENERAL_NAME* gn = sk_GENERAL_NAME_value(alt, i);
				if (gn->type == gen_type)
				{
					cdstring sn((char*)ASN1_STRING_data(gn->d.ia5), ASN1_STRING_length(gn->d.ia5));
					
					results.push_back(sn);
					result = true;
				}
			}

			GENERAL_NAMES_free(alt);
		}
	}

	// If no subject alt name, use actual subject and extract email from that
	if (!result)
	{
		X509_NAME* xn = ::X509_get_subject_name(cert);

		char x509_buf[BUFSIZ];
		if (::X509_NAME_get_text_by_NID(xn, nid, x509_buf, sizeof(x509_buf)) != -1)
		{
			results.push_back(x509_buf);
			result = true;
		}
	}
}

// Print cert as string
cdstring CCertificate::StringCert() const
{
	cdstring result;

	// First make sure crt is cached
	X509* x = GetCertificate();
	if (x == NULL)
		return cdstring::null_str;

	// Create BIO for output to memeory
	NSSL::StSSLObject<BIO> out(::BIO_new(::BIO_s_mem()));
	if (out.get() == NULL)
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}

	// Print X509 to BIO
	::X509_print(out.get(), x);

	// Copy BIO into cdstring
	BUF_MEM* buf = NULL;
	::BIO_get_mem_ptr(out.get(), &buf);
	result.assign(buf->data, ::BIO_number_written(out.get()));
	result.ConvertToOS();
	
	return result;
}

// Get location from store
cdstring CCertificate::GetStoreLocation() const
{
	if (mStore)
		return mStore->GetCertificateLocation(*this);
	else
		return cdstring::null_str;
}
