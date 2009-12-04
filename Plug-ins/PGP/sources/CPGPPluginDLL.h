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

// CPGPPluginDLL.h
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

#pragma once

#include "CSecurityPluginDLL.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define PGP_MACINTOSH	1
#define _MAC	1
#elif __dest_os == __win32_os
#define PGP_WIN32		1
#endif
#define PGP_DEBUG		0
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <PGP/pgpFeatures.h>
#include <PGP/pgpEncode.h>
#include <PGP/pgpRandomPool.h>
#include <PGP/pgpKeys.h>
#include <PGP/pgpErrors.h>
#include <PGP/pgpUtilities.h>
#else
#include "pgpFeatures.h"
#include "pgpEncode.h"
#include "pgpRandomPool.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#endif

// Classes
class CPGPPluginDLL : public CSecurityPluginDLL
{
	typedef PGPError (*CallbackPtr)(PGPContextRef pgpContext, PGPEvent* event, PGPUserValue userValue);

public:

	// Actual plug-in class

	CPGPPluginDLL();
	virtual ~CPGPPluginDLL();
	
	// Entry codes
	virtual void	Initialise(void);					// Initialisation
	virtual bool	CanRun(void);						// Test whether plug-in can run

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

	virtual long	GetLastError(long* errnum, char** error);	// Get last textual error

protected:
	enum EPGPPluginState
	{
		eError= 0,
		eDone
	};

	// Various stack-based PGP items
	template<class T> class CPGPItem
	{
	public:
		CPGPItem() {}
		~CPGPItem() {}
	
		operator T*()
			{ return &mData; }
		operator T&()
			{ return mData; }
		
		T& get()
			{ return mData; }
	protected:
		T mData;
	};

	class CPGPKeySetRef : public CPGPItem<PGPKeySetRef>
	{
	public:
		CPGPKeySetRef();
		~CPGPKeySetRef();
	};

	class CPGPFilterRef : public CPGPItem<PGPFilterRef>
	{
	public:
		CPGPFilterRef();
		~CPGPFilterRef();
	};

	class CPGPKeyListRef : public CPGPItem<PGPKeyListRef>
	{
	public:
		CPGPKeyListRef();
		~CPGPKeyListRef();
	};

	class CPGPKeyIterRef : public CPGPItem<PGPKeyIterRef>
	{
	public:
		CPGPKeyIterRef();
		~CPGPKeyIterRef();
	};

	class CPGPFileSpecRef : public CPGPItem<PGPFileSpecRef>
	{
	public:
		CPGPFileSpecRef();
		~CPGPFileSpecRef();
	};

	EPGPPluginState mState;
	cdstring		mLastError;
	long			mLastErrno;
	cdstring		mMissingItem;
	cdstring		mErrorFunction;
	cdstring		mErrorFile;
	unsigned long	mErrorLine;
	bool			mGotPassphrase;
	bool			mVerified;
	bool			mDidSignature;
	PGPContextRef	mContextRef;
	PGPKeyDBRef 	mKeyDBRef;
	PGPKeySetRef 	mFilteredSetRef;
	PGPFileSpecRef	mInputFile;
	PGPFileSpecRef	mOutputFile;
	cdstrvect		mSignedBy;
	cdstrvect		mEncryptedTo;
	const char**	mSignedByList;
	const char**	mEncryptedToList;
	CallbackPtr		mCallbackPtr;

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

	void HandleError(PGPError& ex);						// Handle thrown error

	void SetMIMEDetails(SMIMEInfo* mime,				// Set MIME details
						const char* type,
						const char* subtype,
						const char** params);

	void InitKeys();										// Initialise key rings
	void GetKeyringPaths(cdstring& pub, cdstring& sec);
	void GetKeyringPathsText(cdstring& pub, cdstring& sec);
	void GetKeyringPathsXML(cdstring& pub, cdstring& sec);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	void ConvertToMacPath(cdstring& path);
#endif
	PGPKeyDBObjRef GetKeyRef(const char* key,				// Get Key reference
						char* passphrase);
	void GenerateKeySet(const char** keys);				// Generate a key set
	void GenerateUsers(PGPKeySetRef keys,				// Generate a keys from set
						char**& users,
						cdstrvect* userids,
						bool unique);
	void FreeUsers(char**& users);						// Free list of users
	void FinishKeys();									// Finish with Key reference

	virtual void SetLastError(long err, const char* errstr)
		{ mLastErrno = err; mLastError = errstr; }
	
	static PGPError Callback(PGPContextRef pgpContext, PGPEvent* event, PGPUserValue userValue);
	PGPError EventCallback(PGPContextRef pgpContext, PGPEvent* event);
};
