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

// CCertificateStoreDir.cp
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

#include "CCertificateStoreDir.h"

#include "CCertificate.h"
#include "CGeneralException.h"
#include "CLocalCommon.h"
#include "CLog.h"
#include "CSSLUtils.h"
#include "CStringUtils.h"
#include "CUnicodeStdLib.h"
#include "CURL.h"

#include "diriterator.h"

#include <algorithm>

#define FORMAT_UNDEF    0
#define FORMAT_ASN1     1
#define FORMAT_TEXT     2
#define FORMAT_PEM      3
#define FORMAT_NETSCAPE 4
#define FORMAT_PKCS12   5
#define FORMAT_SMIME    6

CCertificateStoreDir::CCertificateStoreDir(const cdstring& dir, bool pem, bool read_only) :
	mDirectory(dir), mPEMFormat(pem), mReadOnly(read_only)
{
}

CCertificateStoreDir::~CCertificateStoreDir()
{
}

void CCertificateStoreDir::Init()
{
	// Just read entire directory
	ReadAllCertificates();
}

void CCertificateStoreDir::Reload()
{
	// Just read entire directory
	ReadAllCertificates();
}

// Load X509 certificate data into cert
// Assumes certificate object has a valid fingerprint already stored after descovery
void CCertificateStoreDir::LoadCertificate(CCertificate& cert)
{
	try
	{
		// Determine file path for certificate
		cdstring fpath = FindCertFile(cert);
		if (fpath.empty())
			return;

		// Read cert from file
		NSSL::StSSLObject<X509> x(ReadCertificateFile(fpath));
		cert.SetCertificate(x.get());
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CCertificateStoreDir::ImportCertificate(const CCertificate& cert)
{
	// Double check its not already in the store
	if (FindCertificate(cert.GetFingerprint(), CCertificateManager::eByFingerprint) != NULL)
		return;

	// Write X509 certificate to file
	cdstring fpath;
	if (WriteCertificateFile(cert, fpath))
	{
		// Add to cache
		CCertificate* new_cert = new CCertificate(this, cert);
		mCerts.push_back(new_cert);
		mCertsMap.insert(cdstrmap::value_type(new_cert->GetFingerprint(), fpath));
	}
}

bool CCertificateStoreDir::DeleteCertificate(CCertificate& cert)
{
	bool result = false;
	try
	{
		// Determine file path for certificate
		cdstring fpath = FindCertFile(cert);
		if (fpath.empty())
			return false;
		
		// Remove actual file
		result = (::remove_utf8(fpath) == 0);
		
		// Remove certificate from list
		if (result)
		{
			mCertsMap.erase(cert.GetFingerprint());
			CCertificateList::iterator found = std::find(mCerts.begin(), mCerts.end(), &cert);
			mCerts.erase(found);
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return result;
}

bool CCertificateStoreDir::CanWrite() const
{
	return !mReadOnly && ::dirreadwriteable(mDirectory);
}

cdstring CCertificateStoreDir::GetCertificateLocation(const CCertificate& cert) const
{
	cdstring result = cFileURLScheme;
	cdstring fpath = FindCertFile(cert);
	if (!fpath.empty())
		result += fpath;
	else
		result += mDirectory;
	
	return result;
}

EVP_PKEY* CCertificateStoreDir::LoadPrivateKey(const CCertificate& cert, const char* passphrase) const
{
	NSSL::StSSLObject<EVP_PKEY> pkey;
	int format = FORMAT_PEM;

	try
	{
		// Get cert file for key
		cdstring file = FindCertFile(cert);

		// Must have file
		if (file.empty())
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Create key file
		NSSL::StSSLObject<BIO> bkey(::BIO_new(::BIO_s_file()));
		if (bkey.get() == NULL)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Read the file data
		if (::BIO_read_filename(bkey.get(), file) <= 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Determine format and read in the private key data
		if (format == FORMAT_PEM)
		{
			pkey.reset(::PEM_read_bio_PrivateKey(bkey.get(), NULL, NULL, (void*) passphrase));
		}
		// Only do PEM for now
		else
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
	}
	catch(...)
	{
	}
	
	// Return key
	return pkey.release();
}

// Read all certificates found in directory
void CCertificateStoreDir::ReadAllCertificates()
{
	// Clear out existing data
	mCerts.clear();
	mCertsMap.clear();

	// Find all pem files in directory
	try
	{
		// Check that directory exists (create if not)
		if (!::direxists(mDirectory))
			return;

		// Iterate over all .pem files in directory
		diriterator iter(mDirectory, mPEMFormat ? ".pem" : NULL);
		const char* p = NULL;
		while(iter.next(&p))
		{
			cdstring fullpath = mDirectory;
			::addtopath(fullpath, p);
			ReadCertificate(fullpath);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Read certificate from a file and setup data structures
void CCertificateStoreDir::ReadCertificate(const cdstring& fpath)
{
	try
	{
		// Read cert from file
		NSSL::StSSLObject<X509> x(ReadCertificateFile(fpath));

		// Add the cert
		if (x.get() != NULL)
		{
			// Initialise cert lookup structure
			CCertificate* cert = new CCertificate(this, x.get());

			// Add to lists
			mCerts.push_back(cert);
			mCertsMap.insert(cdstrmap::value_type(cert->GetFingerprint(), fpath));
			
			// Don't cache certificate during descovery
			cert->ClearCertificate();
			
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Read certificate from a file
X509* CCertificateStoreDir::ReadCertificateFile(const cdstring& fpath) const
{
	// Open the file
	FILE* fp = ::fopen_utf8(fpath, "r");
	if (!fp)
		return NULL;

	// Read pem cert
	X509* x = ::PEM_read_X509(fp, NULL, NULL, NULL);

	// Close
	::fclose(fp);
	
	return x;
}

bool CCertificateStoreDir::WriteCertificateFile(const CCertificate& cert, cdstring& fpath) const
{
	bool result = false;
	try
	{
		// Get unique hashed file name of cert
		unsigned long ctr = 0;
		cdstring xhash = cert.GetHash();
		while(ctr < 1000)
		{
			// Get hashed path
			cdstring hash;
			hash.reserve(32);
			::snprintf(hash.c_str_mod(), 32, mPEMFormat ? "%s-%ld.pem" : "%s.%ld", xhash.c_str(), ctr++);
			fpath = mDirectory;
			::addtopath(fpath, hash);
			
			// Check for file
			if (!::fileexists(fpath))
				break;
		}

		// Open the file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		StCreatorType file('R*ch', 'TEXT');
#endif
		FILE* fp = ::fopen_utf8(fpath, "w+");
		if (!fp)
		{
			fpath = cdstring::null_str;
			return false;
		}

		// Write pem public cert
		int ret = ::PEM_write_X509(fp, cert.GetCertificate());

		// Write pem private key if present
		if ((ret == 1) && cert.HasPKey())
		{
			// Create cipher to use for passphrase
			const EVP_CIPHER* cipher = NULL;
			if (!cert.GetPassphrase().empty())
				cipher = ::EVP_des_ede3_cbc();
	
			ret = ::PEM_write_PrivateKey(fp, cert.GetPKey(), cipher, NULL, 0, NULL, (void*) cert.GetPassphrase().c_str());
		}

		// Close
		::fclose(fp);

		// Delete if error
		if (ret == 0)
		{
			::remove_utf8(fpath);
			fpath = cdstring::null_str;
		}
		else
			result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return result;
}

cdstring CCertificateStoreDir::FindCertFile(const CCertificate& cert) const
{
	// Determine file path for certificate
	cdstrmap::const_iterator found = mCertsMap.find(cert.GetFingerprint());
	if (found == mCertsMap.end())
		return cdstring::null_str;
	else
		return (*found).second;
}
