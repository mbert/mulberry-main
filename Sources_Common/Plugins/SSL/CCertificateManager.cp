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

// CCertificateManager.cp
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

#include "CCertificateManager.h"

#include "CAcceptCertDialog.h"
#include "CCertificate.h"
#include "CCertificateStoreDir.h"
#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CGetPassphraseDialog.h"
#include "CLocalCommon.h"
#include "CLog.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CSSLPlugin.h"
#include "CSSLUtils.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CUnicodeStdLib.h"
#include "CURL.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryApp.h"
#include "CStringResources.h"
#endif

#include "diriterator.h"
#include "cdfstream.h"

#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>

#if __dest_os == __win32_os || __dest_os == __mac_os
#    define LIST_SEPARATOR_CHAR ';'
#else
#    define LIST_SEPARATOR_CHAR ':'
#endif

#include __stat_header
#if defined(__GNUC__) || defined(__VCPP__)
#define BIO_FP_BINARY	0
#endif

extern "C" BIO_METHOD *BIO_f_crlfbuffer(void);

const char* cCACertsDir = "CACerts";
const char* cCertsMapFile = "certmap.db";
const char* cPEMExtension = ".pem";

#define FORMAT_UNDEF    0
#define FORMAT_ASN1     1
#define FORMAT_TEXT     2
#define FORMAT_PEM      3
#define FORMAT_NETSCAPE 4
#define FORMAT_PKCS12   5
#define FORMAT_SMIME    6

#define NETSCAPE_CERT_HDR	"certificate"

CCertificateManager* CCertificateManager::sCertificateManager = NULL;

CCertificateManager::CCertificateManager() :
	mServerAccept(eServerCertificates), mUserAccept(eUserCertificates)
{
	sCertificateManager = this;
	mLoadedCerts = false;
}

CCertificateManager::~CCertificateManager()
{
	sCertificateManager = NULL;
}

// Check fr and load cert maager
bool CCertificateManager::HasCertificateManager()
{
	// Check whether it currently exists and whether SSL is avilable
	if ((CCertificateManager::sCertificateManager == NULL) && CPluginManager::sPluginManager.HasSSL())
	{
		// Load SSL (which also creates certifcate manager)
		CPluginManager::sPluginManager.GetSSL()->InitSSL();
	}
	
	return CCertificateManager::sCertificateManager != NULL;
}

// Load user/personal certificates
void CCertificateManager::LoadCertificates()
{
	if (!mLoadedCerts)
	{
		// Read in accept DB for servers
		mServerAccept.Init();

		// Read in accept DB for users
		mUserAccept.Init();


		// Create certificate stores for directory based certs and load all certs

#if __dest_os == __linux_os
		// Add default openssl certs directory as read-only store
		const char* openssldir = ::getenv(::X509_get_default_cert_dir_env());
		if (!openssldir || (*openssldir == 0))
			openssldir = ::X509_get_default_cert_dir();

		if (openssldir && (*openssldir != 0))
		{
			mCAStore.push_back(new CCertificateStoreDir(openssldir, false, true));
			mCAStore.back()->Init();
		}
#endif
		// Plug-in/CACerts is read-only (use openssl .0 file extensions)
		for(cdstrvect::const_iterator iter = CPluginManager::sPluginManager.GetPluginDirs().begin(); iter != CPluginManager::sPluginManager.GetPluginDirs().end(); iter++)
		{
			cdstring cadir = *iter;
			::addtopath(cadir, cCACertsDir);
			if (::direxists(cadir))
			{
				mCAStore.push_back(new CCertificateStoreDir(cadir, false, true));
				mCAStore.back()->Init();
			}
		}

		// CA certificates (use openssl .0 file extensions)
		mCAStore.push_back(new CCertificateStoreDir(CConnectionManager::sConnectionManager.GetAuthoritiesCertsDirectory(), false));
		mCAStore.back()->Init();

		// Acceptable server certificates (use -0.pem file extensions)
		mServerStore.push_back(new CCertificateStoreDir(CConnectionManager::sConnectionManager.GetServerCertsDirectory()));
		mServerStore.back()->Init();

		// User S/MIME public certs (use -0.pem file extensions)
		mUserStore.push_back(new CCertificateStoreDir(CConnectionManager::sConnectionManager.GetUserCertsDirectory()));
		mUserStore.back()->Init();

		// Personal S/MIME public/private certs (use -0.pem file extensions)
		mPersonalStore.push_back(new CCertificateStoreDir(CConnectionManager::sConnectionManager.GetPersonalCertsDirectory()));
		mPersonalStore.back()->Init();
	}

	mLoadedCerts = true;
}

// Load user/personal certificates
void CCertificateManager::ReloadCertificates()
{
	if (!mLoadedCerts)
		LoadCertificates();
	else
	{
		ReloadCertificates(eCACertificates);
		ReloadCertificates(eServerCertificates);
		ReloadCertificates(eUserCertificates);
		ReloadCertificates(ePersonalCertificates);
	}
}

// Load user/personal certificates
void CCertificateManager::ReloadCertificates(ECertificateType type)
{
	// Generate a suitable name for the cert
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return;
	}

	// Reload each store
	for(CCertificateStoreList::const_iterator iter = store->begin(); iter != store->end(); iter++)
	{
		(*iter)->Reload();
	}
}

