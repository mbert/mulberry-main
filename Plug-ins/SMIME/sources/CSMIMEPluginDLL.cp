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

// CSMIMEPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a PGP security DLL based plug-in for use in Mulberry.
//
// History:
// 04-May-1998: Created initial header and implementation.
//

#include <stdio.h>
#include <stdlib.h>

#include "CSMIMEPluginDLL.h"

#include "CCertificateManagerCOM.h"
#if __dest_os == __win32_os
#include "CDLLFunctions.h"
#endif
#include "CPluginInfo.h"
#include "CStringUtils.h"
#include "CURL.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <memory>

#if __dest_os == __linux_os
#define O_BINARY 0
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#endif

#define FORMAT_UNDEF    0
#define FORMAT_ASN1     1
#define FORMAT_TEXT     2
#define FORMAT_PEM      3
#define FORMAT_NETSCAPE 4
#define FORMAT_PKCS12   5
#define FORMAT_SMIME    6

#if __dest_os == __win32_os
CDLLLoader* sSSLLoader = NULL;
#endif

extern "C" {
int RAND_load_rsrc();
}

#pragma mark ____________________________consts

const char* cPluginName = "SMIME Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSecurity;
const char* cPluginDescription = "SMIME Security plugin for Mulberry." COPYRIGHT;
const char* cProcessedBy = "processed by Mulberry SMIME Plugin";
const char* cProcessVersion = "Mulberry SMIME Plugin v2.0";

#pragma mark ____________________________CSMIMEPluginDLL

class StRemoveFile
{
public:
	StRemoveFile()
		{ }
	StRemoveFile(const char* filename)
		{ mFileName = filename; }
	~StRemoveFile()
		{ if (!mFileName.empty()) ::remove(mFileName); }
	
	void set(const char* filename)
		{ mFileName = filename; }
private:
	cdstring mFileName;
};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class StRemoveFileSpec
{
public:
	StRemoveFileSpec()
		{ *mFileSpec.name = 0; }
	StRemoveFileSpec(FSSpec* filespec)
		{ mFileSpec = *filespec; }
	~StRemoveFileSpec()
		{ if (*mFileSpec.name) ::FSpDelete(&mFileSpec); }

	void set(FSSpec* filespec)
		{ mFileSpec = *filespec; }
private:
	FSSpec mFileSpec;
};
#else
class StRemoveFileSpec
{
public:
	StRemoveFileSpec()
		{ }
	StRemoveFileSpec(const char* filename)
		{ mFileName = filename; }
	~StRemoveFileSpec()
		{ if (!mFileName.empty()) ::remove(mFileName); }
	
	void set(const char* filename)
		{ mFileName = filename; }
private:
	cdstring mFileName;
};
#endif

// Constructor
CSMIMEPluginDLL::CSMIMEPluginDLL()
{
	mData = new SData;
	mData->mErrno = eSecurity_NoErr;
	mData->mSignedByList = NULL;
	mData->mEncryptedToList = NULL;
	
	mCertificateManagerCOM = NULL;
}

// Destructor
CSMIMEPluginDLL::~CSMIMEPluginDLL()
{
	if (mData->mSignedByList != NULL)
		cdstring::FreeArray(mData->mSignedByList);
	if (mData->mEncryptedToList != NULL)
		cdstring::FreeArray(mData->mEncryptedToList);

	delete mData;
}

// Initialise plug-in
void CSMIMEPluginDLL::Initialise(void)
{
	// Do default
	CSecurityPluginDLL::Initialise();
}

// DLL entry point and dispatch
long CSMIMEPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CSecurityPluginDLL::eSecuritySetSMIMEContext:
	{
		SSMIMEContext* context = (SSMIMEContext*) data;
		return SetContext(context);
	}
	default:
		return CSecurityPluginDLL::Entry(code, data, refCon);
	}
}

// Set dll contexts
long CSMIMEPluginDLL::SetContext(SSMIMEContext* context)
{
	// Initialise dll
#if __dest_os == __win32_os
	if (sSSLLoader == NULL)
		sSSLLoader = new CDLLLoader(context->mDLL);
#endif
	mCertificateManagerCOM = context->mCertMgr;
	return 1;
}

// Does plug-in need to be registered
bool CSMIMEPluginDLL::UseRegistration(unsigned long* key)
{
	if (key != NULL)
		*key = ('Mlby' | 'PGP5');
	return false;
}

// Can plug-in run as demo
bool CSMIMEPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

#define DATE_PROTECTION		0

#define	COPYP_MAX_YEAR	2000
#define COPYP_MAX_MONTH	3

// Test for run ability
bool CSMIMEPluginDLL::CanRun(void)
{
	bool result = false;

#if DATE_PROTECTION
	time_t systime = ::time(nil);
	struct tm* currtime = ::localtime(&systime);

	if ((currtime->tm_year + 1900 > COPYP_MAX_YEAR) ||
		((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 > COPYP_MAX_MONTH)))
		return false;
#endif

	// If we get here we must be able to run and have linked with SSL plugin
	return true;
}

// Returns the name of the plug-in
const char* CSMIMEPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CSMIMEPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CSMIMEPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CSMIMEPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CSMIMEPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

#pragma mark ____________________________Memory Based

// Sign data with address
long CSMIMEPluginDLL::SignData(const char* in, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	long result = 0;

	// Write data to temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec in_spec;
	in_spec.name[0] = 0;
	FSSpec* in_tmp = &in_spec;
	FSSpec out_spec;
	out_spec.name[0] = 0;
	FSSpec* out_tmp = &out_spec;
#else
	char in_tmp[1024];
	*in_tmp = 0;

	char out_tmp[1024];
	*out_tmp = 0;
#endif

	try
	{
		// Create temp files
		TempCreate(in_tmp, out_tmp, in);

		// Make sure temp files are deleted once we are done
		StRemoveFileSpec _in_remove(in_tmp);
		StRemoveFileSpec _out_remove(out_tmp);

		// Do file based sign (flag for use of temp files)
		result = SignFileX(in_tmp, key, out_tmp, useMime, binary, true);

		// Copy output data to memory
		if (result != 0)
			// Read in temp data
			TempRead(out_tmp, out, out_len);
	}
	catch(...)
	{
		// Catch all and fall through to clean-up
	}

	return result;
}

