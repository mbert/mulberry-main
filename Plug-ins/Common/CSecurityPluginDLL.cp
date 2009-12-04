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

// CSecurityPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL basedauthorization plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the authorization DLL being called by Mulberry.
//
// History:
// CD:	 04-May-1998:	Created initial header and implementation.
//

#include "CSecurityPluginDLL.h"

#include "CStringUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __dest_os == __mac_os_x
#include "MoreFilesX.h"
#endif
#if __dest_os == __win32_os
#include <fcntl.h>
#include <stat.h>
typedef size_t ssize_t;
#endif
#if __dest_os == __linux_os
#include <fcntl.h>
#define O_BINARY 0
#include <sys/stat.h>
#endif

const char* f_tmp = "/tmp/Mulberry_tmp_XXXXXX";

//#define VISIBLE_TEMP_FILES

#pragma mark ____________________________CSecurityPluginDLL

// Constructor
CSecurityPluginDLL::CSecurityPluginDLL()
{
	mCallback = NULL;
}

// Destructor
CSecurityPluginDLL::~CSecurityPluginDLL()
{
}

// DLL entry point and dispatch
long CSecurityPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{
	case CSecurityPluginDLL::eSecuritySignData:
	{
		SSignData* sign = (SSignData*) data;
		return SignData(sign->mInputData, sign->mKey, sign->mOutputData, sign->mOutputDataLength, sign->mUseMIME, sign->mBinary);
	}
	case CSecurityPluginDLL::eSecurityEncryptData:
	{
		SEncryptData* encrypt = (SEncryptData*) data;
		return EncryptData(encrypt->mInputData, encrypt->mKeys, encrypt->mOutputData, encrypt->mOutputDataLength, encrypt->mUseMIME, encrypt->mBinary);
	}
	case CSecurityPluginDLL::eSecurityEncryptSignData:
	{
		SEncryptSignData* esign = (SEncryptSignData*) data;
		return EncryptSignData(esign->mInputData, esign->mKeys, esign->mSignKey, esign->mOutputData, esign->mOutputDataLength, esign->mUseMIME, esign->mBinary);
	}
	case CSecurityPluginDLL::eSecurityDecryptVerifyData:
	{
		SDecryptVerifyData* decrypt = (SDecryptVerifyData*) data;
		return DecryptVerifyData(decrypt->mInputData, decrypt->mInputSignature, decrypt->mInputFrom,
									decrypt->mOutputData, decrypt->mOutputDataLength, decrypt->mOutputSignedby, decrypt->mOutputEncryptedto,
									decrypt->mSuccess, decrypt->mDidSig, decrypt->mSigOK, decrypt->mBinary);
	}
	case CSecurityPluginDLL::eSecuritySignFile:
	{
		SSignFile* signf = (SSignFile*) data;
		return SignFile(signf->mInputFile, signf->mKey, signf->mOutputFile, signf->mUseMIME, signf->mBinary);
	}
	case CSecurityPluginDLL::eSecurityEncryptFile:
	{
		SEncryptFile* encryptf = (SEncryptFile*) data;
		return EncryptFile(encryptf->mInputFile, encryptf->mKeys, encryptf->mOutputFile, encryptf->mUseMIME, encryptf->mBinary);
	}
	case CSecurityPluginDLL::eSecurityEncryptSignFile:
	{
		SEncryptSignFile* esignf = (SEncryptSignFile*) data;
		return EncryptSignFile(esignf->mInputFile, esignf->mKeys, esignf->mSignKey, esignf->mOutputFile, esignf->mUseMIME, esignf->mBinary);
	}
	case CSecurityPluginDLL::eSecurityDecryptVerifyFile:
	{
		SDecryptVerifyFile* decryptf = (SDecryptVerifyFile*) data;
		return DecryptVerifyFile(decryptf->mInputFile, decryptf->mInputSignature, decryptf->mInputFrom,
									decryptf->mOutputFile, decryptf->mOutputSignedby, decryptf->mOutputEncryptedto,
									decryptf->mSuccess, decryptf->mDidSig, decryptf->mSigOK, decryptf->mBinary);
	}
	case eSecurityDisposeData:
		return DisposeData(static_cast<char*>(data));

	case eSecurityGetLastError:
	{
		SGetLastError* last_error = static_cast<SGetLastError*>(data);
		return GetLastError(last_error->errnum, last_error->error);
	}
	case eSecurityGetMIMEParamsSign:
	{
		GetMIMESign(static_cast<SMIMEMultiInfo*>(data));
		return 0;
	}
	case eSecurityGetMIMEParamsEncrypt:
		GetMIMEEncrypt(static_cast<SMIMEMultiInfo*>(data));
		return 0;

	case eSecurityGetMIMEParamsEncryptSign:
		GetMIMEEncryptSign(static_cast<SMIMEMultiInfo*>(data));
		return 0;

	case eSecurityCanVerifyThis:
		return CanVerifyThis(static_cast<char*>(data));

	case eSecurityCanDecryptThis:
		return CanDecryptThis(static_cast<char*>(data));

	case eSecuritySetCallback:
		SetCallback((CallbackProcPtr) data);
		return 0;

	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}

