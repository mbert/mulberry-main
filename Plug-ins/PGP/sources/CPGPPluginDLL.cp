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

// CPGPPluginDLL.cp
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

#if __dest_os == __win32_os
#undef Boolean
#endif

#include "CPGPPluginDLL.h"

#if __dest_os == __mac_os
#include "CMachOFunctions.h"
#endif
#include "CPluginInfo.h"
#include "CStringUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MoreFilesX.h"
#endif

#if __dest_os == __mac_os
#include "loadpgpsdkfunc.h"
#elif __dest_os == __mac_os_x
//#undef PGP_MACINTOSH
//#define PGP_UNIX_DARWIN	1
//#define PGP_OSX	1
//#include "pgpClientLib.h"
//#include "PGPOpenPrefs.h"
//#define PGP_MACINTOSH	1

#include <sys/stat.h>

#elif __dest_os == __win32_os
#include "loadpgpsdkfuncWin32.h"

#include <stat.h>

#include <SHLOBJ.H>
#endif

#include <fstream.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if __dest_os == __mac_os
void* MachOFunctionPointerForCFMFunctionPointer(void* cfmfp);
#endif

#define ThrowIfPGPErr(_thetest_)	\
		{ PGPError _err_  = _thetest_; \
		  if IsPGPError(_err_) {\
		  	mErrorFunction = __FUNCTION__; mErrorFile = __FILE__; mErrorLine = __LINE__; \
		  	throw _err_ ;}}

#define ThrowPGPErr(_err_)		\
			{ PGPError _err__ = _err_; mErrorFunction = __FUNCTION__; mErrorFile = __FILE__; mErrorLine = __LINE__; throw _err__ ; }

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define BUFFER_TYPE	(unsigned long*)
#elif __dest_os == __win32_os
#define BUFFER_TYPE	(unsigned int*)
#endif

#pragma mark ____________________________consts

const char* cPluginName = "PGP Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSecurity;
const char* cPluginDescription = "PGP Security plugin for Mulberry." COPYRIGHT;
const char* cProcessedBy = "processed by Mulberry PGP Plugin";
const char* cProcessVersion = "Mulberry PGP Plugin v4";

#pragma mark ____________________________CPGPPluginDLL

#if __dest_os == __mac_os
CMachOLoader* sPGPSDKLoader = NULL;
#endif

// Constructor
CPGPPluginDLL::CPGPPluginDLL()
{
	// Initialize allocatable storage to  kInvalidPGP..
	mContextRef		= kInvalidPGPContextRef;
	mKeyDBRef		= kInvalidPGPKeyDBRef;
	mFilteredSetRef	= kInvalidPGPKeySetRef;
	mInputFile		= kInvalidPGPFileSpecRef;
	mOutputFile		= kInvalidPGPFileSpecRef;

	mLastErrno = eSecurity_NoErr;
	mErrorLine = 0;
	mGotPassphrase = false;
	mVerified = false;

	mSignedByList = NULL;
	mEncryptedToList = NULL;
	
	mCallbackPtr = NULL;
}

// Destructor
CPGPPluginDLL::~CPGPPluginDLL()
{
	if (mSignedByList)
		cdstring::FreeArray(mSignedByList);
	if (mEncryptedToList)
		cdstring::FreeArray(mEncryptedToList);
}

// Initialise plug-in
void CPGPPluginDLL::Initialise(void)
{
	// Do default
	CSecurityPluginDLL::Initialise();

#if __dest_os == __mac_os
	if (sPGPSDKLoader == NULL)
	{
		sPGPSDKLoader = new CMachOLoader("PGP.Framework");
		if (sPGPSDKLoader->IsLoaded())
			PGPSDK_LoadFuncPtrs();
	}
#elif __dest_os == __win32_os
	PGPSDK_LoadFuncPtrs();
#endif
}

// Does plug-in need to be registered
bool CPGPPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'PGP5');
	return false;
}

// Can plug-in run as demo
bool CPGPPluginDLL::CanDemo(void)
{
	// Must be registered
	return false;
}

#define DATE_PROTECTION		0

#define	COPYP_MAX_YEAR	2000
#define COPYP_MAX_MONTH	3

// Test for run ability
bool CPGPPluginDLL::CanRun(void)
{
	bool result = false;

#if DATE_PROTECTION
	time_t systime = ::time(nil);
	struct tm* currtime = ::localtime(&systime);

	if ((currtime->tm_year + 1900 > COPYP_MAX_YEAR) ||
		((currtime->tm_year + 1900 == COPYP_MAX_YEAR) && (currtime->tm_mon + 1 > COPYP_MAX_MONTH)))
		return false;
#endif

#if __dest_os == __mac_os
	if (sPGPSDKLoader == NULL)
		sPGPSDKLoader = new CMachOLoader("PGP.Framework");

	// Check that bundle was available
	if (!sPGPSDKLoader->IsLoaded())
	{
		REPORTERROR(eSecurity_UnknownError, "Could not load PGP.Framework");
		return false;
	}
	else
		PGPSDK_LoadFuncPtrs();
#elif __dest_os == __win32_os
	PGPSDK_LoadFuncPtrs();
#endif

	// Check for PGPsdk
	PGPError err = ::PGPsdkInit(0);
	if (IsPGPError(err))
	{
		REPORTERROR(eSecurity_UnknownError, "Could initialise PGP SDK");
		return false;
	}
	PGPUInt32 vers = ::PGPGetPGPsdkAPIVersion();
	result = ((vers & 0xFF000000) == (kPGPsdkAPIVersion & 0xFF000000));
	if (!result)
		REPORTERROR(eSecurity_UnknownError, "PGP SDK version mismatch");

	err = ::PGPsdkCleanup();
	return result;
}

// Returns the name of the plug-in
const char* CPGPPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CPGPPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CPGPPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CPGPPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CPGPPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

#pragma mark ____________________________Memory Based

