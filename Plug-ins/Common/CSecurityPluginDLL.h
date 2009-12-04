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

// CSecurityPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based security plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the security DLL being called by Mulberry.
//
// History:
// CD:	 04-May-1998:	Created initial header and implementation.
//

#ifndef __SECURITY_PLUGIN_MULBERRY__
#define __SECURITY_PLUGIN_MULBERRY__

#include "CPluginDLL.h"

#include "cdstring.h"

#include <stdlib.h>

const int cMaxAuthStringLength = 256;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define fspec FSSpec*
#else
#define fspec const char*
#endif

#if __dest_os == __mac_os
	typedef CFragConnectionID TDLLInstance;
#elif __dest_os == __mac_os_x
	typedef CFBundleRef TDLLInstance;
#elif __dest_os == __win32_os
	typedef HANDLE TDLLInstance;
#elif __dest_os == __linux_os
	typedef void* TDLLInstance;
#else
#error __dest_os
#endif

// Classes
class CCertificateManagerCOM;

class CSecurityPluginDLL : public CPluginDLL
{
public:

	enum ESecurityPluginCall
	{
		// Derived plug-in calls
		eSecuritySignData = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
		eSecurityEncryptData,
		eSecurityEncryptSignData,
		eSecurityDecryptVerifyData,
		eSecuritySignFile,
		eSecurityEncryptFile,
		eSecurityEncryptSignFile,
		eSecurityDecryptVerifyFile,
		
		eSecurityDisposeData,
		
		eSecurityGetLastError,

		eSecurityGetMIMEParamsSign,
		eSecurityGetMIMEParamsEncrypt,
		eSecurityGetMIMEParamsEncryptSign,
		
		eSecuritySetCallback,
		
		eSecurityCanVerifyThis,
		eSecurityCanDecryptThis,
		
		eSecuritySetSMIMEContext
	};

	enum ESecurityPluginCallback
	{
		eCallbackPassphrase = 1
	};

	enum ESecurityError
	{
		eSecurity_NoErr = 0,
		eSecurity_UserAbort,
		eSecurity_BadPassphrase,
		eSecurity_KeyUnavailable,
		eSecurity_InvalidSignature,
		eSecurity_DubiousKey,
		eSecurity_UnknownError = -1L
	};

	typedef bool (*CallbackProcPtr)(ESecurityPluginCallback type, void* data);