#pragma mark ____________________________Callbacks

bool CSecurityPluginDLL::GetSignPassphrase(const char* key, char* passphrase)
{
	const char* users[2];
	users[0] = key;
	users[1] = NULL;
	unsigned long chosen;

	return GetPassphrase(users, passphrase, chosen);
}

bool CSecurityPluginDLL::GetPassphrase(const char** users, char* passphrase, unsigned long& chosen)
{
	// Only if callback available
	if (!mCallback)
		return false;

	SCallbackPassphrase data;
	data.users = users;
	data.passphrase = passphrase;

	bool result = (*mCallback)(eCallbackPassphrase, &data);
	if (result)
		chosen = data.chosen;
	return result;
}

#pragma mark ____________________________Utilities

void CSecurityPluginDLL::ErrorReport(long err_no, const char* errtxt, const char* func, const char* file, int lineno)
{
	// Get default plugin error string
	cdstring err_buf = GetName();
	err_buf += " Error: ";
	err_buf += errtxt;

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

		LogEntry(serr);
	}
	
	// Get short error string for display to user
	SetLastError(err_no, err_buf);
}

// Create temp files for data processing
void CSecurityPluginDLL::TempCreate(fspec in_tmp, fspec out_tmp, const char* in)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	try
	{
		// Create input temp file
		if (!MakeTemp(in_tmp))
		{
			*in_tmp->name = 0;
			REPORTERROR(eSecurity_UnknownError, "Failed to create temporary input file for signing");
			throw -1L;
		}

		// Write input data to temp file
		short refNum = 0;
		OSErr err = ::FSpOpenDF(in_tmp, fsRdWrPerm, &refNum);
		if (err != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open temporary input file for signing");
			throw -1L;
		}
		
		// Write data (may be empty)
		long size_in = (in ? ::strlen(in) : 0);
		if (size_in)
			err = ::FSWrite(refNum, &size_in, in);
		if (err != noErr)
		{
			::FSClose(refNum);
			REPORTERROR(eSecurity_UnknownError, "Failed to write to temporary input file for signing");
			throw -1L;
		}
		err = ::FSClose(refNum);
		if (err != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to close temporary input file for signing");
			throw -1L;
		}

		// Create temp output file
		if (out_tmp && !MakeTemp(out_tmp))
		{
			*out_tmp->name = 0;
			REPORTERROR(eSecurity_UnknownError, "Failed to create temporary output file for signing");
			throw -1L;
		}
	}
	catch(...)
	{
		// Clean up
		if (*in_tmp->name)
		{
			::FSpDelete(in_tmp);
			*in_tmp->name = 0;
		}
		if (*out_tmp->name)
		{
			::FSpDelete(out_tmp);
			*out_tmp->name = 0;
		}

		// Throw up
		throw;
	}
