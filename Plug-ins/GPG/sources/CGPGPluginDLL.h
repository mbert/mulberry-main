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

// CGPGPluginDLL.h
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

#ifndef __GPG_PLUGIN_MULBERRY__
#define __GPG_PLUGIN_MULBERRY__

#include "CSecurityPluginDLL.h"
#include <string.h>
#include "cdstring.h"

// Set various macros for different types of behaviour

// Mac OS X/linux : use unix fork/exec
#if __dest_os == __linux_os || __dest_os == __mac_os_x
#define USE_UNIXFORK
#endif

// Win32 : use Win32 equivalent fork/exec
#if __dest_os == __win32_os
#define USE_WIN32FORK
#endif

// Classes
class CGPGPluginDLL : public CSecurityPluginDLL
{
public:
	// Actual plug-in class

	CGPGPluginDLL();
	virtual ~CGPGPluginDLL();
	
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

	virtual long	GetLastError(long* err_no, char** error);	// Get last textual error

protected:
	typedef std::map<cdstring, cdstrvect> cdstrvectmap;

	struct SData
	{
		cdstrvect 		mListKeys;
		cdstrvect 		mSignatureKeys;
		cdstrvect 		mEncryptionKeys;
		cdstring		mLastID;
		cdstrvectmap	mKeyIDMap;
		const char**	mSignedByList;
		const char**	mEncryptedToList;
		cdstring	 	mErrstr;
		long 			mErrno;
		bool 			mDidSig;
		cdstring 		mStdError;
	};
	SData* mData;
#if __dest_os == __win32_os
		cdstring		mExePath;
#endif

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

#ifdef USE_UNIXFORK
	int unix_closepipes(int* fds);
#endif
#ifdef USE_WIN32FORK
	int win32_createpipes(HANDLE* hdls, bool read_this_end);
	int win32_closepipes(HANDLE* hdls);
	HANDLE win32_spawn(const cdstrvect& args, HANDLE& outfd, HANDLE& errorfd, HANDLE& statusfd);
	int win32_waitpid(HANDLE proc, DWORD& status);
	int win32_select(HANDLE outfd, bool& outfd_bytes, HANDLE errorfd, bool& errorfd_bytes, HANDLE statusfd, bool& statusfd_bytes);
	bool win32_hasbytes(HANDLE hdl);
#endif

	long CallGPG(cdstrvect& args, const char* passphrase, bool binary,
					bool file_status = false, bool key_list = false);							// Call 

	long GetSignKeyPassphrase(const char* key, char* passphrase);
	long GetPassphraseForFile(const char* in_path, char* passphrase, cdstrvect& signedBy, cdstrvect& encryptedTo);
	void LookupKeys(bool secret, const cdstrvect& keyids, cdstrvect& keynames, bool add_missing, bool multiple_uids);

	long ProcessStatus(cdstring& status);
	long ProcessFileStatusOutput(cdstring& output);
	long ProcessKeyListOutput(cdstring& output);
};

#endif