// Encrypt to addresses
long CSMIMEPluginDLL::EncryptData(const char* in, const char** to, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	long result = 0;

	// Write data to temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec in_spec;
	in_spec.name[0] = 0;
	FSSpec* in_tmp = &in_spec;
	FSSpec out_spec;
	out_spec.name[0] = 0;
	FSSpec* out_tmp = &out_spec;
#else
	char in_tmp[1024];
	*in_tmp = 0;

	char out_tmp[1024];
	*out_tmp = 0;
#endif

	try
	{
		// Create temp files
		TempCreate(in_tmp, out_tmp, in);

		// Make sure temp files are deleted once we are done
		StRemoveFileSpec _in_remove(in_tmp);
		StRemoveFileSpec _out_remove(out_tmp);

		// Do file based sign (flag for use of temp files)
		result = EncryptFileX(in_tmp, to, out_tmp, useMime, binary, true);

		// Copy output data to memory
		if (result != 0)
			// Read in temp data
			TempRead(out_tmp, out, out_len);
	}
	catch(...)
	{
		// Catch all and fall through to clean-up
	}

	return result;
}

// Encrypt to addresses and sign with address
long CSMIMEPluginDLL::EncryptSignData(const char* in, const char** to, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	long result = 0;

	// Write data to temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec in_spec;
	in_spec.name[0] = 0;
	FSSpec* in_tmp = &in_spec;
	FSSpec out_spec;
	out_spec.name[0] = 0;
	FSSpec* out_tmp = &out_spec;
#else
	char in_tmp[1024];
	*in_tmp = 0;

	char out_tmp[1024];
	*out_tmp = 0;
#endif

	try
	{
		// Create temp files
		TempCreate(in_tmp, out_tmp, in);

		// Make sure temp files are deleted once we are done
		StRemoveFileSpec _in_remove(in_tmp);
		StRemoveFileSpec _out_remove(out_tmp);

		// Do file based sign (flag for use of temp files)
		result = EncryptSignFileX(in_tmp, to, key, out_tmp, useMime, binary, true);

		// Copy output data to memory
		if (result != 0)
			// Read in temp data
			TempRead(out_tmp, out, out_len);
	}
	catch(...)
	{
		// Catch all and fall through to clean-up
	}

	return result;
}

// Decrypt or verify data
long CSMIMEPluginDLL::DecryptVerifyData(const char* in, const char* sig, const char* in_from,
										char** out, unsigned long* out_len, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	long result = 0;

	// Write data to temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec in_spec;
	in_spec.name[0] = 0;
	FSSpec* in_tmp = &in_spec;
	FSSpec out_spec;
	out_spec.name[0] = 0;
	FSSpec* out_tmp = &out_spec;
#else
	char in_tmp[1024];
	*in_tmp = 0;

	char out_tmp[1024];
	*out_tmp = 0;
#endif

	try
	{
		// Create temp files converting input data to CRLF format
		{
			cdstring in_crlf(in);
			in_crlf.ConvertEndl(eEndl_CRLF);
			TempCreate(in_tmp, out_tmp, in_crlf);
		}

		// Make sure temp files are deleted once we are done
		StRemoveFileSpec _in_remove(in_tmp);
		StRemoveFileSpec _out_remove(out_tmp);

		// Do file based sign (flag for use of temp files)
		result = DecryptVerifyFileX(in_tmp, sig, in_from, out_tmp, out_signedby, out_encryptedto, success, did_sig, sig_ok, binary, true);

		// Copy output data to memory
		if (result && out && out_len)
			// Read in temp data
			TempRead(out_tmp, out, out_len);
	}
	catch(...)
	{
		// Catch all and fall through to clean-up
	}

	return result;
}

long CSMIMEPluginDLL::DisposeData(const char* data)
{
	::free((void*) data);
	return 1;
}

#pragma mark ____________________________File based

// Sign file
long CSMIMEPluginDLL::SignFile(fspec in, const char* key, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return SignFileX(in, key, out, useMime, binary, false);
}

// Encrypt file
long CSMIMEPluginDLL::EncryptFile(fspec in, const char** to, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return EncryptFileX(in, to, out, useMime, binary, false);
}

// Encrypt & sign file
long CSMIMEPluginDLL::EncryptSignFile(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return EncryptSignFileX(in, to, key, out, useMime, binary, false);
}

// Decrypt/verify file
long CSMIMEPluginDLL::DecryptVerifyFile(fspec in, const char* sig, const char* in_from,
										fspec out, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return DecryptVerifyFileX(in, sig, in_from, out, out_signedby, out_encryptedto, success, did_sig, sig_ok, binary, false);
}

#pragma mark ____________________________Local File based operations