#else
	int fd = -1;

	try
	{
		::strcpy(const_cast<char*>(in_tmp), f_tmp);
		fd = mkstemp(const_cast<char*>(in_tmp));
		if (fd == -1)
		{
			*const_cast<char*>(in_tmp) = 0;
			REPORTERROR(eSecurity_UnknownError, "Failed to create temporary input file for signing");
			throw -1L;
		}
		if (in)
			::write(fd, (void*) in, ::strlen(in));
		::close(fd);
		fd = -1;

		// Create temp output file
		if (out_tmp)
		{
			::strcpy(const_cast<char*>(out_tmp), f_tmp);
			fd = mkstemp(const_cast<char*>(out_tmp));
			if (fd == -1)
			{
				*const_cast<char*>(out_tmp) = 0;
				REPORTERROR(eSecurity_UnknownError, "Failed to create temporary output file for signing");
				throw -1L;
			}
			::close(fd);
			fd = -1;
		}
	}
	catch(...)
	{
		// Clean up
		if (fd != -1)
			::close(fd);
		if (*in_tmp)
		{
			::remove(in_tmp);
			*const_cast<char*>(in_tmp) = 0;
			}
		if (out_tmp && *out_tmp)
		{
			::remove(out_tmp);
			*const_cast<char*>(out_tmp) = 0;
		}

		// Throw up
		throw;
	}
#endif
}

// Read temp output file into buffer
void CSecurityPluginDLL::TempRead(fspec out_tmp, char** out, unsigned long* out_len)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	try
	{
		// Read output data to memory
		short refNum = 0;
		OSErr err = ::FSpOpenDF(out_tmp, fsRdWrPerm, &refNum);
		if (err != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open temporary output file after signing");
			throw -1L;
		}

		// Get its size
		long theLength;
		err = ::GetEOF(refNum, &theLength);
		if (err != noErr)
		{
			::FSClose(refNum);
			REPORTERROR(eSecurity_UnknownError, "Could not get size of temporary output file after signing");
			throw -1L;
		}

		// Create output buffer and read in data
		*out = (char*) ::malloc(theLength + 1);
		err = ::FSRead(refNum, &theLength, *out);
		if (err != noErr)
		{
			::FSClose(refNum);
			::free(*out);
			*out = NULL;
			REPORTERROR(eSecurity_UnknownError, "Could not read temporary output file after signing");
			throw -1L;
		}
		(*out)[theLength] = 0;
		*out_len = theLength;

		err = ::FSClose(refNum);
		if (err != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to close temporary output file after signing");
			throw -1L;
		}
	}
	catch(...)
	{
		// Clean up

		throw;
	}
#else
	int fd = -1;

	try
	{
		// Open it
		fd = ::open(out_tmp, O_RDONLY | O_BINARY);
		if (fd == -1)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open temporary output file after signing");
			throw -1L;
		}

		// Get its size
		struct stat fbuf;
		if (::fstat(fd, &fbuf) == -1)
		{
			REPORTERROR(eSecurity_UnknownError, "Could not get size of temporary output file after signing");
			throw -1L;
		}

		// Create output buffer and read in data
		*out = (char*) ::malloc(fbuf.st_size + 1);
		ssize_t rdsize = ::read(fd, *out, fbuf.st_size);
		(*out)[rdsize] = 0;
		*out_len = rdsize;

		::close(fd);
		fd = -1;
	}
	catch(...)
	{
		// Clean up
		if (fd != -1)
			::close(fd);

		throw;
	}
#endif
}

char* CSecurityPluginDLL::ToPath(fspec spec)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (spec)
	{
#if __dest_os == __mac_os
		short fullPathLength;
		Handle fullPath;
		OSErr err = ::FSpGetFullPath(spec, &fullPathLength, &fullPath);
		if (!err)
		{
			cdstring path(*fullPath, fullPathLength);
			::DisposeHandle(fullPath);
			return path.grab_c_str();
		}
#else
		cdstring path;
		path.reserve(1024);
		FSRef fref;
		OSErr err = ::FSpMakeFSRef(spec, &fref);
		if (!err)
		{
			err = ::FSRefMakePath(&fref, (unsigned char*) path.c_str_mod(), 1024);
			if (!err)
				return path.grab_c_str();
		}
#endif
	}

	return ::strdup(cdstring::null_str);
