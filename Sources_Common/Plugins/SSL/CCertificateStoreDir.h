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

// CCertificateStoreDir.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 14-Jun-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate store in a directory for use in Mulberry.
//
// History:
// CD:	 14-Jun-2003:	Created initial header and implementation.
//

#ifndef __CCERTIFICATESTOREDIR__MULBERRY__
#define __CCERTIFICATESTOREDIR__MULBERRY__

#include "CCertificateStore.h"

// Classes
class CCertificateStoreDir : public CCertificateStore
{
public:

	// Constructor/destruct

	CCertificateStoreDir(const cdstring& dir, bool pem = true, bool read_only = false);
	virtual ~CCertificateStoreDir();

	// Certificate utilities
	virtual	void Init();
	virtual	void Reload();
	virtual void LoadCertificate(CCertificate& cert);
	virtual void ImportCertificate(const CCertificate& cert);
	virtual bool DeleteCertificate(CCertificate& cert);
	virtual bool CanWrite() const;
	virtual cdstring GetCertificateLocation(const CCertificate& cert) const;

private:
	cdstring	mDirectory;
	cdstrmap	mCertsMap;
	bool		mPEMFormat;
	bool		mReadOnly;

	virtual EVP_PKEY* LoadPrivateKey(const CCertificate& cert, const char* passphrase) const;

	void ReadAllCertificates();
	void ReadCertificate(const cdstring& fpath);
	X509* ReadCertificateFile(const cdstring& fpath) const;
	bool WriteCertificateFile(const CCertificate& cert, cdstring& fpath) const;
	
	cdstring FindCertFile(const CCertificate& cert) const;
};

#endif
