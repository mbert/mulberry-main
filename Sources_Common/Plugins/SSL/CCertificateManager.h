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

// CCertificateManager.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 16-Mar-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate manager for SSL & SMIME use in Mulberry.
//
// History:
// CD:	 16-Mar-2003:	Created initial header and implementation.
//

#ifndef __CCERTIFICATEMANAGER__MULBERRY__
#define __CCERTIFICATEMANAGER__MULBERRY__

#include "CCertificateManagerCOM.h"

#include "ptrvector.h"

// Classes
class CCertificateStore;
typedef ptrvector<CCertificateStore> CCertificateStoreList;
class CCertificate;
typedef ptrvector<CCertificate> CCertificateList;

class CCertificateManager : public CCertificateManagerCOM
{
public:

	static CCertificateManager* sCertificateManager;

	// Constructor/destruct

	CCertificateManager();
	virtual ~CCertificateManager();

	static bool HasCertificateManager();

	// Certificate utilities
	void LoadCertificates();
	void ReloadCertificates();
	void ReloadCertificates(ECertificateType type);

	void LoadSSLRootCerts(SSL_CTX* ctx);

	void GetAllCertificatesInStores(ECertificateType type, CCertificateList& list);
	void GetPrivateCertificates(cdstrvect& subjs, cdstrvect& fingerprints);
	bool GetSubject(const cdstring& key, cdstring& result, ECertificateLookupType lookup = eAuto);
	bool GetHash(const cdstring& key, cdstring& result, ECertificateLookupType lookup = eAuto);
	bool GetFingerprint(const cdstring& key, cdstring& result, ECertificateLookupType lookup = eAuto);
	bool GetEmail(const cdstring& key, cdstrvect& result, ECertificateLookupType lookup = eAuto);

	bool ImportCertificateFile(CCertificateManager::ECertificateType type, const cdstring& fpath);
	bool ImportCertificateFile(CCertificateManager::ECertificateType type, const cdstring& fpath, int format);
	bool ImportCertificateFile(CCertificateManager::ECertificateType type, X509* xcert, EVP_PKEY* pkey, const cdstring* passphrase, STACK_OF(X509)* ca);
	bool ImportCertificateFile(CCertificateManager::ECertificateType type, const CCertificate* cert, const CCertificateList* cas);

	// SSL/server utilities
	bool CheckServerCertificate(const cdstring& server, std::vector<int>& errors, X509* server_cert);
	bool AcceptableServer(const cdstring& server, X509* server_cert, const std::vector<int>& errors, const cdstrvect& error_txt);
	bool CertificateToString(X509* server_cert, cdstring& txt);

	// SMIME utilities
	virtual void LoadSMIMERootCerts(X509_STORE* store);

	virtual cdstring FindCertificateFile(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto);
	virtual X509* FindCertificate(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto);
#ifdef USE_CMS
	virtual X509* FindCertificate(CMS_RECIP_INFO* ri, ECertificateType type);
#else
	virtual X509* FindCertificate(PKCS7_RECIP_INFO* ri, ECertificateType type);
#endif
	virtual EVP_PKEY* LoadPrivateKey(const char* key, const char* passphrase, ECertificateLookupType lookup = eAuto);

	virtual STACK_OF(X509)* GetCerts(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto);

	virtual bool CheckUserCertificate(const cdstring& email, std::vector<int>& errors, X509* user_cert);
	virtual bool AcceptableUser(const cdstring& email, X509* user_cert, const std::vector<int>& errors, const cdstrvect& error_txt);

private:
	// struct used to store accept certs and map
	struct SCertificateAccept
	{
		STACK_OF(X509)* mCertificates;
		
		SCertificateAccept();
		~SCertificateAccept();
	};

	// Class to manage accept & save/accept once cache
	class CAcceptDB
	{
	public:
		CAcceptDB(ECertificateType type) : mType(type) {}
		~CAcceptDB() {}
		
		void Init();

		void SetDBDirectory(const cdstring& dir);

		bool CertInCache(X509* cert, bool once) const;

		void WriteAcceptable(X509* cert) const;

		void WriteAcceptData() const;
		void ReadAcceptData();
		bool CheckAcceptData(const cdstring& name, const cdstring& fingerprint, bool once) const;
		void AddAcceptData(X509* cert, const cdstring& name, const cdstring& fingerprint, bool once);
			
	private:
		ECertificateType	mType;
		cdstring			mDir;
		cdstring			mPath;
		SCertificateAccept	mAcceptOnce;		// Acceptable server certificates for current session only
		cdstrset			mAcceptOnceData;	// Saved acceptable server certificates
		cdstrset			mAcceptSaveData;	// Saved acceptable server certificates
	};
	friend class CAcceptDB;

	CAcceptDB	mServerAccept;			// Acceptable server certificates
	CAcceptDB	mUserAccept;			// Acceptable user certificates

	CCertificateStoreList	mCAStore;
	CCertificateStoreList	mServerStore;
	CCertificateStoreList	mUserStore;
	CCertificateStoreList	mPersonalStore;

	bool		mLoadedCerts;			// Certificates loaded

	void CertHandleError(X509_STORE_CTX *ctx) const;

	X509* FindCertificateInStore(const CCertificateStoreList& store, const char* key, ECertificateLookupType lookup = eAuto) const;
#ifdef USE_CMS
	X509* FindCertificateInStore(const CCertificateStoreList& store, CMS_RECIP_INFO* ri) const;
#else
	X509* FindCertificateInStore(const CCertificateStoreList& store, PKCS7_RECIP_INFO* ri) const;
#endif

	const CCertificate* LookupCertificate(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto) const;
	void WriteCertificate(X509* cert, ECertificateType type) const;
	void WriteCertificate(X509* cert, const cdstring& dir) const;

	bool DoCertAltNameCheck(X509* cert, const cdstring& name, ECertificateType type);
};

#endif
