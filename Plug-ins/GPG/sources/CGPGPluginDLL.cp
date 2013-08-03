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

// CGPGPluginDLL.cp
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

#include "CGPGPluginDLL.h"
#include "CPluginInfo.h"
#include "CStringUtils.h"
#if __dest_os == __win32_os
#include "CUnicodeStdLib.h"
#endif

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <memory>

#if __dest_os == __win32_os
#include <fcntl.h>
#include <sys/stat.h>
typedef size_t ssize_t;
#endif
#if __dest_os == __linux_os || __dest_os == __mac_os_x
#include <fcntl.h>
#define O_BINARY 0
#include <sys/stat.h>
#define bzero(a,b) memset(a,0,b)
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#endif

//#define DEBUG_OUTPUT

#pragma mark ____________________________consts

const char* cPluginName = "GPG Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSecurity;
const char* cPluginDescription = "GNUpg Security plugin for Mulberry." COPYRIGHT;
const char* cProcessedBy = "processed by Mulberry GPG Plugin";
const char* cProcessVersion = "Mulberry GPG Plugin v2.0";

const char* cGPG = "gpg";
const char* cGNUPGStatus = "[GNUPG:] ";
const char* cNEED_PASSPHRASE = "NEED_PASSPHRASE ";
const char* cBAD_PASSPHRASE = "BAD_PASSPHRASE ";
const char* cGOOD_PASSPHRASE = "GOOD_PASSPHRASE";
const char* cGOODSIG = "GOODSIG ";
const char* cBADSIG = "BADSIG ";
const char* cERRSIG = "ERRSIG ";
const char* cNOPUBKEY = "NO_PUBKEY ";

#pragma mark ____________________________CGPGPluginDLL

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
CGPGPluginDLL::CGPGPluginDLL()
{
	mData = new SData;
	mData->mSignedByList = NULL;
	mData->mEncryptedToList = NULL;
	mData->mErrno = eSecurity_NoErr;
	mData->mDidSig = false;

#if __dest_os == __win32_os
	// Try to get exe path from registry
	mExePath = cdstring::null_str;

	// Open the key for the full path
	HKEY key;
	if (::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\GNU\\GnuPG", 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		// Determine the space required
		DWORD bufsize = 0;
		if (::RegQueryValueExA(key, "gpgProgram", 0, NULL, NULL, &bufsize) == ERROR_SUCCESS)
		{
			// Reserve the space
			mExePath.reserve(bufsize);

			// Get the key's named value
			::RegQueryValueExA(key, "gpgProgram", 0, NULL, reinterpret_cast<unsigned char*>(mExePath.c_str_mod()), &bufsize);
		}
		
		// Close the key
		::RegCloseKey(key);
	}
	
	// If no registry entry, default to C drive location
	if (mExePath.empty())
		mExePath = "C:\\gnupg\\gpg.exe";
#endif
}

// Destructor
CGPGPluginDLL::~CGPGPluginDLL()
{
	if (mData->mSignedByList)
		cdstring::FreeArray(mData->mSignedByList);
	if (mData->mEncryptedToList)
		cdstring::FreeArray(mData->mEncryptedToList);

	delete mData;
}

// Initialise plug-in
void CGPGPluginDLL::Initialise(void)
{
	// Do default
	CSecurityPluginDLL::Initialise();
}

// Does plug-in need to be registered
bool CGPGPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'PGP5');
	return false;
}

// Can plug-in run as demo
bool CGPGPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

#define DATE_PROTECTION		0

#define	COPYP_MAX_YEAR	2000
#define COPYP_MAX_MONTH	3

// Test for run ability
bool CGPGPluginDLL::CanRun(void)
{
	bool result = false;

#if DATE_PROTECTION
	time_t systime = ::time(nil);
	struct tm* currtime = ::localtime(&systime);

	if ((currtime->tm_year + 1900 > COPYP_MAX_YEAR) ||
		((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 > COPYP_MAX_MONTH)))
		return false;
#endif

	// Check for gpg as an executable
#if __dest_os == __win32_os
	// Check for executable
	if (::access_utf8(mExePath.c_str(), X_OK) == 0)
		return true;
	else
		return false;
#else
	const char* path = ::getenv("PATH");
	if (!path)
		return false;

	// Tokenise PATH
	cdstring cpath(path);
#if __dest_os == __mac_os_x
	cpath += ":/usr/local/bin";
#endif	
	const char* p = ::strtok(cpath.c_str_mod(), ":");
	while(p)
	{
		// Make full path
		cdstring npath(p);
		if (npath.c_str()[npath.length() - 1] != '/')
			npath += "/";
		npath += cGPG;

		// Check for executable
		if (::access(npath.c_str(), X_OK) == 0)
			return true;

		// Next token
		p = ::strtok(NULL, ":");
	}

	return false;
#endif
}

// Returns the name of the plug-in
const char* CGPGPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CGPGPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CGPGPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CGPGPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CGPGPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

#pragma mark ____________________________Memory Based