// Sign data with address
long CPGPPluginDLL::SignData(const char* in, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];
	char			passphrase[256];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();

		// Get key ref for key with passphrase
		PGPKeyDBObjRef theKeyRef = GetKeyRef(key, passphrase);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Sign block
		PGPSize mimeBodyOffset;
		char mimeSeparator[kPGPMimeSeparatorSize];
		if (useMime)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
								//::PGPOPGPMIMEEncoding(mContextRef, TRUE, &mimeBodyOffset, mimeSeparator),
					 			::PGPOClearSign(mContextRef, TRUE),
			   					::PGPODetachedSig(mContextRef, ::PGPOLastOption(mContextRef)),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		
		// Convert outging data
		if (out && out_len)
		{
			cdstring temp(*out, *out_len);
			::PGPFreeData((void*) *out);
			temp.ConvertEndl();
			*out_len = temp.length();
			*out = temp.grab_c_str();
		}
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up passphrase memory
	::memset(passphrase, 0, sizeof(passphrase));

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishPGP();

	return result;
}

// Encrypt to addresses
long CPGPPluginDLL::EncryptData(const char* in, const char** to, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();

		// Generate key set
		GenerateKeySet(to);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Encrypt block
		if (useMime)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		
		// Convert outging data
		if (out && out_len)
		{
			cdstring temp(*out, *out_len);
			::PGPFreeData((void*) *out);
			temp.ConvertEndl();
			*out_len = temp.length();
			*out = temp.grab_c_str();
		}
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishPGP();

	return result;
}

// Encrypt to addresses and sign with address
long CPGPPluginDLL::EncryptSignData(const char* in, const char** to, const char* key, char** out, unsigned long* out_len, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];
	char			passphrase[256];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();

		// Get key ref for key with passphrase
		PGPKeyDBObjRef theKeyRef = GetKeyRef(key, passphrase);

		// Generate key set
		GenerateKeySet(to);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Encrypt and sign block
		if (useMime)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOArmorOutput(mContextRef, TRUE),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOArmorOutput(mContextRef, TRUE),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		
		// Convert outging data
		if (out && out_len)
		{
			cdstring temp(*out, *out_len);
			::PGPFreeData((void*) *out);
			temp.ConvertEndl();
			*out_len = temp.length();
			*out = temp.grab_c_str();
		}
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up passphrase memory
	::memset(passphrase, 0, sizeof(passphrase));

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishPGP();

	return result;
}

// Decrypt or verify data
long CPGPPluginDLL::DecryptVerifyData(const char* in, const char* sig, const char* in_from,
										char** out, unsigned long* out_len, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();

		// Get default Keyring, if not already
		InitKeys();

		// Init passphrase repeater preventer
		mGotPassphrase = false;
		mVerified = true;
		mDidSignature = false;

		// Verify/decrypt block - look for detached sig
		if (sig)
		{
			ThrowIfPGPErr(PGPDecode(mContextRef,
		   				 		::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOKeyDBRef(mContextRef, mKeyDBRef),
	   				 			::PGPOInputBuffer(mContextRef, sig, ::strlen(sig)),
					 			::PGPODetachedSig(mContextRef, ::PGPOInputBuffer(mContextRef, in, ::strlen(in)), ::PGPOLastOption(mContextRef)),
	   				 			out && out_len ? ::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len) :
	   				 								::PGPODiscardOutput(mContextRef, TRUE),
								::PGPOLastOption(mContextRef)))
		}
		else
		{
			ThrowIfPGPErr(PGPDecode(mContextRef,
		   				 		::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOKeyDBRef(mContextRef, mKeyDBRef),
	   				 			::PGPOInputBuffer(mContextRef, in, ::strlen(in)),
	   				 			out && out_len ? ::PGPOAllocatedOutputBuffer(mContextRef, (void**) out, 2L * 1024L * 1024L, BUFFER_TYPE out_len) :
	   				 								::PGPODiscardOutput(mContextRef, TRUE),
								::PGPOLastOption(mContextRef)))
		}

		*success = mVerified;
		*did_sig = mDidSignature;
		*sig_ok = true;

		if (mSignedBy.size())
		{
			mSignedByList = cdstring::ToArray(mSignedBy);
			*out_signedby = (char*) mSignedByList;
		}

		if (mEncryptedTo.size())
		{
			mEncryptedToList = cdstring::ToArray(mEncryptedTo);
			*out_encryptedto = (char*) mEncryptedToList;
		}
		
		// Convert outging data
		if (out && out_len)
		{
			cdstring temp(*out, *out_len);
			::PGPFreeData((void*) *out);
			temp.ConvertEndl();
			*out_len = temp.length();
			*out = temp.grab_c_str();
		}
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);

		*success = false;
		*sig_ok = false;

		result = 0;
	}

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishPGP();

	return result;
}

long CPGPPluginDLL::DisposeData(const char* data)
{
	free((void*) data);
	return 1;
}

#pragma mark ____________________________File based

// Sign file
long CPGPPluginDLL::SignFile(fspec in, const char* key, fspec out, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];
	char			passphrase[256];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();
		PrepareFiles(in, out);

		// Get key ref for key with passphrase
		PGPKeyDBObjRef theKeyRef = GetKeyRef(key, passphrase);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Sign file
		if (useMime)
			// Don't use local encoding - the input file is already a MIME message in the data fork (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPODetachedSig(mContextRef, ::PGPOLastOption(mContextRef)),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			// Use local encoding to ensure data and resource forks are signed (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
	   				 			::PGPOLocalEncoding(mContextRef, kPGPLocalEncoding_Force | kPGPLocalEncoding_NoMacBinCRCOkay),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
							   	::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPODetachedSig(mContextRef, ::PGPOLastOption(mContextRef)),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))

#if __dest_os == __mac_os
		// Do LF -> lendl conversion
		auto_ptr<char> out_path(ToPath(out));
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up passphrase memory
	::memset(passphrase, 0, sizeof(passphrase));

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishFiles();
	FinishPGP();

	return result;
}

// Encrypt file
long CPGPPluginDLL::EncryptFile(fspec in, const char** to, fspec out, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();
		PrepareFiles(in, out);

		// Generate key set
		GenerateKeySet(to);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Encrypt file
		if (useMime)
			// Don't use local encoding - the input file is already a MIME message in the data fork (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			// Use local encoding to ensure data and resource forks are signed (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
	   				 			::PGPOLocalEncoding(mContextRef, kPGPLocalEncoding_Force | kPGPLocalEncoding_NoMacBinCRCOkay),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOClearSign(mContextRef, TRUE),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))