// Verify server cert and throw if invalid
void CCertificateManager::LoadSSLRootCerts(SSL_CTX* ctx)
{
	// Setup cert chain from directory
#if __dest_os == __linux_os || __dest_os == __mac_os_x
	// Always use default certs on linux
	if (!::SSL_CTX_set_default_verify_paths(ctx))
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}
#endif

	// Add each default CACerts directory if it exists
	cdstring dir;
	for(cdstrvect::const_iterator iter = CPluginManager::sPluginManager.GetPluginDirs().begin(); iter != CPluginManager::sPluginManager.GetPluginDirs().end(); iter++)
	{
		cdstring temp = *iter;
		::addtopath(temp, cCACertsDir);
		if (::direxists(temp))
		{
			if (!dir.empty())
				dir += LIST_SEPARATOR_CHAR;
			dir += temp;
		}
	}

	// Add user authorities directory
	if (!dir.empty())
		dir += LIST_SEPARATOR_CHAR;
	dir += CConnectionManager::sConnectionManager.GetAuthoritiesCertsDirectory();
	
	if (!::SSL_CTX_load_verify_locations(ctx, NULL, dir))
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}
}

// Verify server cert and throw if invalid
void CCertificateManager::LoadSMIMERootCerts(X509_STORE* store)
{
	//  Must have valid store
	if (store == NULL)
		return;

	// Setup cert chain from directory
#if __dest_os == __linux_os
	::X509_STORE_set_default_paths(store);
#endif

	// Add each default CACerts directory if it exists
	cdstring dir;
	for(cdstrvect::const_iterator iter = CPluginManager::sPluginManager.GetPluginDirs().begin(); iter != CPluginManager::sPluginManager.GetPluginDirs().end(); iter++)
	{
		cdstring temp = *iter;
		::addtopath(temp, cCACertsDir);
		if (::direxists(temp))
		{
			if (!dir.empty())
				dir += LIST_SEPARATOR_CHAR;
			dir += temp;
		}
	}

	// Add user authorities directory
	if (!dir.empty())
		dir += LIST_SEPARATOR_CHAR;
	dir += CConnectionManager::sConnectionManager.GetAuthoritiesCertsDirectory();
	
	// Add a directory lookup item
	X509_LOOKUP* lookup = ::X509_STORE_add_lookup(store, ::X509_LOOKUP_hash_dir());
	if (lookup == NULL)
		return;

	// Add directory to lookup
	if (::X509_LOOKUP_add_dir(lookup, dir, X509_FILETYPE_PEM) != 1)
		return;
}

void CCertificateManager::GetAllCertificatesInStores(ECertificateType type, CCertificateList& list)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Get store to use
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return;
	}

	// Add descriptors for private certs to list
	for (CCertificateStoreList::const_iterator iter = store->begin(); iter != store->end(); iter++)
		(*iter)->GetCertificates(list);
}

void CCertificateManager::GetPrivateCertificates(cdstrvect& subjs, cdstrvect& fingerprints)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Add descriptors for private certs to list
	for (CCertificateStoreList::const_iterator iter = mPersonalStore.begin(); iter != mPersonalStore.end(); iter++)
		(*iter)->GetCertificates(subjs, fingerprints);
}

bool CCertificateManager::GetSubject(const cdstring& key, cdstring& result, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Look at each cached user certificate
	const CCertificate* cert = LookupCertificate(key, eUserCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetSubject();
		return true;
	}

	// Look at each cached user certificate
	cert = LookupCertificate(key, ePersonalCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetSubject();
		return true;
	}

	return false;
}

bool CCertificateManager::GetHash(const cdstring& key, cdstring& result, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Look at each cached user certificate
	const CCertificate* cert = LookupCertificate(key, eUserCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetHash();
		return true;
	}

	// Look at each cached user certificate
	cert = LookupCertificate(key, ePersonalCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetHash();
		return true;
	}

	return false;
}

bool CCertificateManager::GetFingerprint(const cdstring& key, cdstring& result, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Look at each cached user certificate
	const CCertificate* cert = LookupCertificate(key, eUserCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetFingerprint();
		return true;
	}

	// Look at each cached user certificate
	cert = LookupCertificate(key, ePersonalCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetFingerprint();
		return true;
	}

	return false;
}

bool CCertificateManager::GetEmail(const cdstring& key, cdstrvect& result, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Look at each cached user certificate
	const CCertificate* cert = LookupCertificate(key, eUserCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetEmail();
		return true;
	}

	// Look at each cached user certificate
	cert = LookupCertificate(key, ePersonalCertificates, lookup);
	if (cert != NULL)
	{
		result = cert->GetEmail();
		return true;
	}

	return false;
}

