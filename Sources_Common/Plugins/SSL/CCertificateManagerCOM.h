/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

// CCertificateManagerCOM.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 21-Mar-2003
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements an X509 certificate manager for SSL & SMIME use in Mulberry.
//
// History:
// CD:	 21-Mar-2003:	Created initial header and implementation.
//

#ifndef __CCERTIFICATEMANAGERCOM__MULBERRY__
#define __CCERTIFICATEMANAGERCOM__MULBERRY__

#include "openssl_.h"

//#if __dest_os == __win32_os
//#define USE_CMS
//#endif

#ifdef USE_CMS
#include <openssl/cms.h>
#endif

#include "cdstring.h"

// Classes

class CCertificateManagerCOM
{
public:
	enum ECertificateLookupType
	{
		eAuto = 0,
		eByEmail,
		eBySubject,
		eByHash,
		eByFingerprint
	};

	enum ECertificateType
	{
		eNone = 0,
		eCACertificates,
		eServerCertificates,
		eUserCertificates,
		ePersonalCertificates
	};

	// SMIME utilities
	virtual void LoadSMIMERootCerts(X509_STORE* store) = 0;

	virtual cdstring FindCertificateFile(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto) = 0;
	virtual X509* FindCertificate(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto) = 0;
#ifdef USE_CMS
	virtual X509* FindCertificate(CMS_RECIP_INFO* ri, ECertificateType type) = 0;
#else
	virtual X509* FindCertificate(PKCS7_RECIP_INFO* ri, ECertificateType type) = 0;
#endif
	virtual EVP_PKEY* LoadPrivateKey(const char* key, const char* passphrase, ECertificateLookupType lookup = eAuto) = 0;
	
	virtual STACK_OF(X509)* GetCerts(const char* key, ECertificateType type, ECertificateLookupType lookup = eAuto) = 0;

	virtual bool CheckUserCertificate(const cdstring& server, std::vector<int>& errors, X509* server_cert) = 0;
	virtual bool AcceptableUser(const cdstring& server, X509* server_cert, const std::vector<int>& errors, const cdstrvect& error_txt) = 0;

protected:

	// Constructor/destruct

	CCertificateManagerCOM() {}
	virtual ~CCertificateManagerCOM() {}
};

#endif
