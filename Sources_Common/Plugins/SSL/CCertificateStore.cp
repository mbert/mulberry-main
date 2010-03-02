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

// CCertificateStore.cp
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

#include "CCertificateStore.h"

#include "CCertificate.h"
#include "CStringUtils.h"

#include <openssl/x509v3.h>

CCertificateStore::CCertificateStore()
{
}

CCertificateStore::~CCertificateStore()
{
}

void CCertificateStore::GetCertificates(CCertificateList& certs) const
{
	// Add descriptors forcerts to each list
	for (CCertificateList::const_iterator iter = mCerts.begin(); iter != mCerts.end(); iter++)
		certs.push_back(*iter);
}

void CCertificateStore::GetCertificates(cdstrvect& subjs, cdstrvect& fingerprints) const
{
	// Add descriptors forcerts to each list
	for (CCertificateList::const_iterator iter = mCerts.begin(); iter != mCerts.end(); iter++)
	{
		subjs.push_back((*iter)->GetSubject());
		fingerprints.push_back((*iter)->GetFingerprint());
	}
}

bool CCertificateStore::GetSubject(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup) const
{
	// Look at each cached user certificate
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert != NULL)
	{
		result = cert->GetSubject();
		return true;
	}

	return false;
}

bool CCertificateStore::GetHash(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup) const
{
	// Look at each cached user certificate
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert != NULL)
	{
		result = cert->GetHash();
		return true;
	}

	return false;
}

bool CCertificateStore::GetFingerprint(const cdstring& key, cdstring& result, CCertificateManager::ECertificateLookupType lookup) const
{
	// Look at each cached user certificate
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert != NULL)
	{
		result = cert->GetFingerprint();
		return true;
	}

	return false;
}

bool CCertificateStore::GetDNS(const cdstring& key, cdstrvect& result, CCertificateManager::ECertificateLookupType lookup) const
{
	// Look at each cached user certificate
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert != NULL)
	{
		result = cert->GetDNS();
		return true;
	}

	return false;
}

bool CCertificateStore::GetEmail(const cdstring& key, cdstrvect& result, CCertificateManager::ECertificateLookupType lookup) const
{
	// Look at each cached user certificate
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert != NULL)
	{
		result = cert->GetEmail();
		return true;
	}

	return false;
}

const CCertificate* CCertificateStore::FindCertificate(const cdstring& key, CCertificateManager::ECertificateLookupType lookup) const
{
	int test = 3;
	cdstring use_key = key;
	switch(lookup)
	{
	case CCertificateManager::eAuto:
	default:
		// Determine type of match to do:
		// 0 hash :			starts with #
		// 1 fingerprint :	starts with $
		// 2 subject :		starts with cn=
		// 3 email :		whatever is left
		if (key[(cdstring::size_type)0] == '#')
		{
			test = 0;
			use_key = key.c_str() + 1;
		}
		else if (key[(cdstring::size_type)0] == '$')
		{
			test = 1;
			use_key = key.c_str() + 1;
		}
		else if (!::strncmpnocase(key, "cn=", 3))
			test = 2;
		else
			test = 3;
		break;
	case CCertificateManager::eByEmail:
		test = 3;
		break;
	case CCertificateManager::eBySubject:
		test = 2;
		break;
	case CCertificateManager::eByHash:
		test = 0;
		break;
	case CCertificateManager::eByFingerprint:
		test = 1;
		break;
	}
	
	// Look at each cached certificate - return the one that is the most up to date
	int i = 0;
	const CCertificate* cert = NULL;
	for (CCertificateList::const_iterator iter = mCerts.begin(); iter != mCerts.end(); iter++, i++)
	{
		// Do test
		bool matches = false;
		switch(test)
		{
		case 0:	// hash
			if (::strpmatch((*iter)->GetHash(), use_key))
				matches = true;
			break;
		
		case 1: // fingerprint
			if (::strpmatch((*iter)->GetFingerprint(), use_key))
				matches = true;
			break;
		
		case 2: // subject
			if (::strpmatch((*iter)->GetSubject(), use_key))
				matches = true;
			break;
		
		default:	// email
			for(cdstrvect::const_iterator iter2 = (*iter)->GetEmail().begin(); iter2 != (*iter)->GetEmail().end(); iter2++)
			{
				if (::strpmatch(*iter2, use_key))
				{
					matches = true;
					break;
				}
			}
			break;
		}
		
		if (matches)
		{
			// If cert is completely valid, return it
			if ((*iter)->IsNotBeforeValid() && (*iter)->IsNotBeforeValid())
			{
				cert = *iter;
				break;
			}
			
			// Cache current cert if another invalid one is not already cached
			else if (!cert)
				cert = *iter;
		}
	}

	return cert;
}

