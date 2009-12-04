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

// CSecurityPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based security plug-ins in Mulberry.
//
// History:
// CD:	 04-May-1998:	Created initial header and implementation.
//

#ifndef __CSECURITYPLUGIN__MULBERRY__
#define __CSECURITYPLUGIN__MULBERRY__

#include "CPlugin.h"

// Classes
class CMessage;
class CMessageCryptoInfo;
class CAttachment;

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

class CCertificateManagerCOM;

class CSecurityPlugin : public CPlugin
{

public:

	enum ESecureMessage
	{
		eNone = 0,
		eSign,
		eEncrypt,
		eEncryptSign
	};

	// Actual plug-in class

	CSecurityPlugin(fspec files) :
		CPlugin(files) {}
	virtual ~CSecurityPlugin() {}

	static void RegisterSecurityPlugin(CSecurityPlugin* plugin);
	static CSecurityPlugin* GetRegisteredPlugin(const cdstring& descriptor);
	static CSecurityPlugin* GetDefaultPlugin();

	// Initialise/terminate
	virtual bool VerifyVersion() const;					// Make sure version matches
	virtual void LoadPlugin();							// Set callback on each load

	// Calls codes

	// Signing/encryption

	// Top call
	bool ProcessMessage(CMessage* msg, ESecureMessage mode, const char* key);

protected:
	// Operations on entire body
	void ProcessBody(CMessage* msg, ESecureMessage mode, const char* key);
	bool FileBody(const CAttachment* part, unsigned long& size) const;

	// Operations on single parts
	bool CanSecureAttachment(const CAttachment* part) const;
	bool FileAttachment(const CAttachment* part) const;
	void ProcessAttachment(CMessage* msg, CAttachment* part, ESecureMessage mode, const char* key);

	// Data process from message
	void Process(const CMessage* msg,
						ESecureMessage mode,
						const char* in,
						fspec fin,
						const char* key,
						char** out,
						fspec fout,
						unsigned long* out_len,
						bool useMIME,
						bool binary);

	bool DoesEncryptSignAllInOne() const;
	bool UseMIME() const;

	// Verifying/decryption
public:
	static bool VerifyDecryptPart(CMessage* msg, CAttachment* part, CMessageCryptoInfo& info);

protected:
	typedef std::map<cdstring, CSecurityPlugin*> SSecurityPluginHandlers;
	static SSecurityPluginHandlers	sSecurityPlugins;
	static cdstring sPreferredPlugin;

	static cdstrmap sCanVerify;
	static cdstrmap sCanDecrypt;

	static CSecurityPlugin* GetVerifyPlugin(const cdstring& type);
	static CSecurityPlugin* GetDecryptPlugin(const cdstring& type);

	bool VerifyDecryptPartInternal(CMessage* msg, CAttachment* part, CMessageCryptoInfo& info);

	bool FileVerifyDecrypt(const CMessage* msg) const;

	bool VerifyMessage(CMessage* msg, CMessageCryptoInfo& info);
	bool DecryptMessage(CMessage* msg, CMessageCryptoInfo& info, bool use_multi_part);

	long HandleError(CMessageCryptoInfo* info = NULL);

	enum ESecurityPluginCall
	{
		// Derived plug-in calls
		eSecuritySignData = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
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

	typedef bool (*CallbackProcPtr)(ESecurityPluginCallback type, void* data);

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

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

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
		fspec_old mInputFile;
		const char* mKey;
		fspec_old mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptFile
	{
		fspec_old mInputFile;
		const char** mKeys;
		fspec_old mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SEncryptSignFile
	{
		fspec_old mInputFile;
		const char** mKeys;
		const char* mSignKey;
		fspec_old mOutputFile;
		bool mUseMIME;
		bool mBinary;
	};

	struct SDecryptVerifyFile
	{
		fspec_old mInputFile;
		const char* mInputSignature;
		const char* mInputFrom;
		fspec_old mOutputFile;
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
		CCertificateManagerCOM*	mCertMgr;
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	static cdstrmap sPassphrases;
	static cdstring sLastPassphraseUID;
	
	// Utilities
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	void CreateTempFile(PPx::FSObject* ftemp, ESecureMessage mode, const cdstring& name);
#else
	void CreateTempFile(cdstring& ftemp, ESecureMessage mode, const cdstring& name);
#endif
	void ApplyMIME(CAttachment* part, SMIMEInfo* info);

	// Operations on in memory da¡ta
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

	virtual long	GetLastError(long* errnum, char** error);	// Get last error from plugin

	virtual long	GetMIMESign(SMIMEMultiInfo* params);		// Get MIME parameters for signing
	virtual long	GetMIMEEncrypt(SMIMEMultiInfo* params);		// Get MIME parameters for encryption
	virtual long	GetMIMEEncryptSign(SMIMEMultiInfo* params);	// Get MIME parameters for encryption

	virtual long	CanVerifyThis(const char* type);			// Check that MIME type is verifiable by this plugin
	virtual long	CanDecryptThis(const char* type);			// Check that MIME type is decryptable by this plugin

	virtual long	SetCallback();								// Set callback into Mulberry

	virtual long	SetContext();								// Set COM/dll into Mulberry

	// Callbacks to application
	static bool Callback(ESecurityPluginCallback type, void* data);

public:
	static bool GetPassphrase(const char** users, char* passphrase, unsigned long& chosen);
	static void ClearLastPassphrase();
};

#endif
