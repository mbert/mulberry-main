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

// CCertificate.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 14-Jun-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate for use in Mulberry.
//
// History:
// CD:	 14-Jun-2003:	Created initial header and implementation.
//

#ifndef __CCERTIFICATE__MULBERRY__
#define __CCERTIFICATE__MULBERRY__

#include "CCertificateManager.h"

// Classes
class CCertificateStore;

class CCertificate
{
public:

	typedef cdstring (CCertificate::*StringInfo)() const;

	// find_if functors
	class find_if
	{
	public:
		find_if(const cdstring& data, StringInfo func) :
			mData(data), mFunc(func) {}

		bool operator() (const CCertificate* item) const
			{ return item ? ((item->*mFunc)() == mData) : false; }
	private:
		cdstring mData;
		StringInfo mFunc;
	};

	// Construct/destruct

	CCertificate(CCertificateStore* store, X509* cert = NULL, EVP_PKEY* pkey = NULL, const cdstring* passphrase  = NULL);
	CCertificate(CCertificateStore* store, const CCertificate& copy, bool add_ref = false);
	~CCertificate();

	X509* GetCertificate(bool add_ref = false) const;
	void SetCertificate(X509* cert);
	void ClearCertificate();

	bool HasPKey() const
		{ return GetPKey() != NULL; }
	EVP_PKEY* GetPKey(bool add_ref = false) const;
	void SetPKey(EVP_PKEY* pkey, const cdstring& passphrase);
	void ClearPKey();

	void SetPassphrase(const cdstring& passphrase)
		{ mPassphrase = passphrase; }
	const cdstring& GetPassphrase() const
		{ return mPassphrase; }

	CCertificateStore* GetStore() const
		{ return mStore; }

	// Get items from cert
	void		CacheData();
	void		UncacheData();
	const cdstring&		GetSubject() const;
	const cdstring&		GetIssuer() const;
	const cdstrpair		GetValidity() const;
	bool				IsNotBeforeValid() const;
	bool				IsNotAfterValid() const;
	const cdstring&		GetHash() const;
	const cdstring&		GetFingerprint() const;
	const cdstring&		GetName() const;
	const cdstrvect&	GetDNS() const;
	const cdstrvect&	GetEmail() const;

	cdstring StringCert() const;
	
	cdstring GetStoreLocation() const;

	static bool SubjectCompare(const CCertificate*, const CCertificate*);

private:
	typedef std::pair<cdstrvect, bool>	cdstrvectbool;

	mutable cdstrbool		mSubject;
	mutable cdstrbool		mIssuer;
	mutable bool			mValidityOK;
	mutable X509_VAL*		mValidity;
	mutable cdstrbool		mHash;
	mutable cdstrbool		mFingerprint;
	mutable cdstrbool		mName;
	mutable cdstrvectbool	mDNS;
	mutable cdstrvectbool	mEmail;
	X509*					mCert;
	EVP_PKEY*				mPKey;
	cdstring				mPassphrase;
	CCertificateStore*		mStore;
	
	cdstring GetNID(int gen_type, int nid) const;
	void GetNIDs(int gen_type, int nid, cdstrvect& results) const;
};

typedef ptrvector<CCertificate> CCertificateList;

#endif