#else
	return ::strdup(spec);
#endif
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
bool CSecurityPluginDLL::MakeTemp(fspec ftemp)
{
	bool result = false;

	try
	{
		// Generate a suitable name
		cdstring new_name("MulberryTemp");

		// Find temporary folder
#ifdef VISIBLE_TEMP_FILES
		OSErr err = ::FindFolder(kOnSystemDisk, kDesktopFolderType, kCreateFolder,
								&ftemp->vRefNum, &ftemp->parID);
#else
		OSErr err = ::FindFolder(kUserDomain, kTemporaryFolderType, kCreateFolder, &ftemp->vRefNum, &ftemp->parID);
		if (err != noErr)
			err = ::FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &ftemp->vRefNum, &ftemp->parID);
#endif

		if (err != noErr)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to find temporary folder");
			throw err;
		}

		// Convert to FSSpec
		Str255 pnew_name;
		c2pstrcpy(pnew_name, new_name.c_str());
		*ftemp->name = 0;
		::PLstrncpy(ftemp->name, pnew_name, 31);
		new_name = ftemp->name;

		// Loop to create a temp file name that is not a duplicate of an existing item
		err = dupFNErr;
		unsigned long ctr = 0;
		while((err == dupFNErr) && (err != noErr) && (ctr < 100))
		{
			err = ::FSpCreate(ftemp, 'Mlby', 'SECR', 0);
			if (err == dupFNErr)
			{
				if (new_name.length() > 28)
					new_name.c_str_mod()[28] = 0;
				cdstring temp = new_name;
				temp += cdstring(++ctr);
				Str255 ptemp;
				c2pstrcpy(ptemp, temp.c_str());
				*ftemp->name = 0;
				::PLstrncpy(ftemp->name, ptemp, 31);
			}
			else if ((err != noErr) || (ctr > 100))
			{
				REPORTERROR(eSecurity_UnknownError, "Failed to create temporary file");
				throw err;
			}
		}
		
		if (ctr >= 100)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to create temporary file");
			throw err;
		}

		result = true;
	}
	catch(...)
	{
		result = false;
	}
	
	return result;
}
#elif __dest_os == __win32_os
int CSecurityPluginDLL::mkstemp(fspec ftemp, bool open_fd)
{
	int fd = -1;
	try
	{
		// Generate a suitable name
		cdstring new_name;
		new_name.reserve(L_tmpnam);
		::tmpnam(new_name.c_str_mod());
		cdstring temp_name(::strrchr(new_name.c_str(), '\\') + 1);

		// Locate temp directory and create full path
		cdstring tmp;
		tmp.reserve(1024);
		::GetTempPath(1024, tmp.c_str_mod());
		if (tmp[tmp.length() -1] != '\\')
			tmp += '\\';
		tmp += temp_name;
		
		::strcpy(const_cast<char*>(ftemp), tmp.c_str());

		// Create/open the temp file
		if (open_fd)
			fd = ::open(tmp.c_str(), O_CREAT | O_RDWR | O_BINARY);
		else
			fd = 1;
	}
	catch(...)
	{
	}
	
	return fd;
}
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
long CSecurityPluginDLL::lendl_convertLF(const char* spec, cdstring* tspec, bool to_lendl)
{
	return lendl_convert(spec, tspec, to_lendl, true);
}

long CSecurityPluginDLL::lendl_convertCRLF(const char* spec, cdstring* tspec, bool to_lendl)
{
	return lendl_convert(spec, tspec, to_lendl, false);
}