// Sign file
long CSMIMEPluginDLL::SignFileX(fspec in, const char* key, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	long result = 1;

	// Must have cert manager at this point
	if (!mCertificateManagerCOM)
	{
		REPORTERROR(eSecurity_UnknownError, "No certificate manager");
		return 0;
	}

	// Signing requires passphrase
	char passphrase[256];
	if (!GetSignKeyPassphrase(key, passphrase))
	{
		REPORTERROR(eSecurity_UserAbort, "User cancelled passphrase");
		return 0;
	}

	EVP_PKEY* pkey = NULL;
	X509* signer = NULL;
	STACK_OF(X509)*	others = NULL;
	BIO* in_bio = NULL;
	BIO* out_bio = NULL;
#ifdef USE_CMS
	CMS* p7 = NULL;
#else
	PKCS7* p7 = NULL;
#endif

	// Convert fspec to file path
	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

	try
	{
		// load private key
		if(!(pkey = mCertificateManagerCOM->LoadPrivateKey(key, passphrase)))
		{
			cdstring temp = "Could not read private key for: ";
			temp += key;
			REPORTERROR(eSecurity_BadPassphrase, temp.c_str());
			throw -1L;
		}

		// Load cert to use for signature
		if(!(signer = mCertificateManagerCOM->FindCertificate(key, CCertificateManagerCOM::ePersonalCertificates)))
		{
			cdstring temp = "Could not read public key for: ";
			temp += key;
			REPORTERROR(eSecurity_UnknownError, temp.c_str());
			throw -1L;
		}

		// Generate certificate chain
		{
			// Start at signer's cert
			X509* node = signer;
			
			// Loop until self-signed detected or no issuer cert
			while(true)
			{
				// Self-signed test
				if (::X509_check_issued(node, node) == X509_V_OK)
					break;
				
				cdstring issuer;
				GetIssuer(node, issuer);

				// Try to get issuer cert
				X509* parent = mCertificateManagerCOM->FindCertificate(issuer, CCertificateManagerCOM::eCACertificates, CCertificateManagerCOM::eBySubject);
				if (parent == NULL)
					break;
				
				// Valid issuer test
				if (::X509_check_issued(parent, node) != X509_V_OK)
					break;
				
				// Create stack
				if (others == NULL)
					others = sk_X509_new_null();

				// Add valid intermediate cert
				sk_X509_push(others, parent);
				
				node = parent;
			}
		}

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
		// Make sure temp file is deleted when we return
		StRemoveFile _in_remove;

		// Do lendl -> CRLF conversion using temporary file
		cdstring in_tmp;
		if (lendl_convertCRLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not convert data file");
			throw -1L;
		}
		
		// If original file was not overwritten, replace original path with temp path
		if (!using_temp_files)
		{
			// Switch to use temp file as input path
			in_path.reset(in_tmp.grab_c_str());
			
			// Make sure temp file is deleted when we return
			_in_remove.set(in_path.get());
		}
#endif
		// Open data file to sign
		if (!(in_bio = ::BIO_new_file(in_path.get(), "rb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read data file");
			throw -1L;
		}
		
		// Open output file for detached signature
		if (!(out_bio = ::BIO_new_file(out_path.get(), "wb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create output file");
			throw -1L;
		}

		// Try to sign it
#ifdef USE_CMS
		p7 = ::CMS_sign(signer, pkey, others, in_bio, CMS_DETACHED);
#else
		p7 = ::PKCS7_sign(signer, pkey, others, in_bio, PKCS7_DETACHED);
#endif
		if (!p7)
		{
			REPORTERROR(eSecurity_UnknownError, "Signature failed");
			throw -1L;
		}

#ifdef USE_CMS
		B64write_CMS(out_bio, p7);
#else
		B64write_PKCS7(out_bio, p7);
#endif
	}
	catch(...)
	{
		// Catch all and fall through to clean up
		result = 0;
	}
	
	// Clean up
	if (p7)
#ifdef USE_CMS
		::CMS_free(p7);
#else
		::PKCS7_free(p7);
#endif
	if (out)
		::BIO_free(out_bio);
	if (in)
		::BIO_free(in_bio);
	if (others)
	{
		// Free each certificate first
		for (int i = 0; i < sk_X509_num(others); i++)
		{
			X509* x509 = sk_X509_value(others, i);
			::X509_free(x509);
		}
		::sk_X509_free(others);
	}
	if (signer)
		::X509_free(signer);
	if (key)
		::EVP_PKEY_free(pkey);

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

// Encrypt file
long CSMIMEPluginDLL::EncryptFileX(fspec in, const char** to, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	long result = 1;
	STACK_OF(X509)*	encrypted_to = NULL;
	BIO* in_bio = NULL;
	BIO* out_bio = NULL;
#ifdef USE_CMS
	CMS* p7 = NULL;
#else
	PKCS7* p7 = NULL;
#endif

	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Make sure temp file is deleted when we return
	StRemoveFile _in_remove;

	// Do lendl -> CRLF conversion using temporary file
	cdstring in_tmp;
	if (lendl_convertCRLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
		return 0;
	
	// If original file was not overwritten, replace original path with temp path
	if (!using_temp_files)
	{
		// Switch to use temp file as input path
		in_path.reset(in_tmp.grab_c_str());
		
		// Make sure temp file is deleted when we return
		_in_remove.set(in_path.get());
	}
#endif

	try
	{
		// Get certs to encrypt to
		encrypted_to = GetCertificates(to);

		// Open data file to sign
		if (!(in_bio = ::BIO_new_file(in_path.get(), "rb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read data file");
			throw -1L;
		}
		
		// Open output file for detached signature
		if (!(out_bio = ::BIO_new_file(out_path.get(), "wb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create output file");
			throw -1L;
		}

		// Try to encrypt it (use 3-DES cipher)
#ifdef USE_CMS
		p7 = ::CMS_encrypt(encrypted_to, in_bio, EVP_des_ede3_cbc(), 0);
#else
		p7 = ::PKCS7_encrypt(encrypted_to, in_bio, EVP_des_ede3_cbc(), 0);
#endif
		if (!p7)
		{
			REPORTERROR(eSecurity_UnknownError, "Encryption failed");
			throw -1L;
		}

#ifdef USE_CMS
		B64write_CMS(out_bio, p7);
#else
		B64write_PKCS7(out_bio, p7);
#endif
	}
	catch(...)
	{
		// Catch all and fall through to clean up
		result = 0;
	}

	// Clean up
	if (p7)
#ifdef USE_CMS
		::CMS_free(p7);
#else
		::PKCS7_free(p7);
#endif
	if (out)
		::BIO_free(out_bio);
	if (in)
		::BIO_free(in_bio);
	if (encrypted_to)
		::sk_X509_free(encrypted_to);

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

// Encrypt & sign file
long CSMIMEPluginDLL::EncryptSignFileX(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	// This does nothing - instead the calling app will split encrypt and sign into two operations Sign then Encrypt
	long result = 1;
	return result;
}

// Decrypt/verify file
long CSMIMEPluginDLL::DecryptVerifyFileX(fspec in, const char* sig, const char* in_from,
											fspec out, char** out_signedby, char** out_encryptedto,
											bool* success, bool* did_sig, bool* sig_ok, bool binary, bool using_temp_files)
{
	// Switch on sig to decrypt to verify
	return (sig ? VerifyFileX(in, sig, in_from, out_signedby, success, did_sig, sig_ok, binary, using_temp_files) :
					DecryptFileX(in, in_from, out, out_signedby, out_encryptedto, success, did_sig, sig_ok, binary, using_temp_files));
}

// Verify file
long CSMIMEPluginDLL::VerifyFileX(fspec in, const char* sig, const char* in_from, char** out_signedby,
									bool* success, bool* did_sig, bool* sig_ok, bool binary, bool using_temp_files)
{
	long result = 0;
	mData->mDidSig = true;
	mData->mDecryptPassphrase = false;

	BIO* sig_bio = NULL;
	X509_STORE* store = NULL;
#ifdef USE_CMS
	CMS* p7 = NULL;
#else
	PKCS7* p7 = NULL;
#endif
	BIO* in_bio = NULL;

	try
	{
		// Convert fspec to file path
		std::auto_ptr<char> in_path(ToPath(in));

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		FSSpec sig_spec;
		sig_spec.name[0] = 0;
		FSSpec* sig_tmp = &sig_spec;
#else
		char sig_tmp[1024];
		*sig_tmp = 0;
#endif
		// Make sure temp file is deleted when we return
		StRemoveFile _sig_remove;

		// Create temp sig file
		TempCreate(sig_tmp, NULL, sig);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (!*sig_spec.name)
#else
		if (!*sig_tmp)
#endif
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create signature file");
			throw -1L;
		}

		std::auto_ptr<char> sig_path(ToPath(sig_tmp));
		_sig_remove.set(sig_path.get());

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
		// Do lendl -> LF conversion
		if (lendl_convertLF(sig_path.get(), NULL, false) != 1)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not convert signature file");
			throw -1L;
		}
#endif

		// Open sig file
		if (!(sig_bio = ::BIO_new_file(sig_path.get(), "rb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not open signature file");
			throw -1L;
		}

		// Read in PKCS#7
#ifdef USE_CMS
		if(!(p7 = B64read_CMS(sig_bio)))
#else
		if(!(p7 = B64read_PKCS7(sig_bio)))
#endif
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read PKCS7 signature data");
			throw -1L;
		}

		// Write out signers certs to log if required
		if (mLogging)
		{
			// Get signers certs
#ifdef USE_CMS
			STACK_OF(X509)* signers = ::CMS_get0_signers(p7, NULL, 0);
#else
			STACK_OF(X509)* signers = ::PKCS7_get0_signers(p7, NULL, 0);
#endif
			for(int i = 0; i < sk_X509_num(signers); i++)
			{
				// get next cert
				X509* cert = sk_X509_value(signers, i);

				// Create BIO for output to memeory
				BIO* out = ::BIO_new(BIO_s_mem());
				if (!out)
				{
					::sk_X509_free(signers);
					REPORTERROR(eSecurity_UnknownError, "Could not log signers");
					throw -1L;
				}

				// Print X509 to BIO
				::X509_print(out, cert);

				// Copy BIO into cdstring
				cdstring temp = "Signed by:";
				temp += os_endl;
				BUF_MEM* buf = NULL;
				BIO_get_mem_ptr(out, &buf);
				temp.append(buf->data, BIO_number_written(out));
				temp.ConvertToOS();

				// Logit
				LogEntry(temp);

				// Done with BIO
				::BIO_free(out);
			}
			::sk_X509_free(signers);
		}

		// Create certificate verify store
		store = SetupCertificateStore();
		if (!store)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create certificate verification store");
			throw -1L;
		}

		// Open data file
		if (!(in_bio = ::BIO_new_file(in_path.get(), "rb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read data file");
			throw -1L;
		}

		// Do signature verify
#ifdef USE_CMS
		result = ::CMS_verify(p7, NULL, store, in_bio, NULL, 0);
#else
		result = ::PKCS7_verify(p7, NULL, store, in_bio, NULL, 0);
#endif
		*did_sig = true;
		*sig_ok = (result == 1);
		if (!result)
		{
			REPORTERROR(eSecurity_UnknownError, "Bad Signature");
			throw -1L;
		}

		{
			// Get signers certs
			cdstring from(in_from);
			cdstrvect signed_by;
#ifdef USE_CMS
			STACK_OF(X509)* signers = ::CMS_get0_signers(p7, NULL, 0);
#else
			STACK_OF(X509)* signers = ::PKCS7_get0_signers(p7, NULL, 0);
#endif
			for(int i = 0; i < sk_X509_num(signers); i++)
			{
				// Get next cert
				X509* cert = sk_X509_value(signers, i);

				// Get signed by details
				GetEmail(cert, signed_by);

				// Double-check validity of certificates
				if (!mCertificateManagerCOM->CheckUserCertificate(from, mData->mCertErrors, cert))
				{
					REPORTERROR(eSecurity_UnknownError, "User Refused Certificate");
					throw -1L;
				}
			}
			::sk_X509_free(signers);
			mData->mSignedByList = cdstring::ToArray(signed_by);
		}

		*success = true;
		*out_signedby = (char*) mData->mSignedByList;
	}
	catch(...)
	{
		result = 0;
		*success = false;
	}

	// Free resources
	if (sig_bio)
		::BIO_free(sig_bio);
	if (store)
		::X509_STORE_free(store);
	if (p7)
#ifdef USE_CMS
		::CMS_free(p7);
#else
		::PKCS7_free(p7);
#endif
	if (in_bio)
		::BIO_free(in_bio);

	return result;
}

// Decrypt file
long CSMIMEPluginDLL::DecryptFileX(fspec in, const char* in_from, fspec out, char** out_signedby, char** out_encryptedto, bool* success, bool* did_sig, bool* sig_ok, bool binary, bool using_temp_files)
{
	long result = 1;
	STACK_OF(X509)*	encrypted_to = NULL;
	BIO* in_bio = NULL;
	BIO* out_bio = NULL;
	X509_STORE* store = NULL;
#ifdef USE_CMS
	CMS* p7 = NULL;
#else
	PKCS7* p7 = NULL;
#endif
	EVP_PKEY* pkey = NULL;

	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

	try
	{
		// Open data file to decrypt
		if (!(in_bio = ::BIO_new_file(in_path.get(), "rb")))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read data file");
			throw -1L;
		}
		
		// Read in PKCS#7
#ifdef USE_CMS
		if(!(p7 = B64read_CMS(in_bio)))
#else
		if(!(p7 = B64read_PKCS7(in_bio)))
#endif
		{
			REPORTERROR(eSecurity_UnknownError, "Could not read PKCS7 data");
			throw -1L;
		}

		// Determine what type it is - we must have enveloped or enveloped and signed
#ifdef USE_CMS
		if (!CMS_type_is_signed(p7) && !CMS_type_is_enveloped(p7))
#else
		if (!PKCS7_type_is_signed(p7) && !PKCS7_type_is_enveloped(p7))
#endif
		{
			REPORTERROR(eSecurity_UnknownError, "PKCS7 object type cannot be processed");
			throw -1L;
		}
		
#ifdef USE_CMS
		bool signed_only = CMS_type_is_signed(p7);
		bool encrypt_only = CMS_type_is_enveloped(p7);
#else
		bool signed_only = PKCS7_type_is_signed(p7);
		bool encrypt_only = PKCS7_type_is_enveloped(p7);
#endif
		
		if (encrypt_only)
		{
			// Now get the certs to which this has been encrypted to
			encrypted_to = ::sk_X509_new_null();
#ifdef USE_CMS
			STACK_OF(CMS_RECIP_INFO)* recips = NULL;
#else
			STACK_OF(PKCS7_RECIP_INFO)* recips = NULL;
#endif
			recips = p7->d.enveloped->recipientinfo;

			cdstrvect signed_by_list;
			cdstrvect encrypted_to_list;
#ifdef USE_CMS
			for (int i = 0; i< sk_CMS_RECIP_INFO_num(recips); i++)
#else
			for (int i = 0; i< sk_PKCS7_RECIP_INFO_num(recips); i++)
#endif
			{
				// Get the recipient info
#ifdef USE_CMS
				CMS_RECIP_INFO* ri = sk_CMS_RECIP_INFO_value(recips, i);
#else
				PKCS7_RECIP_INFO* ri = sk_PKCS7_RECIP_INFO_value(recips, i);
#endif
				
				// Try to match recipient info with cert in our cache
				X509* x = mCertificateManagerCOM->FindCertificate(ri, CCertificateManagerCOM::ePersonalCertificates);
				if (x)
				{
					sk_X509_push(encrypted_to, x);
					
					// Get email address in cert
					GetEmail(x, encrypted_to_list);
				}
			}

			// Must have something to decrypt
			if (sk_X509_num(encrypted_to) == 0)
			{
				REPORTERROR(eSecurity_KeyUnavailable, "No private keys found");
				return 0;
			}

			// Get the user to choose which cert to decrypt with and return it
			cdstring passphrase;
			X509* decrypt_with = GetDecryptCertificate(encrypted_to, passphrase);
			if (!decrypt_with)
				return 0;

			// Get file to private key
			cdstring certfinger;
			GetFingerprint(decrypt_with, certfinger);

			// Load private key using fingerprint to avoid duplicate email problems
			if(!(pkey = mCertificateManagerCOM->LoadPrivateKey(certfinger, passphrase, CCertificateManagerCOM::eByFingerprint)))
			{
				cdstrvect certemail;
				GetEmail(decrypt_with, certemail);

				cdstring temp = "Could not read private key for: ";
				if (certemail.size() != 0)
					temp += certemail.front();
				REPORTERROR(eSecurity_BadPassphrase, temp.c_str());
				throw -1L;
			}

			// Open output file for decrypted data
			if (!(out_bio = ::BIO_new_file(out_path.get(), "wb")))
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create output file");
				throw -1L;
			}

			// Try to decrypt it
#ifdef USE_CMS
			if(!::CMS_decrypt(p7, pkey, decrypt_with, out_bio, 0))
#else
			if(!::PKCS7_decrypt(p7, pkey, decrypt_with, out_bio, 0))
#endif
			{
				REPORTERROR(eSecurity_UnknownError, "Decryption failed");
				throw -1L;
			}
			*success = (result == 1);
			*did_sig = !encrypt_only;

			if (!encrypt_only)
			{
				mData->mSignedByList = cdstring::ToArray(signed_by_list);
				*out_signedby = (char*) mData->mSignedByList;
			}

			mData->mEncryptedToList = cdstring::ToArray(encrypted_to_list);
			*out_encryptedto = (char*) mData->mEncryptedToList;
		}
		else if (signed_only)
		{
			// Write out signers certs to log if required
			if (mLogging)
			{
				// Get signers certs
#ifdef USE_CMS
				STACK_OF(X509)* signers = ::CMS_get0_signers(p7, NULL, 0);
#else
				STACK_OF(X509)* signers = ::PKCS7_get0_signers(p7, NULL, 0);
#endif
				for(int i = 0; i < sk_X509_num(signers); i++)
				{
					// get next cert
					X509* cert = sk_X509_value(signers, i);

					// Create BIO for output to memeory
					BIO* out = ::BIO_new(BIO_s_mem());
					if (!out)
					{
						::sk_X509_free(signers);
						REPORTERROR(eSecurity_UnknownError, "Could not log signers");
						throw -1L;
					}

					// Print X509 to BIO
					::X509_print(out, cert);

					// Copy BIO into cdstring
					cdstring temp = "Signed by:";
					temp += os_endl;
					BUF_MEM* buf = NULL;
					BIO_get_mem_ptr(out, &buf);
					temp.append(buf->data, BIO_number_written(out));
					temp.ConvertToOS();

					// Logit
					LogEntry(temp);

					// Done with BIO
					::BIO_free(out);
				}
				::sk_X509_free(signers);
			}

			// Create certificate verify store
			store = SetupCertificateStore();
			if (!store)
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create certificate verification store");
				throw -1L;
			}

			// Open output file for decrypted data
			if (!(out_bio = ::BIO_new_file(out_path.get(), "wb")))
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create output file");
				throw -1L;
			}

			// Do signature verify
#ifdef USE_CMS
			result = ::CMS_verify(p7, NULL, store, NULL, out_bio, 0);
#else
			result = ::PKCS7_verify(p7, NULL, store, NULL, out_bio, 0);
#endif
			*did_sig = true;
			*sig_ok = (result == 1);
			if (!result)
			{
				REPORTERROR(eSecurity_UnknownError, "Bad Signature");
				throw -1L;
			}

			{
				// Get signers certs
				cdstring from(in_from);
				cdstrvect signed_by;
#ifdef USE_CMS
				STACK_OF(X509)* signers = ::CMS_get0_signers(p7, NULL, 0);
#else
				STACK_OF(X509)* signers = ::PKCS7_get0_signers(p7, NULL, 0);
#endif
				for(int i = 0; i < sk_X509_num(signers); i++)
				{
					// Get next cert
					X509* cert = sk_X509_value(signers, i);

					// Get signed by details
					GetEmail(cert, signed_by);

					// Double-check validity of certificates
					if (!mCertificateManagerCOM->CheckUserCertificate(from, mData->mCertErrors, cert))
					{
						REPORTERROR(eSecurity_UnknownError, "User Refused Certificate");
						throw -1L;
					}
				}
				::sk_X509_free(signers);
				mData->mSignedByList = cdstring::ToArray(signed_by);
			}

			*success = true;
			*out_signedby = (char*) mData->mSignedByList;
		}
	}
	catch(...)
	{
		// Catch all and fall through to clean up
		result = 0;

		*success = false;
		*did_sig = false;
		*sig_ok = false;
	}

	// Clean up
	if (pkey)
		::EVP_PKEY_free(pkey);
	if (store)
		::X509_STORE_free(store);
	if (p7)
#ifdef USE_CMS
		::CMS_free(p7);
#else
		::PKCS7_free(p7);
#endif
	if (out_bio)
		::BIO_free(out_bio);
	if (in_bio)
		::BIO_free(in_bio);
	if (encrypted_to)
		::sk_X509_free(encrypted_to);

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

#pragma mark ____________________________Others

// MIME parameters

// top-level multipart
const char* cMIMEMultipartType = "multipart";
const char* cMIMEMultipartSigned = "signed";
const char* cMIMEMultipartEncrypted = "encrypted";

const char* cMIMEMultipartSignedParams[] = 
	{ "micalg", "sha1", "protocol", "application/pkcs7-signature", NULL };
const char* cMIMEMultipartEncryptedParams[] = 
	{ "protocol", "application/pkcs7-mime", NULL };
const char* cMIMEPKCS7MIMESignedParams[] = 
	{ "smime-type", "signed-data", NULL };
const char* cMIMEPKCS7MIMEEnvelopedParams[] = 
	{ "smime-type", "enveloped-data", NULL };
const char* cMIMENoParams[] = 
	{ NULL };

const char* cMIMEApplicationType = "application";
const char* cMIMEPKCSSigned = "pkcs7-signature";
const char* cMIMEPKCSEncrypted = "pkcs7-mime";
const char* cMIMEOctetStream = "octet-stream";

// Get MIME parameters for signing
long CSMIMEPluginDLL::GetMIMESign(SMIMEMultiInfo* params)
{
	SetMIMEDetails(&params->multipart,
					cMIMEMultipartType,
					cMIMEMultipartSigned,
					cMIMEMultipartSignedParams);
	
	SetMIMEDetails(&params->first,
					NULL,
					NULL,
					NULL);
	
	SetMIMEDetails(&params->second,
					cMIMEApplicationType,
					cMIMEPKCSSigned,
					NULL);
	
	return 1;
}

// Get MIME parameters for encryption
long CSMIMEPluginDLL::GetMIMEEncrypt(SMIMEMultiInfo* params)
{
	// S/MIME does not use multipart/encrypted
	// instead use applcation/pkcs7-mime at top-level
	SetMIMEDetails(&params->multipart,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					cMIMEPKCS7MIMEEnvelopedParams);
	
	SetMIMEDetails(&params->first,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					NULL);
	
	SetMIMEDetails(&params->second,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					cMIMEPKCS7MIMEEnvelopedParams);
	
	return 1;
}

// Get MIME parameters for encryption
long CSMIMEPluginDLL::GetMIMEEncryptSign(SMIMEMultiInfo* params)
{
	// S/MIME does not use multipart/encrypted
	// instead use applcation/pkcs7-mime at top-level
	SetMIMEDetails(&params->multipart,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					cMIMEPKCS7MIMEEnvelopedParams);
	
	SetMIMEDetails(&params->first,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					NULL);
	
	SetMIMEDetails(&params->second,
					cMIMEApplicationType,
					cMIMEPKCSEncrypted,
					cMIMEPKCS7MIMEEnvelopedParams);
	
	return 1;
}

// Check that MIME type is verifiable by this plugin
long CSMIMEPluginDLL::CanVerifyThis(const char* type)
{
	// Return 0 if it can verify, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	if (!::strcmpnocase(type, "application/pkcs7-signature") ||
		!::strcmpnocase(type, "application/x-pkcs7-signature"))
		return 0;
	else
		return 1;
}

// Check that MIME type is decryptable by this plugin
long CSMIMEPluginDLL::CanDecryptThis(const char* type)
{
	// Return 0 if it can decrypt, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	if (!::strcmpnocase(type, "application/pkcs7-mime") ||
		!::strcmpnocase(type, "application/x-pkcs7-mime"))
		return 0;
	else
		return 1;
}