#if __dest_os == __mac_os
		// Do LF -> lendl conversion
		auto_ptr<char> out_path(ToPath(out));
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishFiles();
	FinishPGP();

	return result;
}

// Encrypt & sign file
long CPGPPluginDLL::EncryptSignFile(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary)
{
	// non allocatable storage
	PGPSize			mimeBodyOffset;
	char			mimeSeparator[kPGPMimeSeparatorSize];
	char			passphrase[256];

	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();
		PrepareFiles(in, out);

		// Get key ref for key with passphrase
		PGPKeyDBObjRef theKeyRef = GetKeyRef(key, passphrase);

		// Generate key set
		GenerateKeySet(to);

		// check for Minimum Entropy before encoding
		if(!::PGPGlobalRandomPoolHasMinimumEntropy())
			ThrowPGPErr(kPGPError_OutOfEntropy);

		// Sign block
		// Sign file
		if (useMime)
			// Don't use local encoding - the input file is already a MIME message in the data fork (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOArmorOutput(mContextRef, TRUE),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))
		else
			// Use local encoding to ensure data and resource forks are signed (Mac OS)
			ThrowIfPGPErr(PGPEncode(mContextRef,
	   				 			::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOInputFile(mContextRef, mInputFile),
	   				 			::PGPOOutputFile(mContextRef, mOutputFile),
		   				 		::PGPOLocalEncoding(mContextRef, kPGPLocalEncoding_Force | kPGPLocalEncoding_NoMacBinCRCOkay),
					 			::PGPOSignWithKey(mContextRef, theKeyRef, ::PGPOPassphrase(mContextRef, passphrase), ::PGPOLastOption(mContextRef)),
								::PGPODataIsASCII(mContextRef, binary ? TRUE : FALSE),
					 			::PGPOArmorOutput(mContextRef, TRUE),
					 			::PGPOEncryptToKeySet(mContextRef, mFilteredSetRef),
					 			::PGPOCommentString(mContextRef, cProcessedBy),
					 			::PGPOVersionString(mContextRef, cProcessVersion),
								::PGPOLastOption(mContextRef)))

#if __dest_os == __mac_os
		// Do LF -> lendl conversion
		auto_ptr<char> out_path(ToPath(out));
		result = lendl_convertLF(out_path.get(), NULL, true);
#endif
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);
		result = 0;
	}

	// Clean up passphrase memory
	::memset(passphrase, 0, sizeof(passphrase));

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishFiles();
	FinishPGP();

	return result;
}

// Decrypt/verify file
long CPGPPluginDLL::DecryptVerifyFile(fspec in, const char* sig, const char* in_from,
										fspec out, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	long result = 1;
	try
	{
		// Prepare it
		PreparePGP();
		PrepareFiles(in, out);

		// Get default Keyring, if not already
		InitKeys();

		// Init passphrase repeater preventer
		mGotPassphrase = false;
		mVerified = true;
		mDidSignature = false;

		// Verify/decrypt block - look for detached sig

		// NB Don't use local encoding - we are always decoding just the data of the file, not the resource fork (Mac OS)
		if (sig)
		{
			ThrowIfPGPErr(PGPDecode(mContextRef,
		   				 		::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOKeyDBRef(mContextRef, mKeyDBRef),
	   				 			::PGPOInputBuffer(mContextRef, sig, ::strlen(sig)),
					 			::PGPODetachedSig(mContextRef, ::PGPOInputFile(mContextRef, mInputFile), ::PGPOLastOption(mContextRef)),
   				 				out ? ::PGPOOutputFile(mContextRef, mOutputFile) : ::PGPODiscardOutput(mContextRef, TRUE),
								::PGPOLastOption(mContextRef)))
		}
		else
		{
			ThrowIfPGPErr(PGPDecode(mContextRef,
		   				 		::PGPOEventHandler(mContextRef, mCallbackPtr, this),
	   				 			::PGPOKeyDBRef(mContextRef, mKeyDBRef),
   				 				::PGPOInputFile(mContextRef, mInputFile),
   				 				out ? ::PGPOOutputFile(mContextRef, mOutputFile) : ::PGPODiscardOutput(mContextRef, TRUE),
								::PGPOLastOption(mContextRef)))
		}

		*success = mVerified;
		*did_sig = mDidSignature;
		*sig_ok = true;

		if (mSignedBy.size())
		{
			mSignedByList = cdstring::ToArray(mSignedBy);
			*out_signedby = (char*) mSignedByList;
		}

		if (mEncryptedTo.size())
		{
			mEncryptedToList = cdstring::ToArray(mEncryptedTo);
			*out_encryptedto = (char*) mEncryptedToList;
		}

#if __dest_os == __mac_os_x || __dest_os == __win32_os
		// Do LF -> lendl conversion only for valid output file
		// When verifying there may not be an output
		auto_ptr<char> out_path(ToPath(out));
		if (out_path.get() && *out_path.get())
			result = lendl_convertLF(out_path.get(), NULL, true);
#endif
	}

	// Error Reporting
	catch (PGPError &ex)
	{
		HandleError(ex);

		*success = false;
		*sig_ok = false;

		result = 0;
	}

	// Clean up - Deallocate all refs
	FinishKeys();
	FinishFiles();
	FinishPGP();

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
long CPGPPluginDLL::GetMIMESign(SMIMEMultiInfo* params)
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
long CPGPPluginDLL::GetMIMEEncrypt(SMIMEMultiInfo* params)
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
long CPGPPluginDLL::GetMIMEEncryptSign(SMIMEMultiInfo* params)
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
long CPGPPluginDLL::CanVerifyThis(const char* type)
{
	// Return 0 if it can verify, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	return !::strcmpnocase(type, "application/pgp-signature") ? 0 : 1;
}

// Check that MIME type is decryptable by this plugin
long CPGPPluginDLL::CanDecryptThis(const char* type)
{
	// Return 0 if it can decrypt, 1 if not
	// This ensures that old PGP plugins that don't support this call
	// pretend to support the crypto type
	return ::strcmpnocase(type, "application/pgp-encrypted");
}

// Get last textual error
long CPGPPluginDLL::GetLastError(long* errnum, char** error)
{
	*errnum = mLastErrno;
	*error = mLastError.c_str_mod();
	return 1;
}

#pragma mark ____________________________Utilities
// Prepare PGP context
void CPGPPluginDLL::PreparePGP()
{
#if __dest_os == __mac_os
	if (sPGPSDKLoader == NULL)
		sPGPSDKLoader = new CMachOLoader("PGP.Framework");

	// Check that bundle was available
	if (!sPGPSDKLoader->IsLoaded())
		throw kPGPError_UnknownError;
	else
		PGPSDK_LoadFuncPtrs();
#elif __dest_os == __win32_os
	PGPSDK_LoadFuncPtrs();
#endif

	// Check for PGPsdk
	PGPError err = ::PGPsdkInit(0);
	if (IsPGPError(err))
		throw kPGPError_UnknownError;

	// Create a new PGP context
	ThrowIfPGPErr(::PGPNewContext(kPGPsdkAPIVersion, &mContextRef))
	
#if __dest_os == __mac_os
	mCallbackPtr = (CallbackPtr) MachOFunctionPointerForCFMFunctionPointer(Callback);
#else
	mCallbackPtr = Callback;
#endif
}

// Finish with PGP context
void CPGPPluginDLL::FinishPGP()
{
	if (PGPContextRefIsValid(mContextRef))
		::PGPFreeContext(mContextRef);

	// Initialize allocatable storage to  kInvalidPGP..
	mContextRef = kInvalidPGPContextRef;

	PGPError err = ::PGPsdkCleanup();

#if __dest_os == __mac_os
	if (mCallbackPtr)
		DisposePtr((Ptr)mCallbackPtr);
#endif
	mCallbackPtr = NULL;
}

// Prepare file references
void CPGPPluginDLL::PrepareFiles(fspec in, fspec out)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (in)
		ThrowIfPGPErr(::PGPNewFileSpecFromFSSpec(mContextRef, in, &mInputFile))
	if (out)
		ThrowIfPGPErr(::PGPNewFileSpecFromFSSpec(mContextRef, out, &mOutputFile))
#elif __dest_os == __win32_os
	if (in)
		ThrowIfPGPErr(::PGPNewFileSpecFromFullPath(mContextRef, in, &mInputFile))
	if (out)
		ThrowIfPGPErr(::PGPNewFileSpecFromFullPath(mContextRef, out, &mOutputFile))
#endif
}