bool CCertificateManager::ImportCertificateFile(CCertificateManager::ECertificateType type, const cdstring& fpath)
{
	// Determine file type based on file extension/type
	bool imported = false;
	if (fpath.compare_end(".pem", true))
		imported = ImportCertificateFile(type, fpath, FORMAT_PEM);
	else if (fpath.compare_end(".p12", true))
		imported = ImportCertificateFile(type, fpath, FORMAT_PKCS12);
	else if (fpath.compare_end(".pfx", true))
		imported = ImportCertificateFile(type, fpath, FORMAT_PKCS12);
	else if (fpath.compare_end(".crt", true))
		imported = ImportCertificateFile(type, fpath, FORMAT_PEM);

	if (!imported)
	{
		const int format_types[] = {FORMAT_PEM, FORMAT_PKCS12, FORMAT_NETSCAPE, FORMAT_ASN1, 0};

		// Try import by guessing
		const int* format_type = format_types;
		while(!imported && (*format_type != 0))
			imported = ImportCertificateFile(type, fpath, *format_type++);
	}

	return imported;
}

bool CCertificateManager::ImportCertificateFile(CCertificateManager::ECertificateType type, const cdstring& fpath, int format)
{
	bool imported = false;

	try
	{
		NSSL::StSSLObject<X509> xcert;

		// New input file bio
		NSSL::StSSLObject<BIO> cert(::BIO_new(::BIO_s_file()));
		if (cert.get() == NULL)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Always read as a binary file - we will use crlf converter for text formats
		if (::BIO_ctrl(cert.get(), BIO_C_SET_FILENAME, BIO_CLOSE|BIO_FP_READ|BIO_FP_BINARY, (char*)fpath.c_str()) <= 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// crlf converter bio
		NSSL::StSSLObject<BIO> bcrlf(::BIO_new(::BIO_f_crlfbuffer()));
		if (bcrlf.get() == NULL)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		if (format == FORMAT_PEM)
		{
			// Need crlf converter for text file
			::BIO_push(bcrlf.get(), cert.get());

			NSSL::StSSLObject<STACK_OF(X509_INFO), X509_INFO> xinfos(::PEM_X509_INFO_read_bio(bcrlf.get(), NULL, (pem_password_cb *)NSSL::PassphraseCallback, NULL));
			CCertificateList xcerts;
			bool has_pkey = false;
			for(int i = 0; i < sk_X509_INFO_num(xinfos.get()); i++)
			{
				X509_INFO* xi = sk_X509_INFO_value(xinfos.get(), i);
				if (xi->x509 != NULL)
					xcerts.push_back(new CCertificate(NULL, xi->x509));
				if (xi->x_pkey != NULL)
					has_pkey = true;
			}
			
			// Reread file to get decoded private key if present
			EVP_PKEY* pkey = NULL;
			cdstring passphrase;
			if (has_pkey)
			{
				// Rewind to start
				BIO_reset(bcrlf.get());
				passphrase.reserve(1024);
				pkey = ::PEM_read_bio_PrivateKey(bcrlf.get(), &pkey, (pem_password_cb *)NSSL::PassphraseCallback, passphrase.c_str_mod());
			}

			if (xcerts.size() > 0)
			{
				// Determine which is the leaf and which are ca's
				CCertificateList cacerts;
				CCertificateList leafcerts;
				for(CCertificateList::const_iterator iter = xcerts.begin(); iter != xcerts.end(); iter++)
				{
					// CW9 internal compiler error with find_if
#if 0
					// See if another cert has this one's subject as an issuer
					CCertificateList::const_iterator found = std::find_if(static_cast<CCertificateList::const_iterator>(xcerts.begin()), static_cast<CCertificateList::const_iterator>(xcerts.end()), CCertificatestd::find_if((*iter)->GetSubject(), &CCertificate::GetIssuer));

					// Do not match ourselves (i.e. self-signed)
					if (found == iter)
						found = std::find_if(++found, static_cast<CCertificateList::const_iterator>(xcerts.end()), CCertificatestd::find_if((*iter)->GetSubject(), &CCertificate::GetIssuer));
#else
					CCertificateList::const_iterator found = xcerts.end();
					for(CCertificateList::const_iterator iter2 = xcerts.begin(); iter2 != xcerts.end(); iter2++)
					{
						// Do not match ourselves (i.e. self-signed)
						if (iter2 != iter)
						{
							if ((*iter)->GetSubject() == (*iter2)->GetIssuer())
							{
								found = iter2;
								break;
							}
						}
					}
#endif

					// Add as CA certs if someone else is issued by this, otherwise its a leaf
					if (found != xcerts.end())
						cacerts.push_back(new CCertificate(NULL, **iter, true));
					else
						leafcerts.push_back(new CCertificate(NULL, **iter, true));
				}
				
				// Add pkey to first leaf
				if ((pkey != NULL) && (leafcerts.size() > 0))
					leafcerts.front()->SetPKey(pkey, passphrase);

				// Import each leaf
				for(CCertificateList::const_iterator iter = leafcerts.begin(); iter != leafcerts.end(); iter++)
					imported |= ImportCertificateFile(type, *iter, NULL);

				// Import all cas
				if (cacerts.size() > 0)	
					imported |= ImportCertificateFile(type, NULL, &cacerts);
			}
		}

		else if (format == FORMAT_PKCS12)
		{
			NSSL::StSSLObject<PKCS12> p12(::d2i_PKCS12_bio(cert.get(), NULL));
			NSSL::StSSLObject<X509> cert;
			NSSL::StSSLObject<EVP_PKEY> pkey;
			NSSL::StSSLObject<STACK_OF(X509), X509> ca(sk_X509_new_null());

			// If importing to personal certs, then get the pkey passphrase
			bool personal = (type == ePersonalCertificates);
			cdstring passphrase;
			if (personal)
			{
				// Ask user for passphrase
				cdstring chosen_user;
				unsigned long index = 0;
				CGetPassphraseDialog::PoseDialog(passphrase, NULL, chosen_user, index);
			}

			// Parse pkcs12 data
			X509* temp = NULL;
			EVP_PKEY* ptemp = NULL;
			STACK_OF(X509)* catemp = NULL;
			if (::PKCS12_parse(p12.get(), personal ? passphrase.c_str() : NULL, personal ? &ptemp : NULL, &temp, &catemp) == 1)
			{
				// Cache the xcert
				cert.reset(temp);
				pkey.reset(ptemp);
				ca.reset(catemp);
				
				imported = ImportCertificateFile(type, cert.get(), personal ? pkey.get() : NULL, personal ? &passphrase : NULL, ca.get());
			}
		}

		else if (format == FORMAT_ASN1)
		{
			xcert.reset(::d2i_X509_bio(cert.get(), NULL));
		}

#ifdef ASN1_HEADER
		else if (format == FORMAT_NETSCAPE)
		{
			// Need crlf converter for text file
			::BIO_push(bcrlf.get(), cert.get());

			// We sort of have to do it this way because it is sort of nice
			// to read the header first and check it, then try to read the certificate
			NSSL::StSSLObject<BUF_MEM> buf(::BUF_MEM_new());
			int i;
			int size=0;
			for (;;)
			{
				if ((buf.get() == NULL) || (!::BUF_MEM_grow(buf.get(), size + 1024*10)))
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
				i = ::BIO_read(bcrlf.get(), &(buf->data[size]), 1024*10);
				size += i;
				if (i == 0)
					break;
				if (i < 0)
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}

			const unsigned char* p = (unsigned char*) buf->data;
			const unsigned char* op = p;

#if 0 // KAP: until we find a suitable replacement in openssl-1.0 code
			// First load the header
			NSSL::StSSLObject<ASN1_HEADER> ah(::d2i_ASN1_HEADER(NULL, &p, (long)size));
			if (ah.get() == NULL)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			if ((ah->header == NULL) || (ah->header->data == NULL) || (::strncmp(NETSCAPE_CERT_HDR, (char *)ah->header->data, ah->header->length) != 0))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// header is ok, so now read the object
			p = op;
			ah->meth = ::X509_asn1_meth();
			ASN1_HEADER* temp = ah.release();
			ah.reset(::d2i_ASN1_HEADER(&temp, &p, (long)size));
			if (ah.get() == NULL)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			xcert.reset((X509 *)ah->data);
			ah->data = NULL;
#endif
		}
#endif

		// Do actual certificate import if one is left
		if (xcert.get() != NULL)
			imported = ImportCertificateFile(type, xcert.get(), NULL, NULL, NULL);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		imported = false;
		
		// Do error alert
		cdstring serr;
		cdstring lerr;
		
		NSSL::ERR_better_errors(serr, lerr);
		CLOG_LOGERROR(lerr);

		CErrorHandler::PutStopAlertRsrcTxt("Alerts::General::CertificateImportError", serr);
	}

	return imported;
}

bool CCertificateManager::ImportCertificateFile(CCertificateManager::ECertificateType type, X509* xcert, EVP_PKEY* pkey, const cdstring* passphrase, STACK_OF(X509)* ca)
{
	if (xcert || pkey)
	{
		// Create cert out of this
		CCertificate cert(NULL, xcert, pkey, passphrase);
		
		// Generate a suitable name for the cert
		const CCertificateStoreList* store = NULL;
		switch(type)
		{
		case eCACertificates:
			store = &mCAStore;
			break;
		case eServerCertificates:
			store = &mServerStore;
			break;
		case eUserCertificates:
			store = &mUserStore;
			break;
		case ePersonalCertificates:
			store = &mPersonalStore;
			break;
		default:
			return false;
		}

		// Reload each store
		for(CCertificateStoreList::const_iterator iter = store->begin(); iter != store->end(); iter++)
		{
			if ((*iter)->CanWrite())
				(*iter)->ImportCertificate(cert);
		}
	}
	
	// Add each ca
	if (ca)
	{
		for (int i = 0; i < sk_X509_num(ca); i++)
		{
			X509* x509 = sk_X509_value(ca, i);

			// Create cert out of this
			CCertificate cacert(NULL, x509);

			// Add to CA stores
			for(CCertificateStoreList::const_iterator iter = mCAStore.begin(); iter != mCAStore.end(); iter++)
			{
				if ((*iter)->CanWrite())
					(*iter)->ImportCertificate(cacert);
			}
		}
	}
	
	return true;
}

bool CCertificateManager::ImportCertificateFile(CCertificateManager::ECertificateType type, const CCertificate* cert, const CCertificateList* cas)
{
	if (cert)
	{
		// Generate a suitable name for the cert
		const CCertificateStoreList* store = NULL;
		switch(type)
		{
		case eCACertificates:
			store = &mCAStore;
			break;
		case eServerCertificates:
			store = &mServerStore;
			break;
		case eUserCertificates:
			store = &mUserStore;
			break;
		case ePersonalCertificates:
			store = &mPersonalStore;
			break;
		default:
			return false;
		}

		// Reload each store
		for(CCertificateStoreList::const_iterator iter = store->begin(); iter != store->end(); iter++)
		{
			if ((*iter)->CanWrite())
				(*iter)->ImportCertificate(*cert);
		}
	}
	
	// Add each ca
	if (cas)
	{
		for(CCertificateList::const_iterator iter1 = cas->begin(); iter1 != cas->end(); iter1++)
		{
			// Add to CA stores
			for(CCertificateStoreList::const_iterator iter2 = mCAStore.begin(); iter2 != mCAStore.end(); iter2++)
			{
				if ((*iter2)->CanWrite())
					(*iter2)->ImportCertificate(**iter1);
			}
		}
	}
	
	return true;
}

bool CCertificateManager::CheckServerCertificate(const cdstring& server, std::vector<int>& errors, X509* server_cert)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Get the fingerprint of the cert
	CCertificate cert(NULL, server_cert);
	cdstring fingerprint = cert.GetFingerprint();

	// Check whether cert is in accept-save cache
	bool in_cache = false;
	if (mServerAccept.CertInCache(server_cert, false))
	{
		// Check its in the accept map
		if (mServerAccept.CheckAcceptData(server, fingerprint, false))
			return true;
		else
			in_cache = true;
	}

	// Check whether cert is in accept-once cache
	else if (mServerAccept.CertInCache(server_cert, true))
	{
		// Check its in the accept map
		if (mServerAccept.CheckAcceptData(server, fingerprint, true))
			return true;
		else
			in_cache = true;
	}
	
	// Check whether certificate has been accepted before
	if (in_cache)
	{
		// Clear out existing errors as we've accepted this certificate before
		errors.clear();
	}

	// Normalise server name by stipping off port number
	cdstring real_server(server);
	if (::strchr(real_server.c_str(), ':') != NULL)
		*::strchr(real_server.c_str_mod(), ':') = 0;

	// Important: must check that server name in cert matches the one we are using
	if (!DoCertAltNameCheck(server_cert, real_server, eServerCertificates))
		// Add an application error to error list
		errors.push_back(X509_V_ERR_APPLICATION_VERIFICATION);	
	
	// See whether any errors exist and exit with true if not
	if (!errors.size())
		return true;
	
	// Errors exist => prompt user
	
	// Get list of error strings
	cdstrvect error_txt;
	for(std::vector<int>::const_iterator iter = errors.begin(); iter != errors.end(); iter++)
	{
		// Special for application error
		if (*iter == X509_V_ERR_APPLICATION_VERIFICATION)
			error_txt.push_back("Certificate server address does not match account address");
		else
			error_txt.push_back(::X509_verify_cert_error_string(*iter));
	}
	
	// Now check acceptance
	return AcceptableServer(server, server_cert, errors, error_txt);
}