// Get last textual error
long CSMIMEPluginDLL::GetLastError(long* err_no, char** error)
{
	*err_no = mData->mErrno;
	*error = mData->mErrstr.c_str_mod();

	return 0;
}

// Set MIME details
void CSMIMEPluginDLL::SetMIMEDetails(SMIMEInfo* mime, const char* type, const char* subtype, const char** params)
{
	mime->type = type;
	mime->subtype = subtype;
	mime->params = params;
}

long CSMIMEPluginDLL::GetSignKeyPassphrase(const char* key, char* passphrase)
{
	if (key && *key && passphrase)
		return GetSignPassphrase(key, passphrase);
	else
		return 0;
}

#pragma mark ____________________________openssl SMIME Utils

// Convert pkcs7 data into base64 string
#ifdef USE_CMS
int CSMIMEPluginDLL::B64write_CMS(BIO* bio, CMS* p7)
#else
int CSMIMEPluginDLL::B64write_PKCS7(BIO* bio, PKCS7* p7)
#endif
{
	BIO* b64;
	if(!(b64 = ::BIO_new(BIO_f_base64())))
		return 0;

	bio = ::BIO_push(b64, bio);

#ifdef USE_CMS
	::i2d_CMS_bio(bio, p7);
#else
	::i2d_PKCS7_bio(bio, p7);
#endif

	BIO_flush(bio);
	bio = ::BIO_pop(bio);

	::BIO_free(b64);
	return 1;
}