// Finish with file references
void CPGPPluginDLL::FinishFiles()
{
	if (PGPFileSpecRefIsValid(mInputFile))
		::PGPFreeFileSpec(mInputFile);
	if (PGPFileSpecRefIsValid(mOutputFile))
		::PGPFreeFileSpec(mOutputFile);

	// Initialize allocatable storage to  kInvalidPGP..
	mInputFile = kInvalidPGPFileSpecRef;
	mOutputFile = kInvalidPGPFileSpecRef;
}

// Set MIME details
void CPGPPluginDLL::SetMIMEDetails(SMIMEInfo* mime, const char* type, const char* subtype, const char** params)
{
	mime->type = type;
	mime->subtype = subtype;
	mime->params = params;
}

// Handle thrown error
void CPGPPluginDLL::HandleError(PGPError& ex)
{
	long errtype;
	switch(ex)
	{
	case kPGPError_NoErr:
		errtype = eSecurity_NoErr;
		break;
	case kPGPError_UserAbort:
		errtype = eSecurity_UserAbort;
		break;
	case kPGPError_BadPassphrase:
		errtype = eSecurity_BadPassphrase;
		break;
	default:
		errtype = eSecurity_UnknownError;
	}

	if(IsPGPError(ex))
	{
		cdstring temp;
		temp.reserve(1024);
		::PGPGetErrorString(ex, 1024, temp.c_str_mod());

		if (ex == kPGPError_ItemNotFound)
		{
			temp += " ";
			temp += mMissingItem;
		}
		ErrorReport(errtype, temp, mErrorFunction, mErrorFile, mErrorLine);
	}
}

#pragma mark ____________________________Keys

// Initialise key rings
void CPGPPluginDLL::InitKeys()
{
	// Get default Keyring, if not already
	if (mKeyDBRef == kInvalidPGPKeyDBRef)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		cdstring pub;
		cdstring priv;
		GetKeyringPaths(pub, priv);

		FSSpec fpub;
		if (::FSPathMakeFSSpec((UInt8*) pub.c_str(), &fpub, NULL) != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to find public key file");
			throw -1;
		}

		FSSpec fpriv;
		if (::FSPathMakeFSSpec((UInt8*) priv.c_str(), &fpriv, NULL) != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to find private key file");
			throw -1;
		}

		CPGPFileSpecRef pubspec;
		CPGPFileSpecRef privspec;

		ThrowIfPGPErr(::PGPNewFileSpecFromFSSpec(mContextRef, &fpub, pubspec));
		ThrowIfPGPErr(::PGPNewFileSpecFromFSSpec(mContextRef, &fpriv, privspec));
#elif __dest_os == __win32_os
		cdstring pub;
		cdstring priv;
		GetKeyringPaths(pub, priv);

		CPGPFileSpecRef pubspec;
		CPGPFileSpecRef privspec;
		
		ThrowIfPGPErr(::PGPNewFileSpecFromFullPath(mContextRef, pub, pubspec));
		ThrowIfPGPErr(::PGPNewFileSpecFromFullPath(mContextRef, priv, privspec));
#endif

		ThrowIfPGPErr(::PGPOpenKeyDBFile(mContextRef, kPGPOpenKeyDBFileOptions_None, pubspec, privspec, &mKeyDBRef))
	}
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Brute memory search
int brutesearch(const char* p, int plen, const char* a, int alen);
int brutesearch(const char* p, int plen, const char* a, int alen)
{
	int i;
	int j;
	int M = plen;
	int N = alen;
	
	for(i = 0, j = 0; (j < M) && (i < N); i++, j++)
		while((a[i] != p[j]) && (i < N))
		{
			i -= j-1; j=0;
		}
	if (j == M)
		return i - M;
	else
		return -1;
}
#elif __dest_os == __win32_os
bool GetSpecialFolderPath(int nFolder, cdstring& folder);
bool GetSpecialFolderPath(int nFolder, cdstring& folder)
{
	// Try Shell folder first
	bool result = false;
	LPITEMIDLIST pidl = NULL;
	LPMALLOC pMalloc = NULL;

	::SHGetMalloc(&pMalloc);

	if (::SHGetSpecialFolderLocation(NULL, nFolder, &pidl) == NOERROR)
	{
		cdstring temp;
		temp.reserve(MAX_PATH);
		if (::SHGetPathFromIDList(pidl, temp))
		{
			// Must end with dir-delim
			folder = temp;
			if (folder[folder.length() - 1] != os_dir_delim)
				folder += os_dir_delim;
			
			result = true;
		}

		if (pMalloc)
			pMalloc->Free(pidl);
	}
	
	// Clean up
	if (pMalloc)
		pMalloc->Release();
	
	return result;
}
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CPGPPluginDLL::ConvertToMacPath(cdstring& path)
{
	FSRef fref;
	Boolean isDirectory;
	OSStatus err = ::FSPathMakeRef((unsigned char*)path.c_str(), &fref, &isDirectory);
	if (err == noErr)
	{
		FSSpec fsspec;
		err = ::FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &fsspec, NULL);
		if (err == noErr)
		{		
			path.steal(ToPath(&fsspec));
			return;
		}
	}
	
	REPORTERROR(eSecurity_UnknownError, "Could not convert file path");
	throw -1;
}
#endif