bool CCertificateManager::AcceptableServer(const cdstring& server, X509* server_cert, const std::vector<int>& errors, const cdstrvect& error_txt)
{
	// Get the fingerprint of the cert
	CCertificate cert(NULL, server_cert);
	cdstring fingerprint = cert.GetFingerprint();

	// Check to see whether we have any 'dire' errors that need an extra alert to the user
	bool dire_error = false;
	for(std::vector<int>::const_iterator iter = errors.begin(); iter != errors.end(); iter++)
	{
		switch(*iter)
		{
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
		case X509_V_ERR_CRL_SIGNATURE_FAILURE:
		case X509_V_ERR_CERT_NOT_YET_VALID:
		case X509_V_ERR_CERT_HAS_EXPIRED:
		case X509_V_ERR_CRL_NOT_YET_VALID:
		case X509_V_ERR_CRL_HAS_EXPIRED:
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
		case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
		case X509_V_ERR_CERT_REVOKED:
		case X509_V_ERR_INVALID_CA:
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
		case X509_V_ERR_INVALID_PURPOSE:
		case X509_V_ERR_CERT_UNTRUSTED:
		case X509_V_ERR_CERT_REJECTED:
			dire_error = true;
			break;
		default:;
		}
	}

	// Prompt Accept Once, Accept & Save, Cancel
	cdstring certtxt = cert.StringCert();
	
	CAcceptCertTask* task = new CAcceptCertTask(certtxt, error_txt);
	int result = task->Go();

	// Check dire error status
	if (dire_error && ((result == CAcceptCertDialog::eAcceptOnce) || (result == CAcceptCertDialog::eAcceptSave)))
	{
		// Put up warning alert - force cancel of accept if user cancels
		CCautionAlertTxtTask* task = new CCautionAlertTxtTask(true, "Alerts::General::DireSSLCertError");
		if (task->Go() == CErrorHandler::Cancel)
			return false;
	}

	// Accept once
	if (result == CAcceptCertDialog::eAcceptOnce)
	{
		// Add it to accept once map
		mServerAccept.AddAcceptData(server_cert, server, fingerprint, true);
	}
	else if (result == CAcceptCertDialog::eAcceptSave)
	{
		// Add it to accept save map
		mServerAccept.AddAcceptData(server_cert, server, fingerprint, false);
	}
	else
		return false;

	return true;
}

