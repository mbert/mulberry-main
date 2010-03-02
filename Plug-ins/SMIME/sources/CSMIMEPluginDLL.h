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

// CSMIMEPluginDLL.h
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

#ifndef __SMIME_PLUGIN_MULBERRY__
#define __SMIME_PLUGIN_MULBERRY__

#include "CSecurityPluginDLL.h"
#include <string.h>
#include "cdstring.h"

#if __dest_os == __win32_os
//#define USE_CMS
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#else
#include "openssl_.h"
#endif

#ifdef USE_CMS
#include <openssl/cms.h>
#endif

// Set various macros for different types of behaviour

// Classes
class CCertificateManagerCOM;

class CSMIMEPluginDLL : public CSecurityPluginDLL
{
public:
	// Actual plug-in class

	CSMIMEPluginDLL();
	virtual ~CSMIMEPluginDLL();
	
	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run
			long	SetContext(SSMIMEContext* context);	// Set dll contexts

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

	// Operations on in memory data
	virtual long 	SignData(const char* in,					// Sign data
								const char* key,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary);
	virtual long 	EncryptData(const char* in,					// Encrypt data
								const char** to,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary);
	virtual long 	EncryptSignData(const char* in,				// Encrypt & sign data
								const char** to,
								const char* key,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary);
	virtual long 	DecryptVerifyData(const char* in,			// Decrypt/verify data
								const char* sig,
								const char* in_from,
								char** out,
								unsigned long* out_len,
								char** out_signedby,
								char** out_encryptedto,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary);

	// Operations on files
	virtual long 	SignFile(fspec in,							// Sign file
								const char* key,
								fspec out,
								bool useMime,
								bool binary);
	virtual long 	EncryptFile(fspec in,						// Encrypt file
								const char** to,
								fspec out,
								bool useMime,
								bool binary);
	virtual long 	EncryptSignFile(fspec in,					// Encrypt & sign file
								const char** to,
								const char* key,
								fspec out,
								bool useMime,
								bool binary);
	virtual long 	DecryptVerifyFile(fspec in,					// Decrypt/verify file
								const char* sig,
								const char* in_from,
								fspec out,
								char** out_signedby,
								char** out_encryptedto,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary);

	virtual long	DisposeData(const char* data);

	virtual long	GetMIMESign(SMIMEMultiInfo* params);		// Get MIME parameters for signing
	virtual long	GetMIMEEncrypt(SMIMEMultiInfo* params);		// Get MIME parameters for encryption
	virtual long	GetMIMEEncryptSign(SMIMEMultiInfo* params);	// Get MIME parameters for encryption

	virtual long	CanVerifyThis(const char* type);			// Check that MIME type is verifiable by this plugin
	virtual long	CanDecryptThis(const char* type);			// Check that MIME type is decryptable by this plugin

	virtual long	GetLastError(long* err_no, char** error);	// Get last textual error

protected:
	struct SCertificateLookup
	{
		cdstring mFilename;
		cdstring mSubject;
		cdstring mEmail;
		cdstring mHash;
		cdstring mFingerprint;
	};
	typedef std::vector<SCertificateLookup> SLookupList;

	struct SData
	{
		const char**	mSignedByList;
		const char**	mEncryptedToList;
		cdstring		mErrstr;
		long			mErrno;
		bool			mDecryptPassphrase;
		bool			mDidSig;
		std::vector<int>		mCertErrors;
	};
	SData* mData;
	CCertificateManagerCOM* mCertificateManagerCOM;
	cdstring mStdError;

	// These should be returned by specific sub-class
	virtual const char* GetName() const;				// Returns the name of the plug-in
	virtual long GetVersion() const;					// Returns the version number of the plug-in
	virtual EPluginType GetType() const;				// Returns the type of the plug-in
	virtual const char* GetManufacturer() const;		// Returns manufacturer of plug-in
	virtual const char* GetDescription() const;			// Returns description of plug-in
	
	void PreparePGP();									// Prepare PGP context
	void FinishPGP();									// Finish with PGP context

	void PrepareFiles(fspec in, fspec out);				// Prepare file references
	void FinishFiles();									// Finish with file references

	void SetMIMEDetails(SMIMEInfo* mime,				// Set MIME details
						const char* type,
						const char* subtype,
						const char** params);

	virtual void SetLastError(long err, const char* errstr)
		{ mData->mErrno = err; mData->mErrstr = errstr; }

	// Local operations on files
	long 	SignFileX(fspec in,							// Sign file
								const char* key,
								fspec out,
								bool useMime,
								bool binary,
								bool using_temp_files);
	long 	EncryptFileX(fspec in,						// Encrypt file
								const char** to,
								fspec out,
								bool useMime,
								bool binary,
								bool using_temp_files);
	long 	EncryptSignFileX(fspec in,					// Encrypt & sign file
								const char** to,
								const char* key,
								fspec out,
								bool useMime,
								bool binary,
								bool using_temp_files);
	long 	DecryptVerifyFileX(fspec in,					// Decrypt/verify file
								const char* sig,
								const char* in_from,
								fspec out,
								char** out_signedby,
								char** out_encryptedto,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary,
								bool using_temp_files);
	long 	VerifyFileX(fspec in,							// Verify file
								const char* sig,
								const char* in_from,
								char** out_signedby,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary,
								bool using_temp_files);
	long 	DecryptFileX(fspec in,							// Decrypt file
								const char* in_from,
								fspec out,
								char** out_signedby,
								char** out_encryptedto,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary,
								bool using_temp_files);

	long GetSignKeyPassphrase(const char* key, char* passphrase);

	// openssl S/MIME utils
#ifdef USE_CMS
	int B64write_CMS(BIO* bio, CMS* p7);
	CMS* B64read_CMS(BIO* bio);
#else
	int B64write_PKCS7(BIO* bio, PKCS7* p7);
	PKCS7* B64read_PKCS7(BIO* bio);
#endif

	STACK_OF(X509)* GetCertificates(const char** to);
	X509* GetDecryptCertificate(STACK_OF(X509)* certs, cdstring& passphrase);
	X509_STORE* SetupCertificateStore();

	static int VerifyCallback(int ok, X509_STORE_CTX *ctx);
	int Verify(int ok, X509_STORE_CTX *ctx);
	void AddCertError(int err);

	bool GetEmail(X509* cert, cdstrvect& email) const;
	bool GetIssuer(X509* cert, cdstring& issuer) const;
	bool GetSubject(X509* cert, cdstring& subject) const;
	bool GetHash(X509* cert, cdstring& hash) const;
	bool GetFingerprint(X509* cert, cdstring& finger) const;
	cdstring GetNID(X509* cert, int gen_type, int nid) const;
	void GetNIDs(X509* cert, int gen_type, int nid, cdstrvect& results) const;
	
	virtual void ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno);
	void ERR_quick_errors(BIO* bp1, BIO* bp2);
};

#endif