void CPGPPluginDLL::GetKeyringPaths(cdstring& pub, cdstring& sec)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	cdstring user_path;
	user_path += ::getenv("HOME");
	user_path += os_dir_delim;
	user_path += "Library";
	user_path += os_dir_delim;
	user_path += "Preferences";
	user_path += os_dir_delim;
	user_path += "com.pgp.desktop.plist";
#elif __dest_os == __win32_os
	// Try Shell folder first
	cdstring user_path;
	if (!::GetSpecialFolderPath(CSIDL_APPDATA, user_path))
	{
		// Use user profile directory
		user_path = ::getenv("USERPROFILE");
		if (user_path.length() && (user_path[user_path.length() - 1] != os_dir_delim))
		{
			user_path += os_dir_delim;
			user_path += "Application Data";
			user_path += os_dir_delim;
		}
	}
	user_path += "PGP Corporation";
	user_path += os_dir_delim;
	user_path += "PGP";
	user_path += os_dir_delim;
	user_path += "PGPPrefs.xml";
#endif
	
	struct stat info;
	if (::stat(user_path, &info) == 0)
		GetKeyringPathsXML(pub, sec);
	else
		GetKeyringPathsText(pub, sec);
}

void CPGPPluginDLL::GetKeyringPathsText(cdstring& pub, cdstring& sec)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Load "client" preference from PGP prefs file
	CFDataRef data = (CFDataRef)CFPreferencesCopyAppValue(CFSTR("client"), CFSTR("com.pgp.PGP"));
	if (data)
	{
		// Get bytes for this data item
		CFIndex len = CFDataGetLength(data);
		auto_ptr<unsigned char> temp(new unsigned char[len]);
		CFDataGetBytes(data, CFRangeMake(0, len), temp.get());

		// Look for first occurrence of /Volumes, /Users, /Applications, /System or /Library
		const char* tests[] = {"/Users", "/Applications", "/Volumes", "/System", "/Library", NULL};
		const int testslen[] = {6, 13, 8, 7, 8, 0};
		
		const char** p = &tests[0];
		const int* plen = &testslen[0];
		while(*p)
		{
			int pos1 = brutesearch(*p, *plen, (char*)temp.get(), len);
			if (pos1 >= 0)
			{
				// Look for second occurrence of /Volumes
				const char** q = &tests[0];
				const int* qlen = &testslen[0];
				while(*q)
				{
					int pos2 = brutesearch(*q, *qlen, (char*)temp.get() + pos1 + *plen, len - pos1 - *plen);
					if (pos2 >= 0)
					{
						pub = (char*)temp.get() + pos1;
						sec = (char*)temp.get() + pos1 + *plen + pos2;
						ConvertToMacPath(pub);
						ConvertToMacPath(sec);
						break;
					}
					q++;
					qlen++;
				}
				break;
			}
			p++;
			plen++;
		}
		CFRelease(data);
	}
#elif __dest_os == __win32_os
	// Try Shell folder first
	cdstring user_path;
	if (!::GetSpecialFolderPath(CSIDL_APPDATA, user_path))
	{
		// Use user profile directory
		user_path = ::getenv("USERPROFILE");
		if (user_path.length() && (user_path[user_path.length() - 1] != os_dir_delim))
		{
			user_path += os_dir_delim;
			user_path += "Application Data";
			user_path += os_dir_delim;
		}
	}
	user_path += "PGP Corporation";
	user_path += os_dir_delim;
	user_path += "PGP";
	user_path += os_dir_delim;
	user_path += "PGPPrefs.txt";
	
	// Read lines from file
	ifstream fin(user_path);
	while(!fin.fail())
	{
		// Get a line
		cdstring line;
		::getline(fin, line, 0);
		
		// Must have valid items
		if (!line.empty())
		{
			bool public_file  = line.compare_start("PublicKeyringFile=");
			bool private_file = line.compare_start("PrivateKeyringFile=");
			if (public_file || private_file)
			{
				// Get possibly quoted string
				char* p = line.c_str_mod();
				if (public_file)
					p += 18;
				else if (private_file)
					p += 19;
				
				// Loop for continuation, accumulating preference
				cdstring temp;
				while(true)
				{
					temp += ::strgetquotestr(&p, false);
				
					// Look for continuation
					if (p && *p && (p[::strlen(p) - 1] == '\\') && !fin.fail())
					{
						// Get next line
						::getline(fin, line, 0);
						if (line.empty())
							break;
						else
							p = line.c_str_mod();
					}
					else
						break;
				}
				
				// Copy to appropriate item
				if (public_file)
					pub = temp;
				else if (private_file)
					sec = temp;
			}
		}
	}
#endif
}

void CPGPPluginDLL::GetKeyringPathsXML(cdstring& pub, cdstring& sec)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	cdstring user_path;
	user_path += ::getenv("HOME");
	user_path += os_dir_delim;
	user_path += "Library";
	user_path += os_dir_delim;
	user_path += "Preferences";
	user_path += os_dir_delim;
	user_path += "com.pgp.desktop.plist";