bool CCertificateManager::CertificateToString(X509* server_cert, cdstring& txt)
{
	// Get the fingerprint of the cert
	CCertificate cert(NULL, server_cert);
	txt = cert.StringCert();
	return true;
}

// Locate certificate file for the corresponding user
cdstring CCertificateManager::FindCertificateFile(const char* key, ECertificateType type, ECertificateLookupType lookup)
{
	// This is never used
	try
	{
		throw -1L;
	}
	catch(...)
	{
	}

	return cdstring::null_str;
}

// Locate certificate file for the corresponding user
X509* CCertificateManager::FindCertificate(const char* key, ECertificateType type, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Determine store to use
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return NULL;
	}

	// Try user certs first
	return FindCertificateInStore(*store, key, lookup);
}

X509* CCertificateManager::FindCertificateInStore(const CCertificateStoreList& store, const char* key, ECertificateLookupType lookup) const
{
	// Try user certs first
	for(CCertificateStoreList::const_iterator iter = store.begin(); iter != store.end(); iter++)
	{
		// Try to find cert in store
		const CCertificate* cert = (*iter)->FindCertificate(key, lookup);
		
		// Return the cert, bumping its reference count
		if (cert)
		{
			X509* x = cert->GetCertificate(true);
			if (x)
				return x;
		}
	}

	return NULL;
}