// Convert base64 string into pkcs7 data 
#ifdef USE_CMS
CMS* CSMIMEPluginDLL::B64read_CMS(BIO* bio)
#else
PKCS7* CSMIMEPluginDLL::B64read_PKCS7(BIO* bio)
#endif
{
	BIO* b64;
	if(!(b64 = ::BIO_new(BIO_f_base64())))
		return 0;

	bio = ::BIO_push(b64, bio);

#ifdef USE_CMS
	CMS* p7;
	if(!(p7 = ::d2i_CMS_bio(bio, NULL))) 
#else
	PKCS7* p7;
	if(!(p7 = ::d2i_PKCS7_bio(bio, NULL))) 
#endif
	{
		::BIO_free(b64);
		return 0;;
	}

	BIO_flush(bio);
	bio = ::BIO_pop(bio);

	::BIO_free(b64);
	return p7;
}

STACK_OF(X509)* CSMIMEPluginDLL::GetCertificates(const char** to)
{
	STACK_OF(X509)* certs = NULL;
	
	try
	{
		// Create cert stack
		certs = ::sk_X509_new_null();
		if (!certs)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create certificate stack");
			throw -1L;
		}
		
		// Iterate over each key
		const char** p = to;
		while(*p)
		{
			// Lookup in cache
			X509* x = mCertificateManagerCOM->FindCertificate(*p, CCertificateManagerCOM::eUserCertificates);
			if (x == NULL)
				x = mCertificateManagerCOM->FindCertificate(*p, CCertificateManagerCOM::ePersonalCertificates);
			
			// Fail if none found
			if (x == NULL)
			{
				cdstring temp = "Could not find certificate for: ";
				temp += *p;
				REPORTERROR(eSecurity_UnknownError, temp.c_str());
				throw -1L;
			}
			
			// Add to stack
	    	sk_X509_push(certs, x);
	    	
	    	p++;
		}
	}
	catch(...)
	{
		// Clean up
		if (certs != NULL)
			::sk_X509_free(certs);
		
		// Throw up
		throw;
	}
	
	return certs;
}