#elif __dest_os == __win32_os
	// Try Shell folder first
	cdstring user_path;
	if (!::GetSpecialFolderPath(CSIDL_APPDATA, user_path))
	{
		// Use user profile directory
		user_path = ::getenv("USERPROFILE");
		if (user_path.length() && (user_path[user_path.length() - 1] != os_dir_delim))
		{
			user_path += os_dir_delim;
			user_path += "Application Data";
			user_path += os_dir_delim;
		}
	}
	user_path += "PGP Corporation";
	user_path += os_dir_delim;
	user_path += "PGP";
	user_path += os_dir_delim;
	user_path += "PGPPrefs.xml";
#endif
	
	// Read lines from file
	ifstream fin(user_path);
	while(!fin.fail())
	{
		// Get a line
		cdstring line;
		::getline(fin, line, 0);
		
		// Must have valid items
		if (!line.empty())
		{
			bool public_file  = line.find("<key>publicKeyringFile</key>") != cdstring::npos;
			bool private_file = line.find("<key>privateKeyringFile</key>") != cdstring::npos;
			if (public_file || private_file)
			{
				// Get next line
				cdstring line2;
				::getline(fin, line2, 0);
				if (!line2.empty())
				{
					unsigned long start = line2.find("<string>");
					unsigned long end = line2.find("</string>");
					if ((start != cdstring::npos) && (end != cdstring::npos))
					{
						cdstring temp(line2, start + 8, end - (start + 8));

						// Copy to appropriate item
						if (public_file)
							pub = temp;
						else if (private_file)
							sec = temp;
					}
				}
			}
		}
	}
}

// Get Key
PGPKeyDBObjRef CPGPPluginDLL::GetKeyRef(const char* key, char* passphrase)
{
	// non allocatable storage
	PGPKeyDBObjRef theKeyRef = kInvalidPGPKeyDBObjRef;

	char** users = NULL;

	try
	{
		// Get default Keyring, if not already
		InitKeys();

		// Determine nature of filter
		bool keyid = false;
		if ((::strlen(key) == 8) ||
			((key[0] == '0') && (tolower(key[1]) == 'x')))
		{
			keyid = true;
			const char* p = key;
			if ((p[0] == '0') && (tolower(p[1]) == 'x'))
				p += 2;
			while(*p)
				keyid = keyid && (isxdigit(*p++) != 0);
		}

		CPGPKeySetRef filtered;
		if (keyid)
		{
			PGPKeyID pgpid;
			ThrowIfPGPErr(::PGPNewKeyIDFromString(key, kPGPPublicKeyAlgorithm_Invalid, &pgpid));

			PGPKeyDBObjRef keyobj;
			ThrowIfPGPErr(::PGPFindKeyByKeyID(mKeyDBRef, &pgpid, &keyobj));

			// Create a key set contain just the one key we found
			ThrowIfPGPErr(::PGPNewOneKeySet(keyobj, filtered));

			// Get list of user ids
			GenerateUsers(filtered, users, NULL, true);
		}
		else
		{
			// Create filter based on user id
			CPGPFilterRef theKeyFilter;
			ThrowIfPGPErr(::PGPNewKeyDBObjDataFilter(mContextRef, kPGPUserIDProperty_EmailAddress, key, ::strlen(key), kPGPMatchCriterion_SubString, theKeyFilter))

			// Ignore revoked filters
			PGPFilterRef filter;
			ThrowIfPGPErr(::PGPNewKeyDBObjBooleanFilter(mContextRef, kPGPKeyProperty_IsRevoked, false, &filter))

			// Intersect existing
			ThrowIfPGPErr(::PGPIntersectFilters(theKeyFilter, filter, theKeyFilter))

			// Filter default key set
			ThrowIfPGPErr(::PGPFilterKeyDB(mKeyDBRef, theKeyFilter, filtered))

			// Get list of user ids
			GenerateUsers(filtered, users, NULL, true);
		}

		if (users)
		{
			// Get signing passphrase
			unsigned long index = 0;
			if (!GetPassphrase((const char**)(users), passphrase, index))
				ThrowPGPErr(kPGPError_UserAbort);

			FreeUsers(users);

			// Now get matching keyref

			// Sort the key Set in no particular order
			CPGPKeyListRef theKeyListRef;
			ThrowIfPGPErr(::PGPOrderKeySet(filtered, kPGPKeyOrdering_UserID, false, theKeyListRef))

			// Create an Key Iterator for this Keyset
			CPGPKeyIterRef theIterator;
			ThrowIfPGPErr(::PGPNewKeyIter(theKeyListRef, theIterator))

			// Grab the first key
			ThrowIfPGPErr(::PGPKeyIterNextKeyDBObj(theIterator, kPGPKeyDBObjType_Key, &theKeyRef))

			// Now move to the one we want
			if (index)
				ThrowIfPGPErr(::PGPKeyIterMove(theIterator, index, &theKeyRef))
		}
		else
			ThrowPGPErr(kPGPError_ItemNotFound);
	}

	catch (PGPError &ex)
	{
		// Clean up - Deallocate all refs
		FreeUsers(users);
		FinishKeys();

		throw;
	}

	return theKeyRef;
}

// Generate a key set
void CPGPPluginDLL::GenerateKeySet(const char** keys)
{
	try
	{

		// Get default Keyring, if not already
		InitKeys();

		// Iterate over all input keys
		const char** p = keys;
		while(*p)
		{
			// Create filter based on user id
			PGPFilterRef filter1;
			ThrowIfPGPErr(::PGPNewKeyDBObjDataFilter(mContextRef, kPGPUserIDProperty_EmailAddress, *p, ::strlen(*p), kPGPMatchCriterion_SubString, &filter1))

			// Ignore revoked filters
			PGPFilterRef filter2;
			ThrowIfPGPErr(::PGPNewKeyDBObjBooleanFilter(mContextRef, kPGPKeyProperty_IsRevoked, false, &filter2))

			// Intersect them
			CPGPFilterRef filter3;
			ThrowIfPGPErr(::PGPIntersectFilters(filter1, filter2, &filter3.get()))

			// Do filter of default key set
			CPGPKeySetRef found_set;
			ThrowIfPGPErr(::PGPFilterKeyDB(mKeyDBRef, filter3, &found_set.get()))

			// Check that there are some there
			PGPUInt32 count;
			ThrowIfPGPErr(::PGPCountKeys(found_set, &count))

			// Must have some - otherwise error!
			if (!count)
			{
				mMissingItem = *p;
				ThrowPGPErr(kPGPError_ItemNotFound)
			}

			// Accumulate found keys
			if (mFilteredSetRef == kInvalidPGPKeySetRef)
				ThrowIfPGPErr(::PGPNewEmptyKeySet(mKeyDBRef, &mFilteredSetRef))

			ThrowIfPGPErr(::PGPAddKeys(found_set, mFilteredSetRef))

			// Bump to next
			p++;
		}
	}

	catch (PGPError &ex)
	{
		// Clean up - Deallocate all refs
		FinishKeys();

		throw;
	}
}