#ifdef USE_CMS
X509* CCertificateManager::FindCertificate(CMS_RECIP_INFO* ri, ECertificateType type)
#else
X509* CCertificateManager::FindCertificate(PKCS7_RECIP_INFO* ri, ECertificateType type)
#endif
{
	// Make sure certs are loaded
	LoadCertificates();

	// Determine store to use
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return NULL;
	}

	// Try user certs first
	return FindCertificateInStore(*store, ri);
}
 
#ifdef USE_CMS
X509* CCertificateManager::FindCertificateInStore(const CCertificateStoreList& store, CMS_RECIP_INFO* ri) const
#else
X509* CCertificateManager::FindCertificateInStore(const CCertificateStoreList& store, PKCS7_RECIP_INFO* ri) const
#endif
{
	// Try user certs first
	for(CCertificateStoreList::const_iterator iter = store.begin(); iter != store.end(); iter++)
	{
		// Try to find cert in store
		const CCertificate* cert = (*iter)->FindCertificate(ri);
		
		// Return the cert, bumping its reference count
		if (cert)
		{
			X509* x = cert->GetCertificate(true);
			if (x)
				return x;
		}
	}

	return NULL;
}

EVP_PKEY* CCertificateManager::LoadPrivateKey(const char* key, const char* passphrase, ECertificateLookupType lookup)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Try personal certs
	for(CCertificateStoreList::const_iterator iter = mPersonalStore.begin(); iter != mPersonalStore.end(); iter++)
	{
		// Try to find cert in store
		EVP_PKEY* pkey = (*iter)->LoadPrivateKey(key, passphrase, lookup);
		
		// Return the pkey
		if (pkey)
			return pkey;
	}

	return NULL;
}

STACK_OF(X509)* CCertificateManager::GetCerts(const char* key, ECertificateType type, ECertificateLookupType lookup)
{
	// Determine store to use
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return NULL;
	}

	STACK_OF(X509)* result = sk_X509_new_null();
	
	// Add each matching pkey to the stack
	for(CCertificateStoreList::const_iterator iter1 = store->begin(); iter1 != store->end(); iter1++)
	{
		// Try to find cert in store
		CCertificateList xcerts;
		(*iter1)->FindCertificates(key, xcerts, lookup);
		
		// Add the certs
		for(CCertificateList::const_iterator iter2 = xcerts.begin(); iter2 != xcerts.end(); iter2++)
			sk_X509_push(result, (*iter2)->GetCertificate(true));
	}

	return result;	
}

bool CCertificateManager::CheckUserCertificate(const cdstring& email, std::vector<int>& errors, X509* user_cert)
{
	// Make sure certs are loaded
	LoadCertificates();

	// Get the fingerprint of the cert
	CCertificate cert(NULL, user_cert);
	cdstring fingerprint = cert.GetFingerprint();

	// Check whether cert is in accept-save cache
	bool in_cache = false;
	if (mUserAccept.CertInCache(user_cert, false))
	{
		// Check its in the accept map
		if (mUserAccept.CheckAcceptData(email, fingerprint, false))
			return true;
		else
			in_cache = true;
	}

	// Check whether cert is in accept-once cache
	else if (mUserAccept.CertInCache(user_cert, true))
	{
		// Check its in the accept map
		if (mUserAccept.CheckAcceptData(email, fingerprint, true))
			return true;
		else
			in_cache = true;
	}
	
	// Check whether certificate has been accepted before
	if (in_cache)
	{
		// Clear out existing errors as we've accepted this certificate before
		errors.clear();
	}

	// Important: must check that email address in cert matches the one we are using
	if (!DoCertAltNameCheck(user_cert, email, eUserCertificates))
		// Add an application error to error list
		errors.push_back(X509_V_ERR_APPLICATION_VERIFICATION);
	
	// See whether any errors exist and exit with true if not
	if (!errors.size())
	{
		// Cache the user cert if required
		if (CPreferences::sPrefs->mCacheUserCerts.GetValue() && !mUserAccept.CertInCache(user_cert, false))
		{
			// Save it to disk
			mUserAccept.WriteAcceptable(user_cert);
		}
		return true;
	}
	
	// Errors exist => prompt user
	
	// Get list of error strings
	cdstrvect error_txt;
	for(std::vector<int>::const_iterator iter = errors.begin(); iter != errors.end(); iter++)
	{
		// Special for application error
		if (*iter == X509_V_ERR_APPLICATION_VERIFICATION)
			error_txt.push_back("Certificate email address does not match user email address");
		else
			error_txt.push_back(::X509_verify_cert_error_string(*iter));
	}
	
	// Now check acceptance
	return AcceptableUser(email, user_cert, errors, error_txt);
}