// Get a decrypt certificate from the user
X509* CSMIMEPluginDLL::GetDecryptCertificate(STACK_OF(X509)* certs, cdstring& passphrase)
{
	// Get name from each cert
	cdstrvect cert_names;
	for(int i = 0; i < sk_X509_num(certs); i++)
	{
		// Get next cert
		X509* cert = sk_X509_value(certs, i);

		// Get subject
		cdstring subject;
		GetSubject(cert, subject);
		
		// Convert to cn=
		const char* p = ::strstrnocase(subject, "cn=");
		cdstring cn;
		if (p)
			cn = p;
		
		// Get hash
		cdstring hash;
		GetHash(cert, hash);
		
		cn += " (";
		cn += hash;
		cn += ")";
		
		cert_names.push_back(cn);
	}
	
	// Create array of keys
	std::auto_ptr<const char*> users(cdstring::ToArray(cert_names, false));

	// Get passphrase
	unsigned long chosen;
	passphrase.reserve(256);
	if (GetPassphrase(users.get(), passphrase, chosen))
		// Get the certificate
		return sk_X509_value(certs, chosen);
	else
		return NULL;
}

// Setup the certificate store for verification
X509_STORE* CSMIMEPluginDLL::SetupCertificateStore()
{
	// Create the store
	X509_STORE* store = ::X509_STORE_new();
	if (store == NULL)
		return NULL;

	// Set verify callback for store
	X509_STORE_set_verify_cb_func(store, VerifyCallback);

	// Set the ex data to this plugin
	::CRYPTO_set_ex_data(&store->ex_data, 0, this);

	// Add a directory lookup items
	mCertificateManagerCOM->LoadSMIMERootCerts(store);

	return store;
}