// Generate a keys from set
void CPGPPluginDLL::GenerateUsers(PGPKeySetRef keys, char**& users, cdstrvect* userids, bool unique)
{
	// Get size of key set and create array holder
	PGPUInt32 size;
	ThrowIfPGPErr(::PGPCountKeys(keys, &size));

	if (!size)
		ThrowPGPErr(kPGPError_ItemNotFound);

	users = (char**) malloc((size + 1)*sizeof(char*));
	char** p = users;
	for(int i = 0; i <= size; i++)
		*p++ = NULL;

	CPGPKeyListRef keyList;
	CPGPKeyIterRef keyIter;
	try
	{
		// Create keylist
		ThrowIfPGPErr(::PGPOrderKeySet(keys, kPGPKeyOrdering_UserID, false, &keyList.get()));

		// Create iterator
		ThrowIfPGPErr(::PGPNewKeyIter(keyList, &keyIter.get()));

		// Add each user id
		p = users;
		for(int i = 0; i < size; i++, p++)
		{
			// Get uid
			PGPKeyDBObjRef key;
			ThrowIfPGPErr(::PGPKeyIterNextKeyDBObj(keyIter, kPGPKeyDBObjType_Key, &key));

			// Put uid in array
			*p = (char*) malloc(256);
			PGPSize usedLength = 0;
			ThrowIfPGPErr(::PGPGetPrimaryUserIDName(key, *p, 256, &usedLength));
			
			// Copy name into save array if present
			if (userids)
				userids->push_back(*p);

			if (unique)
			{
				// Now add algorithm id
				PGPInt32 algorithm;
				ThrowIfPGPErr(::PGPGetKeyDBObjNumericProperty(key, kPGPKeyProperty_AlgorithmID, &algorithm));
				switch(algorithm)
				{
				case kPGPPublicKeyAlgorithm_RSA:
				case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
				case kPGPPublicKeyAlgorithm_RSASignOnly:
					::strncat(*p, " (RSA ", 256 - ::strlen(*p));
					break;
				case kPGPPublicKeyAlgorithm_ElGamal:
				case kPGPPublicKeyAlgorithm_DSA:
					::strncat(*p, " (DH/DSS ", 256 - ::strlen(*p));
					break;
				}

				// Now add size
				PGPInt32 bits;
				ThrowIfPGPErr(::PGPGetKeyDBObjNumericProperty(key, kPGPKeyProperty_Bits, &bits));
				char bits_txt[256];
				::sprintf(bits_txt, "%d)", bits);
				::strncat(*p, bits_txt, 256 - ::strlen(*p));

				// Now add key id
				PGPKeyID keyID;
				ThrowIfPGPErr(::PGPGetKeyID(key, &keyID));
				char keyIDStr[kPGPMaxKeyIDStringSize];
				ThrowIfPGPErr(::PGPGetKeyIDString(&keyID, kPGPKeyIDString_Abbreviated, keyIDStr));
				char keyIDFormat[kPGPMaxKeyIDStringSize + 3];
				::sprintf(keyIDFormat, " (%s)", keyIDStr);
				::strncat(*p, keyIDFormat, 256 - ::strlen(*p));
			}
		}

		*p = NULL;
	}
	catch(...)
	{
		FreeUsers(users);
	}
}

// Free list of users
void CPGPPluginDLL::FreeUsers(char**& users)
{
	if (users)
	{
		char** p = users;
		while(*p)
			free(*p++);
		free(users);

		users = NULL;
	}
}

// Prepare PGP data
void CPGPPluginDLL::FinishKeys()
{
	// Clean up - Deallocate all refs
	if(PGPKeySetRefIsValid(mFilteredSetRef))
		::PGPFreeKeySet(mFilteredSetRef);
	if(PGPKeyDBRefIsValid(mKeyDBRef))
		::PGPFreeKeyDB(mKeyDBRef);

	// Initialize allocatable storage to  kInvalidPGP..
	mKeyDBRef		= kInvalidPGPKeyDBRef;
	mFilteredSetRef	= kInvalidPGPKeySetRef;
}

#pragma mark ____________________________Callbacks

#if __dest_os == __mac_os
//
//	This function allocates a block of CFM glue code which contains the instructions to call CFM routines
//
UInt32 asm_template[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};

void* MachOFunctionPointerForCFMFunctionPointer(void* cfmfp)
{
    UInt32	*mfp = (UInt32*) NewPtr( sizeof(asm_template) );		//	Must later dispose of allocated memory
    mfp[0] = asm_template[0] | ((UInt32)cfmfp >> 16);
    mfp[1] = asm_template[1] | ((UInt32)cfmfp & 0xFFFF);
    mfp[2] = asm_template[2];
    mfp[3] = asm_template[3];
    mfp[4] = asm_template[4];
    mfp[5] = asm_template[5];
    MakeDataExecutable( mfp, sizeof(asm_template) );
    return( mfp );
}
#endif

PGPError CPGPPluginDLL::Callback(PGPContextRef pgpContext, PGPEvent* event, PGPUserValue userValue)
{
	// Callbacks must not propogate exceptions
	PGPError result;
	try
	{
		result = reinterpret_cast<CPGPPluginDLL*>(userValue)->EventCallback(pgpContext, event);
	}
	catch(PGPError err)
	{
		result = err;
	}

	return result;
}

