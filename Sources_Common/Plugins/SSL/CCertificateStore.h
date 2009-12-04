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

// CCertificateStore.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 14-Jun-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate store API for use in Mulberry.
//
// History:
// CD:	 14-Jun-2003:	Created initial header and implementation.
//

#ifndef __CCERTIFICATESTORE__MULBERRY__
#define __CCERTIFICATESTORE__MULBERRY__

#include "CCertificateManager.h"

#include "ptrvector.h"

// Classes
class CCertificate;
typedef ptrvector<CCertificate> CCertificateList;

class CCertificateStore
{
public:

	// Constructor/destruct

	CCertificateStore();
	virtual ~CCertificateStore();

	// Certificate utilities
	virtual	void Init() = 0;
	virtual	void Reload() = 0;
	virtual void LoadCertificate(CCertificate& cert) = 0;
	virtual void ImportCertificate(const CCertificate& cert) = 0;
	virtual bool DeleteCertificate(CCertificate& cert) = 0;
	virtual bool CanWrite() const = 0;
	virtual cdstring GetCertificateLocation(const CCertificate& cert) const = 0;

	void GetCertificates(CCertificateList& certs) const;
	void GetCertificates(cdstrvect& subjs, cdstrvect& fingerprints) const;

	bool GetSubject(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
	bool GetHash(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
	bool GetFingerprint(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
	bool GetDNS(const cdstring& key, cdstrvect& result, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
	bool GetEmail(const cdstring& key, cdstrvect& result, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;

	const CCertificate* FindCertificate(const cdstring& key, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
	void FindCertificates(const cdstring& key, CCertificateList& results, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto) const;
#ifdef USE_CMS
	const CCertificate* FindCertificate(const CMS_RECIP_INFO* ri) const;
#else
	const CCertificate* FindCertificate(const PKCS7_RECIP_INFO* ri) const;
#endif
	EVP_PKEY* LoadPrivateKey(const cdstring& key, const char* passphrase, CCertificateManager::ECertificateLookupType lookup = CCertificateManager::eAuto);

protected:
	CCertificateList	mCerts;

	virtual EVP_PKEY* LoadPrivateKey(const CCertificate& cert, const char* passphrase) const = 0;

};

typedef ptrvector<CCertificateStore> CCertificateStoreList;

#endif