bool CCertificateManager::AcceptableUser(const cdstring& email, X509* user_cert, const std::vector<int>& errors, const cdstrvect& error_txt)
{
	// Get the fingerprint of the cert
	CCertificate cert(NULL, user_cert);
	cdstring fingerprint = cert.GetFingerprint();

	// Check to see whether we have any 'dire' errors that need an extra alert to the user
	bool dire_error = false;
	for(std::vector<int>::const_iterator iter = errors.begin(); iter != errors.end(); iter++)
	{
		switch(*iter)
		{
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
		case X509_V_ERR_CRL_SIGNATURE_FAILURE:
		case X509_V_ERR_CERT_NOT_YET_VALID:
		case X509_V_ERR_CERT_HAS_EXPIRED:
		case X509_V_ERR_CRL_NOT_YET_VALID:
		case X509_V_ERR_CRL_HAS_EXPIRED:
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
		case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
		case X509_V_ERR_CERT_REVOKED:
		case X509_V_ERR_INVALID_CA:
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
		case X509_V_ERR_INVALID_PURPOSE:
		case X509_V_ERR_CERT_UNTRUSTED:
		case X509_V_ERR_CERT_REJECTED:
			dire_error = true;
			break;
		default:;
		}
	}

	// Prompt Accept Once, Accept & Save, Cancel
	cdstring certtxt = cert.StringCert();
	
	CAcceptCertTask* task = new CAcceptCertTask(certtxt, error_txt);
	int result = task->Go();

	// Check dire error status
	if (dire_error && ((result == CAcceptCertDialog::eAcceptOnce) || (result == CAcceptCertDialog::eAcceptSave)))
	{
		// Put up warning alert - force cancel of accept if user cancels
		CCautionAlertTxtTask* task = new CCautionAlertTxtTask(true, "Alerts::General::DireSSLCertError");
		if (task->Go() == CErrorHandler::Cancel)
			return false;
	}

	// Accept once
	if (result == CAcceptCertDialog::eAcceptOnce)
	{
		// Add it to accept once map
		mUserAccept.AddAcceptData(user_cert, email, fingerprint, true);
	}
	else if (result == CAcceptCertDialog::eAcceptSave)
	{
		// Add it to accept save map
		mUserAccept.AddAcceptData(user_cert, email, fingerprint, false);
	}
	else
		return false;

	return true;
}

void CCertificateManager::CertHandleError(X509_STORE_CTX *ctx) const
{
	// Get error string from openssl
	const char* errstr = ::X509_verify_cert_error_string(ctx->error);
	CStopAlertTxtTask* task = new CStopAlertTxtTask(errstr);
	task->Go();
}

// Locate certificate file for the corresponding user
const CCertificate* CCertificateManager::LookupCertificate(const char* key, ECertificateType type, ECertificateLookupType lookup) const
{
	const CCertificateStoreList* list = NULL;
	switch(type)
	{
	case eCACertificates:
		list = &mCAStore;
		break;
	case eServerCertificates:
		list = &mServerStore;
		break;
	case eUserCertificates:
		list = &mUserStore;
		break;
	case ePersonalCertificates:
		list = &mPersonalStore;
		break;
	default:;
	}
	if (list == NULL)
		return NULL;

	// Look at each store
	for(CCertificateStoreList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		// Try to find cert in store
		const CCertificate* cert = (*iter)->FindCertificate(key, lookup);
		
		// Return the cert
		if (cert)
			return cert;
	}

	return NULL;
}

void CCertificateManager::WriteCertificate(X509* xcert, ECertificateType type) const
{
	// Generate a suitable name for the cert
	const CCertificateStoreList* store = NULL;
	switch(type)
	{
	case eCACertificates:
		store = &mCAStore;
		break;
	case eServerCertificates:
		store = &mServerStore;
		break;
	case eUserCertificates:
		store = &mUserStore;
		break;
	case ePersonalCertificates:
		store = &mPersonalStore;
		break;
	default:
		return;
	}

	// Create dumy cert object
	CCertificate cert(NULL, xcert);

	// Look at each store
	for(CCertificateStoreList::const_iterator iter = store->begin(); iter != store->end(); iter++)
	{
		// Only for stores which are writeable
		if ((*iter)->CanWrite())
			(*iter)->ImportCertificate(cert);
	}
}