int CSMIMEPluginDLL::VerifyCallback(int ok, X509_STORE_CTX *ctx)
{
	// Get pointer to plugin
	CSMIMEPluginDLL* plugin = (CSMIMEPluginDLL*) CRYPTO_get_ex_data(&ctx->ctx->ex_data, 0);
	
	return plugin ? plugin->Verify(ok, ctx) : 0;
}

int CSMIMEPluginDLL::Verify(int ok, X509_STORE_CTX *ctx)
{
	// Add error to list of errors for this certificate
	if (ctx->error != X509_V_OK)
		AddCertError(ctx->error);

	// Always accept the certificate here - we will check the list
	// of errors later and kill the connection if its not acceptable
	return 1;
}

// Add error to list of current certificate errors
void CSMIMEPluginDLL::AddCertError(int err)
{
	// Add to list if unique
	std::vector<int>::const_iterator found = std::find(mData->mCertErrors.begin(), mData->mCertErrors.end(), err);
	if (found == mData->mCertErrors.end())
		mData->mCertErrors.push_back(err);
}

bool CSMIMEPluginDLL::GetEmail(X509* cert, cdstrvect& email) const
{
	GetNIDs(cert, GEN_EMAIL, NID_pkcs9_emailAddress, email);

	return true;
}

bool CSMIMEPluginDLL::GetIssuer(X509* cert, cdstring& issuer) const
{
	// Get subject text
	char x509_buf[BUFSIZ];
	char* str = X509_NAME_oneline(X509_get_issuer_name(cert), x509_buf, BUFSIZ);
	if (str)
	{
		issuer = str;
		return true;
	}
	else
		return false;
}