// Sign data with address
long CGPGPluginDLL::SignData(const char* in, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
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
		if (result)
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
long CGPGPluginDLL::EncryptData(const char* in, const char** to, char** out, unsigned long* out_len, bool useMime, bool binary)
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
		if (result)
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
long CGPGPluginDLL::EncryptSignData(const char* in, const char** to, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
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
		if (result)
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
long CGPGPluginDLL::DecryptVerifyData(const char* in, const char* sig, const char* in_from,
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
		// Create temp files
		TempCreate(in_tmp, out_tmp, in);

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

long CGPGPluginDLL::DisposeData(const char* data)
{
	::free((void*) data);
	return 1;
}

#pragma mark ____________________________File based

// Sign file
long CGPGPluginDLL::SignFile(fspec in, const char* key, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return SignFileX(in, key, out, useMime, binary, false);
}

// Encrypt file
long CGPGPluginDLL::EncryptFile(fspec in, const char** to, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return EncryptFileX(in, to, out, useMime, binary, false);
}

// Encrypt & sign file
long CGPGPluginDLL::EncryptSignFile(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return EncryptSignFileX(in, to, key, out, useMime, binary, false);
}

// Decrypt/verify file
long CGPGPluginDLL::DecryptVerifyFile(fspec in, const char* sig, const char* in_from,
										fspec out, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	// Just do local op but flag as NOT using temp files
	return DecryptVerifyFileX(in, sig, in_from, out, out_signedby, out_encryptedto, success, did_sig, sig_ok, binary, false);
}

#pragma mark ____________________________Local File based operations

// Sign file
long CGPGPluginDLL::SignFileX(fspec in, const char* key, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	// Signing requires passphrase
	char passphrase[256];
	if (!GetSignKeyPassphrase(key, passphrase))
	{
		REPORTERROR(eSecurity_UserAbort, "User cancelled passphrase");
		return 0;
	}

	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Make sure temp file is deleted when we return
	StRemoveFile _in_remove;

	// Do this conversion only if not binary
	if (!binary)
	{
		// Do lendl -> LF conversion using temporary file
		cdstring in_tmp;
		if (lendl_convertLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
			return 0;
		
		// If original file was not overwritten, replace original path with temp path
		if (!using_temp_files)
		{
			// Switch to use temp file as input path
			in_path.reset(in_tmp.grab_c_str());
			
			// Make sure temp file is deleted when we return
			_in_remove.set(in_path.get());
		}
	}
#endif

	cdstrvect args;
	args.push_back("-u");
	args.push_back(key);
	args.push_back("-o");
	args.push_back(out_path.get());
	args.push_back("-a");
	if (useMime)
		args.push_back("--detach-sign");
	else
		args.push_back("--clearsign");
	args.push_back(in_path.get());

	long result = CallGPG(args, passphrase, binary);

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

// Encrypt file
long CGPGPluginDLL::EncryptFileX(fspec in, const char** to, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Make sure temp file is deleted when we return
	StRemoveFile _in_remove;

	// Do this conversion only if not binary
	if (!binary)
	{
		// Do lendl -> LF conversion using temporary file
		cdstring in_tmp;
		if (lendl_convertLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
			return 0;
		
		// If original file was not overwritten, replace original path with temp path
		if (!using_temp_files)
		{
			// Switch to use temp file as input path
			in_path.reset(in_tmp.grab_c_str());
			
			// Make sure temp file is deleted when we return
			_in_remove.set(in_path.get());
		}
	}
#endif

	cdstrvect args;
	const char** p = to;
	while(*p)
	{
		args.push_back("-r");
		args.push_back(*p++);
	}
	args.push_back("-o");
	args.push_back(out_path.get());
	args.push_back("-a");
	args.push_back("-e");
	args.push_back(in_path.get());

	long result = CallGPG(args, NULL, binary);

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

// Encrypt & sign file
long CGPGPluginDLL::EncryptSignFileX(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary, bool using_temp_files)
{
	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

	// Signing requires passphrase
	char passphrase[256];
	if (!GetSignKeyPassphrase(key, passphrase))
	{
		REPORTERROR(eSecurity_UserAbort, "User cancelled passphrase");
		return 0;
	}

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Make sure temp file is deleted when we return
	StRemoveFile _in_remove;

	// Do this conversion only if not binary
	if (!binary)
	{
		// Do lendl -> LF conversion using temporary file
		cdstring in_tmp;
		if (lendl_convertLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
			return 0;
		
		// If original file was not overwritten, replace original path with temp path
		if (!using_temp_files)
		{
			// Switch to use temp file as input path
			in_path.reset(in_tmp.grab_c_str());
			
			// Make sure temp file is deleted when we return
			_in_remove.set(in_path.get());
		}
	}
#endif

	cdstrvect args;
	const char** p = to;
	while(*p)
	{
		args.push_back("-r");
		args.push_back(*p++);
	}
	args.push_back("-u");
	args.push_back(key);
	args.push_back("-o");
	args.push_back(out_path.get());
	args.push_back("-a");
	args.push_back("-es");
	args.push_back(in_path.get());

	long result = CallGPG(args, passphrase, binary);

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion
	if (result == 1)
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif

	return result;
}

// Decrypt/verify file
long CGPGPluginDLL::DecryptVerifyFileX(fspec in, const char* sig, const char* in_from,
											fspec out, char** out_signedby, char** out_encryptedto,
											bool* success, bool* did_sig, bool* sig_ok, bool binary, bool using_temp_files)
{
	long result = 0;
	mData->mDidSig = false;
	cdstrvect signedBy;
	cdstrvect encryptedTo;

	std::auto_ptr<char> in_path(ToPath(in));
	std::auto_ptr<char> out_path(ToPath(out));

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Make sure temp file is deleted when we return
	StRemoveFile _in_remove;

	// Do this conversion only if not binary
	if (!binary)
	{
		// Do lendl -> LF conversion using temporary file
		cdstring in_tmp;
		if (lendl_convertLF(in_path.get(), using_temp_files ? NULL : &in_tmp, false) != 1)
			return 0;
		
		// If original file was not overwritten, replace original path with temp path
		if (!using_temp_files)
		{
			// Switch to use temp file as input path
			in_path.reset(in_tmp.grab_c_str());
			
			// Make sure temp file is deleted when we return
			_in_remove.set(in_path.get());
		}
	}
#endif

	// Signing requires passphrase
	char passphrase[256];

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

	cdstrvect args;
	if (out_path.get() && *out_path.get())
	{
		args.push_back("-o");
		args.push_back(out_path.get());
	}
	if (!sig)
	{
		// Need to get passphrase for file
		if (GetPassphraseForFile(in_path.get(), passphrase, signedBy, encryptedTo) != 1)
		{
			return 0;
		}

		args.push_back("--decrypt");
	}
	else
	{
		// Create temp sig file
		try
		{
			TempCreate(sig_tmp, NULL, sig);
		}
		catch(...)
		{
			*success = false;
		}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (!*sig_spec.name)
#else
		if (!*sig_tmp)
#endif
			return 0;

		std::auto_ptr<char> sig_path(ToPath(sig_tmp));
		_sig_remove.set(sig_path.get());

#if __dest_os == __mac_os_x || __dest_os == __win32_os
		// Do lendl -> LF conversion
		if (lendl_convertLF(sig_path.get(), NULL, false) != 1)
			return 0;
#endif

		// Need to get passphrase for file
		if (GetPassphraseForFile(sig_path.get(), passphrase, signedBy, encryptedTo) != 1)
		{
			return 0;
		}

		args.push_back("--verify");
		args.push_back(sig_path.get());
	}
	args.push_back(in_path.get());

	// Clear out signed by cache in case we need it
	mData->mSignatureKeys.clear();

	result = CallGPG(args, encryptedTo.size() ? passphrase : NULL, binary);
	*success = (result == 1);
	*did_sig = mData->mDidSig;
	*sig_ok = (result == 1);

	// Recover signed by info if not already found
	if (mData->mDidSig && (signedBy.size() == 0) && mData->mSignatureKeys.size())
	{
		LookupKeys(false, mData->mSignatureKeys, signedBy, true, true);
	}

	mData->mSignedByList = cdstring::ToArray(signedBy);
	*out_signedby = (char*) mData->mSignedByList;
	
	if (encryptedTo.size())
	{
		mData->mEncryptedToList = cdstring::ToArray(encryptedTo);
		*out_encryptedto = (char*) mData->mEncryptedToList;
	}

#if __dest_os == __mac_os_x || __dest_os == __win32_os
	// Do LF -> lendl conversion only for valid output file
	// When verifying there may not be an output
	if ((result == 1) && out_path.get() && *out_path.get())
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
	{ "micalg", "pgp-sha1", "protocol", "application/pgp-signature", NULL };
const char* cMIMEMultipartEncryptedParams[] = 
	{ "protocol", "application/pgp-encrypted", NULL };

const char* cMIMEApplicationType = "application";
const char* cMIMEPGPSigned = "pgp-signature";
const char* cMIMEPGPEncrypted = "pgp-encrypted";
const char* cMIMEOctetStream = "octet-stream";

// Get MIME parameters for signing
long CGPGPluginDLL::GetMIMESign(SMIMEMultiInfo* params)
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
					cMIMEPGPSigned,
					NULL);
	
	return 1;
}

// Get MIME parameters for encryption
long CGPGPluginDLL::GetMIMEEncrypt(SMIMEMultiInfo* params)
{
	SetMIMEDetails(&params->multipart,
					cMIMEMultipartType,
					cMIMEMultipartEncrypted,
					cMIMEMultipartEncryptedParams);
	
	SetMIMEDetails(&params->first,
					cMIMEApplicationType,
					cMIMEPGPEncrypted,
					NULL);
	
	SetMIMEDetails(&params->second,
					cMIMEApplicationType,
					cMIMEOctetStream,
					NULL);
	
	return 1;
}

// Get MIME parameters for encryption
long CGPGPluginDLL::GetMIMEEncryptSign(SMIMEMultiInfo* params)
{
	SetMIMEDetails(&params->multipart,
					cMIMEMultipartType,
					cMIMEMultipartEncrypted,
					cMIMEMultipartEncryptedParams);
	
	SetMIMEDetails(&params->first,
					cMIMEApplicationType,
					cMIMEPGPEncrypted,
					NULL);
	
	SetMIMEDetails(&params->second,
					cMIMEApplicationType,
					cMIMEOctetStream,
					NULL);
	
	return 1;
}

// Check that MIME type is verifiable by this plugin
long CGPGPluginDLL::CanVerifyThis(const char* type)
{
	// Return 0 if it can verify, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	return !::strcmpnocase(type, "application/pgp-signature") ? 0 : 1;
}

// Check that MIME type is decryptable by this plugin
long CGPGPluginDLL::CanDecryptThis(const char* type)
{
	// Return 0 if it can decrypt, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	return ::strcmpnocase(type, "application/pgp-encrypted");
}

// Get last textual error
long CGPGPluginDLL::GetLastError(long* err_no, char** error)
{
	*err_no = mData->mErrno;
	*error = mData->mErrstr.c_str_mod();

	return 0;
}

#pragma mark ____________________________Utilities
// Prepare PGP context
void CGPGPluginDLL::PreparePGP()
{
	// Create a new PGP context
	//ThrowIfPGPErr(PGPNewContext(kPGPsdkAPIVersion, &mContextRef))
}

// Finish with PGP context
void CGPGPluginDLL::FinishPGP()
{
	//if(PGPContextRefIsValid(mContextRef)) 	PGPFreeContext(mContextRef);

	// Initialize allocatable storage to  kInvalidPGP..
	//mContextRef = kInvalidPGPContextRef;
}

// Set MIME details
void CGPGPluginDLL::SetMIMEDetails(SMIMEInfo* mime, const char* type, const char* subtype, const char** params)
{
	mime->type = type;
	mime->subtype = subtype;
	mime->params = params;
}

long CGPGPluginDLL::CallGPG(cdstrvect& args, const char* passphrase, bool binary, bool file_status, bool key_list)
{
	mData->mErrno = 0;
	long result = 1;

	cdstrvect out;
#ifdef USE_WIN32FORK
	out.push_back(mExePath.c_str());
#else
#if __dest_os == __mac_os_x
	out.push_back("/usr/local/bin/gpg");
#else
	out.push_back(cGPG);
#endif
#endif
	out.push_back("--batch");
	//out.push_back("--no-options");
	out.push_back("--yes");
	
	// Do canonical text mode if not binary
	if (!binary)
		out.push_back("-t");

#if defined(USE_UNIXFORK)

	int outputfd[2] = {-1, -1};
	int errorfd[2] = {-1, -1};
	int statusfd[2] = {-1, -1};
	int passfd[2] = {-1, -1};

	try
	{
		// Create stdout pipe if required
		if (file_status)
		{
			if (pipe(outputfd) == -1)
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create output pipe");
				throw -1L;
			}
		}

		// Always create stderr pipe
		if (pipe(errorfd) == -1)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create error pipe");
			throw -1L;
		}

		// Always create a status pipe
		if (pipe(statusfd) == -1)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create status pipe");
			throw -1L;
		}
		else
		{
			// Make sure gpg knows what the pipe's fd is
			out.push_back("--status-fd");
			out.push_back(cdstring((unsigned long) statusfd[1]));
		}

		// Create a passphrase pipe if required
		if (passphrase)
		{
			if (pipe(passfd) == -1)
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create passphrase pipe");
				throw -1L;
			}
			else
			{
				// Write passphrase into the pipe ready for gpg to read it
				::write (passfd[1], passphrase, ::strlen(passphrase));
				::write (passfd[1], "\n", 1);

				// Make sure gpg knows what the pipe's fd is
				out.push_back("--passphrase-fd");
				out.push_back(cdstring((unsigned long) passfd[0]));
			}
		}
	}
	catch(...)
	{
		unix_closepipes(outputfd);
		unix_closepipes(errorfd);
		unix_closepipes(statusfd);
		unix_closepipes(passfd);

		return 0;
	}

	// Do log of command line here before the fork:
	// OS X does not like the LogEntry in the forked process
	if (mLogging)
	{
		cdstring logged = "Command line: ";
 		for(unsigned int i = 0; i < out.size(); i++)
 		{
			logged += " ";
			logged += out[i];
		}
 		for(unsigned int i = 0; i < args.size(); i++)
 		{
			logged += " ";
			logged += args[i];
		}
 		LogEntry(logged);
	}

	// Create gpg process
	pid_t pid = fork();
	if (pid == -1)
	{
		REPORTERROR(eSecurity_UnknownError, "Failed to fork");
		unix_closepipes(outputfd);
		unix_closepipes(errorfd);
		unix_closepipes(statusfd);
		unix_closepipes(passfd);
	}
	else if (pid == 0)
	{
		// Child process
 
		// Close other ends of pipes
		if (outputfd[0] != -1)
		{
			::close(outputfd[0]);
			outputfd[0] = -1;
		}
		::close(errorfd[0]);
		errorfd[0] = -1;
		::close(statusfd[0]);
		statusfd[0] = -1;

		// Direct stdin, stdout, stderr to /dev/null
		int devnull = ::open("/dev/null", O_RDWR);
		if (devnull == -1)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open /dev/null in child process");
			exit(1);
		}

		// Setup stdin, stdout & stderr
		::dup2(devnull, 0);
		::dup2(outputfd[1], 1);
		::close(outputfd[1]);
		outputfd[1] = -1;
		::dup2(errorfd[1], 2);
		::close(errorfd[1]);
		errorfd[1] = -1;

		// Close fds that we do not want the child to inherit
		long open_max = ::sysconf(_SC_OPEN_MAX);
		if (open_max > 0)
		{
			for (int i = 3; i < open_max; i++)
			{
				// Must keep status (out) /pswd (in) pipes open
				if ((i != statusfd[1]) && (i != passfd[0]))
					::close(i);
			}
		}

		// create args
		int argc = out.size() + args.size();
		char** argv = (char**) malloc((argc + 1) * sizeof(char*));
		char** p = argv;
		cdstring logged;
 		for(unsigned int i = 0; i < out.size(); i++)
 			*p++ = out[i].c_str_mod();
 		for(unsigned int i = 0; i < args.size(); i++)
 			*p++ = args[i].c_str_mod();
 		*p = NULL;
 
		// Execute it
		::execvp(argv[0], argv);

		// Only get here if failed
		REPORTERROR(eSecurity_UnknownError, "Failed to exec process");
		exit(1);
	}
	else
	{
		// Parent process must wait
		int status;
		pid_t retpid = ::waitpid(pid, &status, WNOHANG);

		fd_set readfds;
		FD_ZERO (&readfds);
		if (file_status)
			FD_SET (outputfd[0], &readfds);
		FD_SET (errorfd[0], &readfds);
		FD_SET (statusfd[0], &readfds);
		struct timeval timeout = { 0, 0 };
		int select_fd = std::max(outputfd[0], std::max(errorfd[0], statusfd[0])) + 1;
		int dataavail = ::select(select_fd, &readfds, NULL, NULL, &timeout);

		cdstring output_line;
		mData->mStdError = cdstring::null_str;
		cdstring status_line;
		while((retpid == 0) || (dataavail != 0))
		{
			// Handle output
			if (file_status && FD_ISSET(outputfd[0], &readfds))
			{
				const int bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				int readsize = ::read(outputfd[0], buf, bufsize);
				if (readsize == -1)
					break;
				buf[(unsigned long) std::min(bufsize - 1, readsize)] = 0;
				buf.ConvertEndl();
				output_line += buf;

#ifdef DEBUG_OUTPUT
				printf("%s", buf);
#endif
				LogEntry(buf);

				if (key_list)
					ProcessKeyListOutput(output_line);
				else
					ProcessFileStatusOutput(output_line);
			}

			// Handle error
			if (FD_ISSET(errorfd[0], &readfds))
			{
				const int bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				int readsize = ::read(errorfd[0], buf, bufsize);
				if (readsize == -1)
					break;
				buf[(unsigned long) std::min(bufsize - 1, readsize)] = 0;
				buf.ConvertEndl();

#ifdef DEBUG_OUTPUT
				printf("%s", buf);
#endif
				LogEntry(buf);
				mData->mStdError += buf;
			}

			// Handle status
			if (FD_ISSET(statusfd[0], &readfds))
			{
				const int bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				int readsize = ::read(statusfd[0], buf, bufsize);
				if (readsize == -1)
					break;
				buf[(unsigned long) std::min(bufsize - 1, readsize)] = 0;
				buf.ConvertEndl();
				status_line += buf;

#ifdef DEBUG_OUTPUT
				printf("%s", buf);
#endif
				LogEntry(buf);

				ProcessStatus(status_line);
			}

			if (retpid == 0)
			{
				retpid = ::waitpid(pid, &status, WNOHANG);
				if (retpid == -1)
					break;
			}
			FD_ZERO (&readfds);
			if (file_status)
				FD_SET (outputfd[0], &readfds);
			FD_SET (errorfd[0], &readfds);
			FD_SET (statusfd[0], &readfds);
			dataavail = ::select(select_fd, &readfds, NULL, NULL, &timeout);
			if (dataavail == -1)
				break;
		}

		// Check process return value
		if (mData->mErrno)
			result = 0;
		else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			// Use stderror output if any as error text
			if (mData->mStdError.length())
				REPORTERROR(eSecurity_UnknownError, mData->mStdError.c_str());
			else
				REPORTERROR(eSecurity_UnknownError, "Exit status non-zero");
			result = 0;
			{
				cdstring buf;
				buf.reserve(1024);
				int exit_status = WEXITSTATUS(status);
				::sprintf(buf.c_str_mod(), "Exit status non-zero: %d\n", exit_status);

				buf.ConvertEndl();
#ifdef DEBUG_OUTPUT
				printf("%s", buf);
#endif
				LogEntry(buf);
			}
		}
		else if (WIFSIGNALED(status) && WTERMSIG(status) != 0)
		{
			REPORTERROR(eSecurity_UnknownError, "Unhandled signal");
			result = 0;
		}
		else
			result = 1;
		
		unix_closepipes(outputfd);
		unix_closepipes(errorfd);
		unix_closepipes(statusfd);
		unix_closepipes(passfd);
	}
#elif defined(USE_WIN32FORK)
	HANDLE outputfd[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
	HANDLE errorfd[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
	HANDLE statusfd[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
	HANDLE passfd[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

	try
	{
		// Create stdout pipe if required
		if (file_status)
		{
			if (!win32_createpipes(outputfd, true))
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create output pipe");
				throw -1L;
			}
		}

		// Always create stderr pipe
		if (!win32_createpipes(errorfd, true))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create error pipe");
			throw -1L;
		}

		// Always create a status pipe
		if (!win32_createpipes(statusfd, true))
		{
			REPORTERROR(eSecurity_UnknownError, "Could not create status pipe");
			throw -1L;
		}
		else
		{
			// Make sure gpg knows what the pipe's fd is
			out.push_back("--status-fd");
			out.push_back(cdstring((unsigned long) statusfd[1]));
		}

		// Create a passphrase pipe if required
		if (passphrase)
		{
			if (!win32_createpipes(passfd, false))
			{
				REPORTERROR(eSecurity_UnknownError, "Could not create passphrase pipe");
				throw -1L;
			}
			else
			{
				// Write passphrase into the pipe ready for gpg to read it
				DWORD written;
				::WriteFile(passfd[1], passphrase, ::strlen(passphrase), &written, NULL);
				::WriteFile(passfd[1], "\n", 1, &written, NULL);

				// Make sure gpg knows what the pipe's fd is
				out.push_back("--passphrase-fd");
				out.push_back(cdstring((unsigned long) passfd[0]));
			}
		}
	}
	catch(...)
	{
		win32_closepipes(outputfd);
		win32_closepipes(errorfd);
		win32_closepipes(statusfd);
		win32_closepipes(passfd);

		return 0;
	}

	// Add args
	for(cdstrvect::iterator iter = args.begin(); iter != args.end(); iter++)
		out.push_back(*iter);

	// Create process
	HANDLE proc = win32_spawn(out, outputfd[1], errorfd[1], statusfd[1]);
    if (proc == INVALID_HANDLE_VALUE)
	{
		REPORTERROR(eSecurity_UnknownError, "Failed to fork");
		win32_closepipes(outputfd);
		win32_closepipes(errorfd);
		win32_closepipes(statusfd);
		win32_closepipes(passfd);
		
		result = 0;
	}
	else
	{
		// Parent process must wait
		DWORD status;
		int retpid = win32_waitpid(proc, status);

		bool outputfd_bytes = false;
		bool errorfd_bytes = false;
		bool statusfd_bytes = false;
		int dataavail = win32_select(outputfd[0], outputfd_bytes, errorfd[0], errorfd_bytes, statusfd[0], statusfd_bytes);

		cdstring output_line;
		mData->mStdError = cdstring::null_str;
		cdstring status_line;
		while((retpid == 0) || (dataavail != 0))
		{
			// Handle output
			if (outputfd_bytes)
			{
				const DWORD bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				DWORD bufread;
				if (::ReadFile(outputfd[0], buf.c_str_mod(), bufsize, &bufread, NULL) && bufread)
				{
					buf[(cdstring::size_type)std::min(bufsize - 1, bufread)] = 0;
					buf.ConvertEndl();
					output_line += buf;

#ifdef DEBUG_OUTPUT
					printf("%s", buf);
#endif
					LogEntry(buf);

					if (key_list)
						ProcessKeyListOutput(output_line);
					else
						ProcessFileStatusOutput(output_line);
				}
			}

			// Handle error
			if (errorfd_bytes)
			{
				const DWORD bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				DWORD bufread;
				if (::ReadFile(errorfd[0], buf.c_str_mod(), bufsize, &bufread, NULL) && bufread)
				{
					buf[(cdstring::size_type)std::min(bufsize - 1, bufread)] = 0;
					buf.ConvertEndl();
#ifdef DEBUG_OUTPUT
					printf("%s", buf);
#endif
					LogEntry(buf);
					mData->mStdError += buf;
				}
			}

			// Handle status
			if (statusfd_bytes)
			{
				const DWORD bufsize = 1024;
				cdstring buf;
				buf.reserve(bufsize);
				DWORD bufread;
				if (::ReadFile(statusfd[0], buf.c_str_mod(), bufsize, &bufread, NULL) && bufread)
				{
					buf[(cdstring::size_type)std::min(bufsize - 1, bufread)] = 0;
					buf.ConvertEndl();
					status_line += buf;

#ifdef DEBUG_OUTPUT
					printf("%s", buf);
#endif
					LogEntry(buf);

					ProcessStatus(status_line);
				}
			}

			if (retpid == 0)
				retpid = win32_waitpid(proc, status);

			dataavail = win32_select(outputfd[0], outputfd_bytes, errorfd[0], errorfd_bytes, statusfd[0], statusfd_bytes);
		}

		// Check process return value
		if (mData->mErrno)
			result = 0;
		else if (status != 0)
		{
			// Use stderror output if any as error text
			if (mData->mStdError.length())
				REPORTERROR(eSecurity_UnknownError, mData->mStdError.c_str());
			else
				REPORTERROR(eSecurity_UnknownError, "Exit status non-zero");
			result = 0;
			{
				cdstring buf;
				buf.reserve(1024);
				::sprintf(buf.c_str_mod(), "Exit status non-zero: %d\n", status);

				buf.ConvertEndl();
#ifdef DEBUG_OUTPUT
				printf("%s", buf);
#endif
				LogEntry(buf);
			}
		}
		else
			result = 1;
		
		win32_closepipes(outputfd);
		win32_closepipes(errorfd);
		win32_closepipes(statusfd);
		win32_closepipes(passfd);
	}
#endif

	return result;
}

#ifdef USE_UNIXFORK
int CGPGPluginDLL::unix_closepipes(int* fds)
{
	for(int i = 0; i < 2; i++)
	{
		if (fds[i] != -1)
		{
			::close(fds[i]);
			fds[i] = -1;
		}
	}
	
	return 1;
}
#endif

#ifdef USE_WIN32FORK
int CGPGPluginDLL::win32_createpipes(HANDLE* hdls, bool read_this_end)
{
	// Create the pipe handles
	if (!::CreatePipe(&hdls[0], &hdls[1], NULL, 1024))
	{
		REPORTERROR(eSecurity_UnknownError, "Could not create pipe");
		::CloseHandle(hdls[0]);
		::CloseHandle(hdls[1]);
		return 0;
	}
	
	// Duplicate the handle for the other end and make it inheritable
	HANDLE h;
	if (!::DuplicateHandle(GetCurrentProcess(), hdls[read_this_end ? 1 : 0], GetCurrentProcess(), &h, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		REPORTERROR(eSecurity_UnknownError, "Could not duplicate pipe");
		::CloseHandle(hdls[0]);
		::CloseHandle(hdls[1]);
		return 0;
	}
	::CloseHandle(hdls[read_this_end ? 1 : 0]);
	hdls[read_this_end ? 1 : 0] = h;
	
	return 1;
}

int CGPGPluginDLL::win32_closepipes(HANDLE* hdls)
{
	for(int i = 0; i < 2; i++)
	{
		if (hdls[i] != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hdls[i]);
			hdls[i] = INVALID_HANDLE_VALUE;
		}
	}
	
	return 1;
}

HANDLE CGPGPluginDLL::win32_spawn(const cdstrvect& args, HANDLE& outputfd, HANDLE& errorfd, HANDLE& statusfd)
{
	// Create process
	cdstring cmd_line;
	for(cdstrvect::const_iterator iter = args.begin(); iter != args.end(); iter++)
	{
		if (iter != args.begin())
			cmd_line += " ";
		
		// May need to quote
		if (::strchr((*iter).c_str(), ' '))
		{
			cmd_line += "\"";
			cmd_line += *iter;
			cmd_line += "\"";
		}
		else
			cmd_line += *iter;
	}
	LogEntry(cmd_line);

    SECURITY_ATTRIBUTES sec_attr;
    ::memset(&sec_attr, 0, sizeof sec_attr);
    sec_attr.nLength = sizeof sec_attr;
    sec_attr.bInheritHandle = FALSE;

    int cr_flags = CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE | GetPriorityClass(GetCurrentProcess());
    char *envblock = NULL;

	int debug_me = 0;
    STARTUPINFOA si;
   	::memset(&si, 0, sizeof si);
    si.cb = sizeof (si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = debug_me? SW_SHOW : SW_HIDE;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	// si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdOutput = (outputfd != INVALID_HANDLE_VALUE ? outputfd : GetStdHandle(STD_OUTPUT_HANDLE));
    si.hStdError = (errorfd != INVALID_HANDLE_VALUE ? errorfd : GetStdHandle(STD_OUTPUT_HANDLE));

    PROCESS_INFORMATION pi = {
        NULL,      /* returns process handle */
        0,         /* returns primary thread handle */
        0,         /* returns pid */
        0         /* returns tid */
    };

    if (::CreateProcessA(NULL, cmd_line, &sec_attr, &sec_attr, TRUE, cr_flags, envblock, NULL, &si, &pi))
	{
		// Child process
 
		// Close other ends of pipes
		if (outputfd != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(outputfd);
			outputfd = INVALID_HANDLE_VALUE;
		}
		if (errorfd != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(errorfd);
			errorfd = INVALID_HANDLE_VALUE;
		}
		::CloseHandle(statusfd);
		statusfd = INVALID_HANDLE_VALUE;

		// Execute it
		if (::ResumeThread(pi.hThread) < 0)
		{
			// Only get here if failed
			REPORTERROR(eSecurity_UnknownError, "Failed to ResumeThread");
	    	return INVALID_HANDLE_VALUE;
		}
	    if (!::CloseHandle(pi.hThread))
	    { 
			REPORTERROR(eSecurity_UnknownError, "Failed to CloseHandle");
	    	return INVALID_HANDLE_VALUE;
	    }

    	return pi.hProcess;
	}
    else
    {
    	DWORD err = ::GetLastError();
    	return INVALID_HANDLE_VALUE;
    }
}

int CGPGPluginDLL::win32_waitpid(HANDLE proc, DWORD& status)
{
	int result = 0;

	DWORD signal = ::WaitForSingleObject(proc, 0);
	switch (signal)
	{
	case WAIT_FAILED:
		REPORTERROR(eSecurity_UnknownError, "WaitForSingleObjectre returned WAIT_FAILED");
		break;

	case WAIT_OBJECT_0:
		if (!::GetExitCodeProcess(proc, &status))
			REPORTERROR(eSecurity_UnknownError, "GetExitCodeProcess returned false");
		result = 1;
		break;

	case WAIT_TIMEOUT:
		break;

	default:
		break;
	}

	return result;
}

int CGPGPluginDLL::win32_select(HANDLE outfd, bool& outfd_bytes, HANDLE errorfd, bool& errorfd_bytes, HANDLE statusfd, bool& statusfd_bytes)
{
	outfd_bytes = win32_hasbytes(outfd);
	errorfd_bytes = win32_hasbytes(errorfd);
	statusfd_bytes = win32_hasbytes(statusfd);

	return (outfd_bytes || errorfd_bytes || statusfd_bytes) ? 1 : 0;
}

bool CGPGPluginDLL::win32_hasbytes(HANDLE hdl)
{
	DWORD bytes_available = 0;
	if (hdl != INVALID_HANDLE_VALUE)
		::PeekNamedPipe(hdl, NULL, 0, NULL, &bytes_available, NULL);
	
	return (bytes_available > 0);
}

#endif

long CGPGPluginDLL::GetSignKeyPassphrase(const char* key, char* passphrase)
{
	if (key && *key && passphrase)
		return GetSignPassphrase(key, passphrase);
	else
		return 0;
}

long CGPGPluginDLL::GetPassphraseForFile(const char* in_path, char* passphrase, cdstrvect& signedBy, cdstrvect& encryptedTo)
{
	// Init required data structures
	mData->mListKeys.clear();
	mData->mSignatureKeys.clear();
	mData->mEncryptionKeys.clear();

	// Get packet data from file
	cdstrvect args;
	args.push_back("--list-packets");
	args.push_back("--list-only");
	args.push_back(in_path);

	// Ignore errors
	CallGPG(args, NULL, true, true);

	// Map signing keyids to names
	if (mData->mSignatureKeys.size())
		LookupKeys(false, mData->mSignatureKeys, signedBy, true, true);

	// Map encryption keyids to names
	if (mData->mEncryptionKeys.size())
	{
		// Map encryption keyids to names (public keys)
		LookupKeys(false, mData->mEncryptionKeys, encryptedTo, true, false);

		// Map encryption keyids to names (private keys)
		cdstrvect secret_keys;
		LookupKeys(true, mData->mEncryptionKeys, secret_keys, false, false);

		// Get a passphrase for a secret key
		if (secret_keys.size())
		{
			// Create array of keys
			std::auto_ptr<const char*> users(cdstring::ToArray(secret_keys, false));

			// Get passphrase
			unsigned long chosen;
			return GetPassphrase(users.get(), passphrase, chosen);
		}
		else
		{
			REPORTERROR(eSecurity_KeyUnavailable, "No secret keys found");
			return 0;
		}
	}
	
	return 1;
}

void CGPGPluginDLL::LookupKeys(bool secret, const cdstrvect& keyids, cdstrvect& keynames, bool add_missing, bool multiple_uids)
{
	mData->mKeyIDMap.clear();

	// List public keys with matching keyids
	cdstrvect args;
	args.push_back(secret ? "--list-secret-keys" : "--list-keys");
	for(cdstrvect::const_iterator iter = keyids.begin(); iter != keyids.end(); iter++)
		args.push_back(*iter);

	// Ignore errors
	CallGPG(args, NULL, true, true, true);

	// Map each keyid found in the PGP data to a name to pass back
	for(cdstrvect::const_iterator iter1 = keyids.begin(); iter1 != keyids.end(); iter1++)
	{
		// If a name mapping exists use the names others use keyid
		if (mData->mKeyIDMap.count(*iter1) == 1)
		{
			for(cdstrvect::const_iterator iter2 =  mData->mKeyIDMap[*iter1].begin(); iter2 != mData->mKeyIDMap[*iter1].end(); iter2++)
			{
				// Append key ids if secret keys
				if (secret)
				{
					cdstring name(*iter2);
					name += " (";
					name += *iter1;
					name += ")";
					keynames.push_back(name);
				}
				else
					keynames.push_back(*iter2);
				
				// Only do one if requested
				if (!multiple_uids)
					break;
			}
		}
		
		// Only add keys that are found if requested
		else if (add_missing)
		{
			// Put key id in parens
			cdstring id;
			id += "(";
			id += *iter1;
			id += ")";
			keynames.push_back(id);
		}
	}
}

long CGPGPluginDLL::ProcessStatus(cdstring& status)
{
	// Look for complete line
	const char* p = ::strchr(status.c_str(), os_endl[0]);
	while(p)
	{
		// Grab line and reset remainder
		cdstring line(status.c_str(), p - status.c_str());
		cdstring temp(p + os_endl_len);
		status = temp;

		// Look for GPG tag and step over
		if (::strncmp(line, cGNUPGStatus, ::strlen(cGNUPGStatus)))
			return 1;
		p = line.c_str() + ::strlen(cGNUPGStatus);

		// Look for specific tags
		if (!::strncmp(p, cGOOD_PASSPHRASE, ::strlen(cGOOD_PASSPHRASE)))
		{
			// Always reset the error here. When decrypting a message with multiple sigs
			// BAD_PASSPHRASE will occur before GOOD_PASSPHRASE for keys the user does not
			// enter a passpharse for. As soon as GOOD_PASSPHRASE is sent, passphrase processing
			// stops, so we clear any earlier errors.
			REPORTERROR(eSecurity_NoErr, "Good Passphrase");
		}
		else if (!::strncmp(p, cBAD_PASSPHRASE, ::strlen(cBAD_PASSPHRASE)))
		{
			// Step over it
			REPORTERROR(eSecurity_BadPassphrase, "Bad Passphrase");
		}
		else if (!::strncmp(p, cBADSIG, ::strlen(cBADSIG)))
		{
			// Step over it
			REPORTERROR(eSecurity_InvalidSignature, "Bad Signature");
		}
		else if (!::strncmp(p, cERRSIG, ::strlen(cERRSIG)))
		{
			// Step over it
			REPORTERROR(eSecurity_InvalidSignature, "Error in Signature");
		}
		else if (!::strncmp(p, cNOPUBKEY, ::strlen(cNOPUBKEY)))
		{
			::strtok(const_cast<char*>(p), " ");		// Token: NO_PUBKEY
			char* q = ::strtok(NULL, "");	// Token: keyid
			if (::strlen(q) == 16)
				q += 8;
			cdstring errtxt("No Public Key (0x");
			errtxt += q;
			errtxt += ") for Signature";

			// Step over it
			REPORTERROR(eSecurity_InvalidSignature, errtxt);
		}
		else if (!::strncmp(p, cGOODSIG, ::strlen(cGOODSIG)))
		{
			// Set flag only
			mData->mDidSig = true;
			
			::strtok(const_cast<char*>(p), " ");		// Token: GOODSIG
			char* q = ::strtok(NULL, " ");	// Token: keyid
			if (::strlen(q) == 16)
				q += 8;

			// Add key id to list of signatures
			mData->mSignatureKeys.push_back(q);
		}

		// Look for next complete line
		p = ::strchr(status.c_str(), os_endl[0]);
	}

	return 1;
}

long CGPGPluginDLL::ProcessFileStatusOutput(cdstring& output)
{
	// Look for complete line
	const char* p = ::strchr(output.c_str(), os_endl[0]);
	while(p)
	{
		// Grab line and reset remainder
		cdstring line(output.c_str(), p - output.c_str());
		cdstring temp(p + os_endl_len);
		output = temp;

		// Look for specific text
		if (line.compare_start(":literal data packet:"))
			;
		else if (line.compare_start(":pubkey enc packet:"))
		{
			// Look for keyid
			char* q = ::strstr(line.c_str_mod(), "keyid ");
			q += 6;
			char* r = ::strtok(q, " ");
			if (::strlen(r) == 16)
				r += 8;
			mData->mEncryptionKeys.push_back(r);
		}
		else if (line.compare_start(":symkey enc packet:"))
			;
		else if (line.compare_start(":compressed packet:"))
			;
		else if (line.compare_start(":onepass_sig packet:"))
			;
		else if (line.compare_start(":signature packet:"))
		{
			// Look for keyid
			char* q = ::strstr(line.c_str_mod(), "keyid ");
			q += 6;
			char* r = ::strtok(q, " ");
			if (::strlen(r) == 16)
				r += 8;
			mData->mSignatureKeys.push_back(r);
		}

		p = ::strchr(output.c_str(), os_endl[0]);
	}

	return 1;
}

long CGPGPluginDLL::ProcessKeyListOutput(cdstring& output)
{
	// Look for complete line
	const char* p = ::strchr(output.c_str(), os_endl[0]);
	while(p)
	{
		// Grab line and reset remainder
		cdstring line(output.c_str(), p - output.c_str());
		cdstring temp(p + os_endl_len);
		output = temp;

		// Look for specific text
		if (line.compare_start("sec ") || line.compare_start("pub "))
		{
			cdstring id;
			::strtok(line, " ");	// sec
			char* q = ::strtok(NULL, " ");		// id
			if (::strchr(q, '/'))
				id += ::strchr(q, '/') + 1;
			else
				id += q;
			::strtok(NULL, " ");		// date
			cdstring name = ::strtok(NULL, "");		// name
			name.trimspace();
			
			// Cache new id
			mData->mLastID = id;

			// Create map entries
			cdstrvect temp;
			mData->mKeyIDMap["current"] = temp;
			mData->mKeyIDMap[mData->mLastID] = temp;
			
			// Skip if not a valid name
			if (!name.empty() && (name[(cdstring::size_type)0] != '['))
			{			
				// Cache current name (before we add the key id)
				mData->mKeyIDMap["current"].push_back(name);
				
				// Add name/id to list
				mData->mKeyIDMap[mData->mLastID].push_back(name);
			}
		}
		else if (line.compare_start("uid "))
		{
			cdstring id;
			::strtok(line, " ");	// uid
			char* q = ::strtok(NULL, "");	// name
			
			// Get current name
			cdstring name(q);
			name.trimspace();

			// Skip if not a valid name
			if (!name.empty() && (name[(cdstring::size_type)0] != '['))
			{			
				// Cache current name (before we add the key id)
				mData->mKeyIDMap["current"].push_back(name);
				
				// Add name/id to list
				mData->mKeyIDMap[mData->mLastID].push_back(name);
			}
		}
		else if (line.compare_start("sub ") || line.compare_start("ssb "))
		{
			cdstring id;
			::strtok(line, " ");	// sec
			char* q = ::strtok(NULL, " ");		// id
			if (::strchr(q, '/'))
				id += ::strchr(q, '/') + 1;
			else
				id += q;
			
			// Get current names
			cdstrvect names = mData->mKeyIDMap["current"];

			// Add id/names to map
			mData->mKeyIDMap[id] = names;
		}

		p = ::strchr(output.c_str(), os_endl[0]);
	}

	return 1;
}