bool CCertificateManager::DoCertAltNameCheck(X509* xcert, const cdstring& name, ECertificateType type)
{
	bool result = false;

	CCertificate cert(NULL, xcert);
	cdstring compare;
	switch(type)
	{
	case eServerCertificates:
		for(cdstrvect::const_iterator iter = cert.GetDNS().begin(); iter != cert.GetDNS().end(); iter++)
		{
			if (::strpmatch(name, *iter))
				return true;
		}
		return false;
	case eUserCertificates:
	case ePersonalCertificates:
		for(cdstrvect::const_iterator iter = cert.GetEmail().begin(); iter != cert.GetEmail().end(); iter++)
		{
			if (::strcmpnocase(name, *iter) == 0)
				return true;
		}
		return false;
	default:
		return false;
	}
}

#pragma mark ____________________________CCertificateManager::SCertificateAccept

CCertificateManager::SCertificateAccept::SCertificateAccept()
{
	// Create initial empty stack
	mCertificates = sk_X509_new_null();
}

// Remove any certificates from stack
CCertificateManager::SCertificateAccept::~SCertificateAccept()
{
	if (mCertificates)
	{
		// Free each certificate first
		for (int i=0; i < sk_X509_num(mCertificates); i++)
		{
			X509* x509 = sk_X509_value(mCertificates, i);
			::X509_free(x509);
		}
		
		// Free the stack
		sk_X509_free(mCertificates);
		mCertificates = NULL;
	}
}

#pragma mark ____________________________CCertificateManager::CAcceptDB

void CCertificateManager::CAcceptDB::Init()
{
	// Get appropriate path
	switch(mType)
	{
	case eServerCertificates:
		SetDBDirectory(CConnectionManager::sConnectionManager.GetServerCertsDirectory());
		break;
	case eUserCertificates:
		SetDBDirectory(CConnectionManager::sConnectionManager.GetUserCertsDirectory());
		break;
	default:
		return;
	}

	// Read in server acceptable items
	ReadAcceptData();
}

void CCertificateManager::CAcceptDB::SetDBDirectory(const cdstring& dir)
{
	mDir = dir;
	mPath = dir;
	::addtopath(mPath, cCertsMapFile);
}

bool CCertificateManager::CAcceptDB::CertInCache(X509* xcert, bool once) const
{
	// Look in once cache
	if (once)
	{
		STACK_OF(X509)* stack = mAcceptOnce.mCertificates;

		// Check whether cert is in cache
		for (int i = 0; i < sk_X509_num(stack); i++)
		{
			X509* x509 = sk_X509_value(stack, i);
			if (::X509_cmp(x509, xcert) == 0)
				return true;
		}
	}
	else
	{
		CCertificate cert(NULL, xcert);
		cdstring fingerprint = cert.GetFingerprint();
		if (CCertificateManager::sCertificateManager->LookupCertificate(fingerprint, mType, eByFingerprint) != NULL)
			return true;
	}
	
	return false;
}

void CCertificateManager::CAcceptDB::WriteAcceptable(X509* cert) const
{
	// Write to stores
	CCertificateManager::sCertificateManager->WriteCertificate(cert, mType);
}

void CCertificateManager::CAcceptDB::WriteAcceptData() const
{
	// Only bother if something present
	if (mAcceptSaveData.size())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		StCreatorType file('R*ch', 'TEXT');
#endif
		cdofstream fout(mPath);
		for(cdstrset::const_iterator iter = mAcceptSaveData.begin(); iter != mAcceptSaveData.end(); iter++)
			fout << *iter << os_endl;
	}
	else
	{
		// Delete whatever was there before
		::remove_utf8(mPath);
	}
}

void CCertificateManager::CAcceptDB::ReadAcceptData()
{
	// Clear anything already present
	mAcceptSaveData.clear();
	
	// See if file present
	if (::fileexists(mPath))
	{
		cdifstream fin(mPath);
		
		// Read lines from file
		while(!fin.fail())
		{
			// Get a line
			cdstring line;
			::getline(fin, line, 0);
			
			// Must have valid items
			if (!line.empty())
				mAcceptSaveData.insert(line);
		}
	}
}

bool CCertificateManager::CAcceptDB::CheckAcceptData(const cdstring& name, const cdstring& fingerprint, bool once) const
{
	const cdstrset& set = (once ? mAcceptOnceData : mAcceptSaveData);
	
	// Create key: "<<name>>\t<<fingerprint>>"
	cdstring key = name;
	key += "\t";
	key += fingerprint;

	// Look for key
	return (set.find(key) != set.end());
}

void CCertificateManager::CAcceptDB::AddAcceptData(X509* cert, const cdstring& name, const cdstring& fingerprint, bool once)
{
	// Write/cache certificate first
	if (once)
	{
		// Add it to the once store if not already there
		if (!CertInCache(cert, true))
		{
			X509* copy_cert = ::X509_dup(cert);
			sk_X509_push(mAcceptOnce.mCertificates, copy_cert);
		}
	}
	else
	{
		// Add it to the save store if not already there
		if (!CertInCache(cert, false))
		{
			// Save it to disk
			WriteAcceptable(cert);
		}
	}

	// Now add to DB
	cdstrset& set = (once ? mAcceptOnceData : mAcceptSaveData);

	// Create key: "<<name>>\t<<fingerprint>>"
	cdstring key = name;
	key += "\t";
	key += fingerprint;

	// Add to map
	std::pair<cdstrset::const_iterator, bool> result = set.insert(key);

	// Always do immediate write for saved maps if something was actually added
	if (!once && result.second)
		WriteAcceptData();
}