bool CSMIMEPluginDLL::GetSubject(X509* cert, cdstring& subject) const
{
	// Get subject text
	char x509_buf[BUFSIZ];
	char* str = X509_NAME_oneline(X509_get_subject_name(cert), x509_buf, BUFSIZ);
	if (str)
	{
		subject = str;
		return true;
	}
	else
		return false;
}

bool CSMIMEPluginDLL::GetHash(X509* cert, cdstring& hash) const
{
	hash.reserve(32);
	::sprintf(hash.c_str_mod(), "%08lx", X509_subject_name_hash(cert));
	return true;
}

bool CSMIMEPluginDLL::GetFingerprint(X509* cert, cdstring& finger) const
{
	// Need to make sure SHA1 is actually calculated
	// Can do this by compare to self
	::X509_cmp(cert, cert);

	// Now get hex form of SHA1
	finger.reserve(2 * SHA_DIGEST_LENGTH);
	char* temp = finger.c_str_mod();
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		*temp++ = cHexChar[cert->sha1_hash[i] >> 4];
		*temp++ = cHexChar[cert->sha1_hash[i] & 0x0F];
	}
	*temp = 0;

	return true;
}

cdstring CSMIMEPluginDLL::GetNID(X509* cert, int gen_type, int nid) const
{
	cdstrvect temp;
	cdstring data;
	GetNIDs(cert, gen_type, nid, temp);
	if (temp.size() != 0)
		data = temp.front();
	
	return data;
}

void CSMIMEPluginDLL::GetNIDs(X509* cert, int gen_type, int nid, cdstrvect& results) const
{
	// First make sure crt is cached
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

void CSMIMEPluginDLL::ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno)
{
	// Get default plugin error string
	cdstring err_buf = GetName();
	err_buf += " Error: ";
	err_buf += errtxt;


	BIO* berr1 = ::BIO_new(::BIO_s_mem());
	BIO* berr2 = ::BIO_new(::BIO_s_mem());
	ERR_quick_errors(berr1, berr2);

	// Get long error string for logging
	if (mLogging)
	{
		cdstring serr(err_buf);
		
		// Add function, file, line information
		serr += os_endl;
		serr += func;
		serr += "  ";
		serr += file;
		serr += ",";
		serr += cdstring((unsigned long)lineno);
		serr += os_endl;

		if (berr2)
		{
			const char* s = NULL;
			int slen = ::BIO_get_mem_data(berr2, &s);
			if (slen)
			{
				serr += os_endl;
				serr.append(s, slen);
				serr.ConvertEndl();
			}
		}

		LogEntry(serr);
	}
	
	// Get short error string for display to user
	{
		cdstring serr(err_buf);
		if (berr1)
		{
			const char* s = NULL;
			int slen = ::BIO_get_mem_data(berr1, &s);
			if (slen)
			{
				serr += os_endl;
				serr.append(s, slen);
				serr.ConvertEndl();
			}
		}
		SetLastError(err_no, serr);
	}

	if (berr2)
		::BIO_free(berr2);
	if (berr1)
		::BIO_free(berr1);
}

void CSMIMEPluginDLL::ERR_quick_errors(BIO* bp1, BIO* bp2)
{
	unsigned long l;
	const char *file, *data;
	int line,flags;

	while ((l = ::ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
	{
		char buf[256];
		char buf2[256];

		if (bp2)
		{
			::ERR_error_string_n(l, buf, sizeof buf);
			::snprintf(buf2, 256, "%s:%s:%d:", buf, file, line);
			::BIO_write(bp2, buf2, ::strlen(buf2));
			::BIO_write(bp2, "\n", 1);
		}

		if (bp1)
		{
			::snprintf(buf, 256, "%s", ::ERR_reason_error_string(l));
			::BIO_write(bp1, buf, ::strlen(buf));
			::BIO_write(bp1, "\n", 1);
		}
	}
}