void CCertificateStore::FindCertificates(const cdstring& key, CCertificateList& results, CCertificateManager::ECertificateLookupType lookup) const
{
	int test = 3;
	cdstring use_key = key;
	switch(lookup)
	{
	case CCertificateManager::eAuto:
	default:
		// Determine type of match to do:
		// 0 hash :			starts with #
		// 1 fingerprint :	starts with $
		// 2 subject :		starts with cn=
		// 3 email :		whatever is left
		if (key[(cdstring::size_type)0] == '#')
		{
			test = 0;
			use_key = key.c_str() + 1;
		}
		else if (key[(cdstring::size_type)0] == '$')
		{
			test = 1;
			use_key = key.c_str() + 1;
		}
		else if (!::strncmpnocase(key, "cn=", 3))
			test = 2;
		else
			test = 3;
		break;
	case CCertificateManager::eByEmail:
		test = 3;
		break;
	case CCertificateManager::eBySubject:
		test = 2;
		break;
	case CCertificateManager::eByHash:
		test = 0;
		break;
	case CCertificateManager::eByFingerprint:
		test = 1;
		break;
	}
	
	// Look at each cached certificate
	int i = 0;
	for (CCertificateList::const_iterator iter = mCerts.begin(); iter != mCerts.end(); iter++, i++)
	{
		// Do test
		switch(test)
		{
		case 0:	// hash
			if (::strpmatch((*iter)->GetHash(), use_key))
				results.push_back(*iter);
			break;
		
		case 1: // fingerprint
			if (::strpmatch((*iter)->GetFingerprint(), use_key))
				results.push_back(*iter);
			break;
		
		case 2: // subject
			if (::strpmatch((*iter)->GetSubject(), use_key))
				results.push_back(*iter);
			break;
		
		default:	// email
			for(cdstrvect::const_iterator iter2 = (*iter)->GetEmail().begin(); iter2 != (*iter)->GetEmail().end(); iter2++)
			{
				if (::strpmatch(*iter2, use_key))
				{
					results.push_back(*iter);
					break;
				}
			}
			break;
		}
	}
}

#ifdef USE_CMS
const CCertificate* CCertificateStore::FindCertificate(const CMS_RECIP_INFO* ri) const
#else
const CCertificate* CCertificateStore::FindCertificate(const PKCS7_RECIP_INFO* ri) const
#endif
{
	// Scan all cert file names looking for a match
	int i = 0;
	for (CCertificateList::const_iterator iter = mCerts.begin(); iter != mCerts.end(); iter++, i++)
	{
		// Compare with recipient info
#ifdef USE_CMS
		if (::CMS_RECIP_INFO_contains(ri, (*iter)->GetCertificate()))
			return *iter;
#else
		if(!::X509_NAME_cmp(ri->issuer_and_serial->issuer, (*iter)->GetCertificate()->cert_info->issuer) &&
		     !::M_ASN1_INTEGER_cmp((*iter)->GetCertificate()->cert_info->serialNumber, ri->issuer_and_serial->serial))
		{
			return *iter;
		}
#endif
	}
	
	return NULL;
}

EVP_PKEY* CCertificateStore::LoadPrivateKey(const cdstring& key, const char* passphrase, CCertificateManager::ECertificateLookupType lookup)
{
	// See if certificate exists
	const CCertificate* cert = FindCertificate(key, lookup);
	if (cert)
		return LoadPrivateKey(*cert, passphrase);
	else
		return NULL;
}