PGPError CPGPPluginDLL::EventCallback(PGPContextRef pgpContext, PGPEvent* event)
{
	// Look for specific events
	switch(event->type)
	{
	case kPGPEvent_SignatureEvent:
		// Check validity of signing if not already failed
		if (mVerified)
		{
			PGPEventSignatureData* sigData = reinterpret_cast<PGPEventSignatureData*>(&event->data);
			if (!sigData->checked)
			{
				mVerified = false;
				cdstring temp = "Signing key is not available. Key ID: ";

				char keyIDStr[kPGPMaxKeyIDStringSize];
				ThrowIfPGPErr(::PGPGetKeyIDString(&sigData->signingKeyID, kPGPKeyIDString_Abbreviated, keyIDStr));
				char keyIDFormat[kPGPMaxKeyIDStringSize + 1];
				::sprintf(keyIDFormat, " %s", keyIDStr);
				temp += keyIDFormat;
				REPORTERROR(eSecurity_KeyUnavailable, temp);
			}
			else if (!sigData->verified)
			{
				mVerified = false;
				REPORTERROR(eSecurity_InvalidSignature, "Signature invalid");
			}
			else if (sigData->keyRevoked || sigData->keyDisabled || sigData->keyExpired)
			{
				mVerified = false;
				REPORTERROR(eSecurity_DubiousKey, "Signing key is dubious");
			}
			else if (sigData->verified)
			{
				mDidSignature = true;
			}
			
			// Get signed by
			if (sigData->signingKey)
			{
#if 0
				// Get prmary uid
				char uid[256];
				PGPSize usedLength = 0;
				ThrowIfPGPErr(::PGPGetPrimaryUserIDName(sigData->signingKey, uid, 256, &usedLength));
				mSignedBy.push_back(uid);
#else
				// Create a key set contain just the one key we found
				CPGPKeySetRef found_set;
				ThrowIfPGPErr(::PGPNewOneKeySet(sigData->signingKey, found_set));

				// Create an Key Iterator for this Keyset
				CPGPKeyIterRef theIterator;
				ThrowIfPGPErr(::PGPNewKeyIterFromKeySet(found_set, theIterator))
				
				// Iterate over each user id
				PGPKeyDBObjRef theUserIDRef = kInvalidPGPKeyDBObjRef;
				while(IsntPGPError(::PGPKeyIterNextKeyDBObj(theIterator, kPGPKeyDBObjType_UserID, &theUserIDRef)))
				{
					// Determine type of user id
					PGPInt32 type;
					ThrowIfPGPErr(::PGPGetKeyDBObjNumericProperty(theUserIDRef, kPGPUserIDProperty_AttributeType, &type));
					
					// Ignore photos
					if (type == kPGPAttribute_Image)
						continue;

					// Get the name
					void* buffer;
					PGPSize bufsize;
					ThrowIfPGPErr(::PGPGetKeyDBObjAllocatedDataProperty(theUserIDRef, kPGPUserIDProperty_Name, &buffer, &bufsize));
					
					cdstring uid((char*) buffer);
					::PGPFreeData(buffer);
					mSignedBy.push_back(uid);
				}
#endif
			}
		}
		return kPGPError_NoErr;

	case kPGPEvent_PassphraseEvent:
	{
		// Only do once
		if (mGotPassphrase)
			return kPGPError_BadPassphrase;
		mGotPassphrase = true;

		char passphrase[256];
		char** keys = NULL;

		typedef struct xPGPEventPassphraseData_
		{
			PGPBoolean				fConventional;
			short					dummy;
			PGPKeySetRef			keyset;
		} xPGPEventPassphraseData;

		GenerateUsers(reinterpret_cast<xPGPEventPassphraseData*>(&event->data)->keyset, keys, &mEncryptedTo, true);
		unsigned long index = 0;
		if (keys && GetPassphrase((const char**)(keys), passphrase, index))
		{
			PGPAddJobOptions(event->job, ::PGPOPassphrase(pgpContext, passphrase), ::PGPOLastOption(pgpContext));

			// Clear out passphrase memeory
			::memset(passphrase, 0, sizeof(passphrase));

			FreeUsers(keys);

			return kPGPError_NoErr;
		}
		else
		{
			FreeUsers(keys);

			return kPGPError_UserAbort;
		}
	}

	default:
		return kPGPError_NoErr;
	}
}

#pragma mark ____________________________CPGPKeySetRef

CPGPPluginDLL::CPGPKeySetRef::CPGPKeySetRef()
{
	mData = kInvalidPGPKeySetRef;
}

CPGPPluginDLL::CPGPKeySetRef::~CPGPKeySetRef()
{
	if (PGPKeySetRefIsValid(mData))
		::PGPFreeKeySet(mData);
}

#pragma mark ____________________________CPGPFilterRef

CPGPPluginDLL::CPGPFilterRef::CPGPFilterRef()
{
	mData = kInvalidPGPFilterRef;
}

CPGPPluginDLL::CPGPFilterRef::~CPGPFilterRef()
{
	if (PGPFilterRefIsValid(mData))
		::PGPFreeFilter(mData);
}

#pragma mark ____________________________CPGPKeyListRef

CPGPPluginDLL::CPGPKeyListRef::CPGPKeyListRef()
{
	mData = kInvalidPGPKeyListRef;
}

CPGPPluginDLL::CPGPKeyListRef::~CPGPKeyListRef()
{
	if (PGPKeyListRefIsValid(mData))
		::PGPFreeKeyList(mData);
}

#pragma mark ____________________________CPGPKeyIterRef

CPGPPluginDLL::CPGPKeyIterRef::CPGPKeyIterRef()
{
	mData = kInvalidPGPKeyIterRef;
}

CPGPPluginDLL::CPGPKeyIterRef::~CPGPKeyIterRef()
{
	// Done with iterator
	if (PGPKeyIterRefIsValid(mData))
		::PGPFreeKeyIter(mData);
}

#pragma mark ____________________________CPGPFileSpecRef

CPGPPluginDLL::CPGPFileSpecRef::CPGPFileSpecRef()
{
	mData = kInvalidPGPFileSpecRef;
}

CPGPPluginDLL::CPGPFileSpecRef::~CPGPFileSpecRef()
{
	// Done with iterator
	if (PGPFileSpecRefIsValid(mData))
		::PGPFreeFileSpec(mData);
}