long CSecurityPluginDLL::lendl_convert(const char* spec, cdstring* tspec, bool to_lendl, bool to_LF)
{
	// Ignore empty spec
	if (!spec || !*spec)
		return 0;

	long result = 1;

	bool replace_original = (tspec == NULL);

	cdstring temp;
	if (replace_original)
	{
		tspec = &temp;
		*tspec = spec;
		*tspec += ".tmp";
	}
	else
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Create temp file for conversion
		FSSpec tmp_spec;
		if (!MakeTemp(&tmp_spec))
			return 0;
		
		*tspec = ToPath(&tmp_spec);

#elif __dest_os == __win32_os
		// Create temp file for conversion (do not open it in mkstemp as we will do that later)
		tspec->reserve(1024);
		if (!mkstemp(tspec->c_str_mod(), false))
			return 0;	
#endif
	}
	
	FILE* fin = NULL;
	FILE* fout = NULL;
	try
	{
		// Open old for read new for write
		fin = ::fopen(spec, "rb");
		if (!fin)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open input file for endl conversion");
			throw -1L;
		}
		fout = ::fopen(tspec->c_str(), "wb");
		if (!fout)
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to open output file for endl conversion");
			throw -1L;
		}
		
		// Buffer to convert
		// Make output twice the size of input to cope with line end doubling
		const int cBufSize = 4096;
		cdstring read_buf;
		read_buf.reserve(cBufSize);
		cdstring write_buf;
		write_buf.reserve(2*cBufSize);
		
		// Read in chunks and convert and write out
		size_t read_in = cBufSize;
		char previous = 0;
		while(read_in == cBufSize)
		{
			// Read in a chuck
			read_in = ::fread(read_buf.c_str_mod(), 1, cBufSize, fin);
			if (read_in == 0)
				break;

			// Convert LFs to CRs
			char* p = read_buf.c_str_mod();
			char* q = write_buf.c_str_mod();
			size_t p_size = read_in;
			size_t q_size = 0;
			while(p_size--)
			{
				char current = *p;
				switch(current)
				{
				// Look for possible line end
				case '\r':
				case '\n':
					// Always ignore \n if its a \r\n pair
					if ((current == '\n') && (previous == '\r'))
					{
						// Ignore it
						previous = current;
						p++;
					}
					else
					{
						// Ignore it
						previous = current;
						p++;
						
						// Write out appropriate endl
						if (to_lendl)
						{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
							*q++ = '\r';
							q_size++;
#elif __dest_os == __win32_os
							*q++ = '\r';
							*q++ = '\n';
							q_size += 2;
#endif
						}
						else
						{
							if (!to_LF)
							{
								*q++ = '\r';
								q_size++;
							}
							*q++ = '\n';
							q_size++;
						}
					}
					break;
				default:
					previous = *p++;
					*q++ = previous;
					q_size++;
					break;
				}
			}
			
			// Write it out
			if (::fwrite(write_buf.c_str_mod(), 1, q_size, fout) != q_size)
			{
				REPORTERROR(eSecurity_UnknownError, "Failed to write file for endl conversion");
				throw -1L;
			}
		}
		
		// Close files
		if (::fclose(fin))
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to close input file for endl conversion");
			throw -1L;
		}
		fin = NULL;
		if (::fclose(fout))
		{
			REPORTERROR(eSecurity_UnknownError, "Failed to close output file for endl conversion");
			throw -1L;
		}
		fout = NULL;
		
		// Delete original and rename temp one
		if (replace_original)
		{
			if (::remove(spec))
			{
				REPORTERROR(eSecurity_UnknownError, "Failed to delete input file for endl conversion");
				throw -1L;
			}
			if (::rename(tspec->c_str(), spec))
			{
				REPORTERROR(eSecurity_UnknownError, "Failed to rename output file for endl conversion");
				throw -1L;
			}
		}
	}
	catch(...)
	{
		// Close open files
		if (fin)
			::fclose(fin);
		fin = NULL;
		if (fout)
			::fclose(fout);
		fout = NULL;

		// Always delete the temp file
		::remove(tspec->c_str());
		
		// Failed
		result = 0;
	}
	
	return result;
}
#endif