	struct SSignData
	{
		const char* mInputData;
		const char* mKey;
		char** mOutputData;
		unsigned long* mOutputDataLength;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptData
	{
		const char* mInputData;
		const char** mKeys;
		char** mOutputData;
		unsigned long* mOutputDataLength;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptSignData
	{
		const char* mInputData;
		const char** mKeys;
		const char* mSignKey;
		char** mOutputData;
		unsigned long* mOutputDataLength;
		bool mUseMIME;
		bool mBinary;
	};

	struct SDecryptVerifyData
	{
		const char* mInputData;
		const char* mInputSignature;
		const char* mInputFrom;
		char** mOutputData;
		unsigned long* mOutputDataLength;
		char** mOutputSignedby;
		char** mOutputEncryptedto;
		bool* mSuccess;
		bool* mDidSig;
		bool* mSigOK;
		bool mBinary;
	};

	struct SSignFile
	{
		fspec mInputFile;
		const char* mKey;
		fspec mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptFile
	{
		fspec mInputFile;
		const char** mKeys;
		fspec mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptSignFile
	{
		fspec mInputFile;
		const char** mKeys;
		const char* mSignKey;
		fspec mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SDecryptVerifyFile
	{
		fspec mInputFile;
		const char* mInputSignature;
		const char* mInputFrom;
		fspec mOutputFile;
		char** mOutputSignedby;
		char** mOutputEncryptedto;
		bool* mSuccess;
		bool* mDidSig;
		bool* mSigOK;
		bool mBinary;
	};

	struct SGetLastError
	{
		long* errnum;
		char** error;
	};

	struct SMIMEInfo
	{
		const char* type;
		const char* subtype;
		const char** params;
		
		SMIMEInfo()
			{ type = NULL; subtype = NULL; params = NULL; }
	};

	struct SMIMEMultiInfo
	{
		SMIMEInfo multipart;
		SMIMEInfo first;
		SMIMEInfo second;
	};

	struct SCallbackPassphrase
	{
		const char** users;
		char* passphrase;
		unsigned long chosen;
	};

	struct SSMIMEContext
	{
		TDLLInstance			mDLL;
		CCertificateManagerCOM*	mCertMgr;;
	};

	// Actual plug-in class

	CSecurityPluginDLL();
	virtual ~CSecurityPluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

	// Entry codes
	
	// Operations on in memory data
	virtual long 	SignData(const char* in,					// Sign data
								const char* key,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary) = 0;
	virtual long 	EncryptData(const char* in,					// Encrypt data
								const char** to,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary) = 0;
	virtual long 	EncryptSignData(const char* in,				// Encrypt & sign data
								const char** to,
								const char* key,
								char** out,
								unsigned long* out_len,
								bool useMime,
								bool binary) = 0;
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
								bool binary) = 0;

	// Operations on files
	virtual long 	SignFile(fspec in,							// Sign file
								const char* key,
								fspec out,
								bool useMime,
								bool binary) = 0;
	virtual long 	EncryptFile(fspec in,						// Encrypt file
								const char** to,
								fspec out,
								bool useMime,
								bool binary) = 0;
	virtual long 	EncryptSignFile(fspec in,					// Encrypt & sign file
								const char** to,
								const char* key,
								fspec out,
								bool useMime,
								bool binary) = 0;
	virtual long 	DecryptVerifyFile(fspec in,					// Decrypt/verify file
								const char* sig,
								const char* in_from,
								fspec out,
								char** out_signedby,
								char** out_encryptedto,
								bool* success,
								bool* did_sig,
								bool* sig_ok,
								bool binary) = 0;

	virtual long	DisposeData(const char* data) = 0;

	virtual long	GetMIMESign(SMIMEMultiInfo* params) = 0;		// Get MIME parameters for signing
	virtual long	GetMIMEEncrypt(SMIMEMultiInfo* params) = 0;		// Get MIME parameters for encryption
	virtual long	GetMIMEEncryptSign(SMIMEMultiInfo* params) = 0;	// Get MIME parameters for encryption

	virtual long	CanVerifyThis(const char* type) = 0;			// Check that MIME type is verifiable by this plugin
	virtual long	CanDecryptThis(const char* type) = 0;			// Check that MIME type is decryptable by this plugin

	virtual long	GetLastError(long* errnum, char** error) = 0;	// Get last textual error

	virtual void	SetCallback(CallbackProcPtr proc)				// Set callback into Mulberry
		{ mCallback = proc; }

protected:
	CallbackProcPtr mCallback;

	// These should be returned by specific sub-class
	virtual long GetDataLength(void) const					// Returns data of plug-in
		{ return 0; }
	virtual const void* GetData(void) const					// Returns data of plug-in
		{ return NULL; }
	
	// Callbacks to application
	virtual bool GetSignPassphrase(const char* key, char* passphrase);
	virtual bool GetPassphrase(const char** users, char* passphrase, unsigned long& chosen);
	
	// Utility functions used by derived classes
	virtual void ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno);
	virtual void SetLastError(long err, const char* errstr) = 0;

	void TempCreate(fspec in_tmp, fspec out_tmp, const char* in);			// Create temp files for data processing
	void TempRead(fspec out_tmp, char** out, unsigned long* out_len);	// Read temp output file into buffer
	char* ToPath(fspec spec);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	bool MakeTemp(fspec ftemp);
#elif __dest_os == __win32_os
	int mkstemp(fspec ftemp, bool open_fd = true);
#endif
#if __dest_os == __win32_os || __dest_os == __mac_os || __dest_os == __mac_os_x
	long lendl_convertLF(const char* spec, cdstring* tspec, bool to_lendl);
	long lendl_convertCRLF(const char* spec, cdstring* tspec, bool to_lendl);
	long lendl_convert(const char* spec, cdstring* tspec, bool to_lendl, bool to_LF);
#endif

};

#endif
